#include "Solver/Callback.hpp"

#include <queue>

#include <ilcplex/ilocplexi.h>

#include "Math.hpp"
#include "Solver.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 Callback::Callback( Config        const & conf
                   , ExtChainGraph const & gs
                   )
   : config( conf )
   , graphs( gs ) {
 }

 Callback::Callback( Callback const & other )
   : config( other.config )
   , graphs( other.graphs ) {
 }

 void Callback::constructRHS( GraphVariables const & vars
                            , NodeSet        const & dS
                            , NodeSet        const & S
                            , IloExpr              & rhs
                            ) const {
   rhs.clear();

   for ( Graph::Node const & j : dS ) {
     rhs += vars.xVars[vars.nodeToIndex[j]];
   }

   for ( Graph::Node const & j : S ) {
     rhs += vars.yVars[vars.nodeToIndex[j]];
   }
 }

 bool Callback::isValid( Graph       const & g
                       , Graph::Node const & i
                       , NodeSet     const & dS
                       , NodeSet     const & S) const {
   if ( S.find( i ) == S.end() ) {
     // i must be in S
     return false;
   }

   if ( dS.find( i ) != dS.end() ) {
     // i must not be in dS
     return false;
   }

   // determine dS again
   NodeSet ddS;
   for ( Graph::Node i : S ) {
     for ( Graph::Edge e : g.incEdges( i) ) {
       Graph::Node j = g.oppositeNode( i, e );
       if ( S.find( j ) == S.end() )
       {
         ddS.insert( j );
       }
     }
   }

   return dS == ddS;
 }

 void Callback::constructNonZeroComponents( Graph          const & g
                                          , GraphVariables const & vars
                                          , IloNumArray            x_values
                                          , NodeSetVector        & nonZeroNodesComponents
                                          ) const {
   using NodeQueue = std::queue< Graph::Node >;
   NodeQueue queue;

   // -3 queued
   // -2 unvisited
   // -1 visited and zero
   // >= 0 visited and non-zero
   auto compMap = g.createNodeMap< int >( -2 );

   int compIdx = 0;
   for ( Graph::Node n : g.nodes() ) {
     if ( compMap[n] == -2 && intIsNonZero( x_values[vars.nodeToIndex[n]] ) ) {
       queue.push( n );
       compMap[n] = -3;
       nonZeroNodesComponents.emplace_back();

       // perform bfs
       while ( !queue.empty() ) {
         Graph::Node n = queue.front();
         assert( compMap[n] == -3 );

         queue.pop();
         compMap[n] = compIdx;

         nonZeroNodesComponents.back().insert( n );

         for ( Graph::Edge e : g.incEdges( n ) ) {
           Graph::Node nn = g.oppositeNode( n, e );

           if ( compMap[nn] == -2 ) {
             if ( intIsNonZero( x_values[vars.nodeToIndex[nn]] ) ) {
               queue.push( nn );
               compMap[nn] = -3;
             } else {
               compMap[nn] = -1;
             }
           }
         }
       }

       ++compIdx;
     }
   }
 }

 void Callback::printNonZeroX( IloNumArray x_values, int graphIndex ) const {
   clog << "xVars graph " << graphIndex << ":" << endl;
   int number = 0;
   for_each( graphs.graph( graphIndex ).nodes(), [&]( Graph::Node const & i ) {
     int nodeIndex = graphs.variables( graphIndex ).nodeToIndex[i];
     double x_i_value = x_values[nodeIndex];
     if ( intIsNonZero( x_i_value ) ) {
       clog << " " << graphs.variables( graphIndex ).xVars[nodeIndex].getName()
            << " " << x_i_value << endl;
       ++number;
     }
   });
   clog << "number: " << number << endl;
 }

 void Callback::printNonZeroY( IloNumArray y_values, int graphIndex ) const {
   for ( Graph::Node const & i : graphs.graph( graphIndex ).nodes() ) {
     int nodeIndex = graphs.variables( graphIndex ).nodeToIndex[i];
     double y_i_value = y_values[nodeIndex];
     if ( intIsNonZero( y_i_value ) )
     {
       clog << graphs.variables( graphIndex ).yVars[nodeIndex].getName()
            << " " << y_i_value;
       clog << endl;
     }
   }
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
