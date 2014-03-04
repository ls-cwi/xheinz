#include "Solver/LazyConstraintCallback.hpp"

#include <ilcplex/ilocplexi.h>

#include "Math.hpp"
#include "Solver.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 LazyConstraintCallback::LazyConstraintCallback( IloEnv env
                                               , Config const & conf
                                               , ExtChainGraph const & gs
                                               )
   : IloCplex::LazyConstraintCallbackI{ env }
   , Callback{ conf, gs } {
 }

 LazyConstraintCallback::LazyConstraintCallback( LazyConstraintCallback const & other )
   : IloCplex::LazyConstraintCallbackI{ other }
   , Callback{ other } {
 }

 IloCplex::CallbackI * LazyConstraintCallback::duplicateCallback() const {
   return new (getEnv()) LazyConstraintCallback( *this );
 }

 void LazyConstraintCallback::main() {
   assert( graphs.numGraphs() == 2 );
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     processGraph( i );
   }
 }

 void LazyConstraintCallback::processGraph( int index ) {
   Graph const          & graph         = graphs.graph( index );
   GraphVariables const & vars          = graphs.variables( index );

   IloNumArray x_values{ getEnv(), vars.xVars.getSize() };
   getValues( x_values, vars.xVars );

   IloNumArray y_values{ getEnv(), vars.yVars.getSize() };
   getValues( y_values, vars.yVars );

   //printNonZeroX( x_values, index );
   //printNonZeroY( y_values, index );

   int nCuts = 0;
   NodeSetVector nonZeroNodesComponents;
   constructNonZeroComponents( graph, vars, x_values, nonZeroNodesComponents );

   if ( nonZeroNodesComponents.size() > 1 )
   {
     Graph::Node root = determineRoot( graph, vars, y_values );
     separateConnectedComponents( graph
                                , vars
                                , root
                                , nonZeroNodesComponents
                                , nCuts
                                );
   }

   x_values.end();
   y_values.end();

   //clog << "Generated " << nCuts << " cuts" << endl;
 }

 void LazyConstraintCallback::separateConnectedComponents( Graph          const & g
                                                         , GraphVariables const & vars
                                                         , Graph::Node    const & root
                                                         , NodeSetVector  const & nonZeroNodesComponents
                                                         , int                  & nCuts
                                                         ) {
   IloExpr rhs( getEnv() );

   for ( const NodeSet& S : nonZeroNodesComponents ) {
     // only consider the non-zero components that don't contain the root
     if ( S.find( root ) == S.end() )
     {
       // determine dS
       NodeSet dS;
       for ( Graph::Node i : S ) {
         for ( Graph::Edge e : g.incEdges( i) ) {
           Graph::Node j = g.oppositeNode( i, e );
           if ( S.find( j ) == S.end() )
           {
             dS.insert( j );
           }
         }
       }

       constructRHS( vars, dS, S, rhs );
       for ( Graph::Node i : S ) {
         assert( isValid( g, i, dS, S ) );
         add( vars.xVars[vars.nodeToIndex[i]] <= rhs, IloCplex::UseCutPurge ).end();
         ++nCuts;
       }
     }
   }

   rhs.end();
 }

 Graph::Node LazyConstraintCallback::determineRoot( Graph          const & g
                                                  , GraphVariables const & vars
                                                  , IloNumArray    const & y_values
                                                  ) const {
   Graph::Node root;

#ifdef DEBUG
   bool first = true;
#endif

   for ( Graph::Node const & i : g.nodes() ) {
     int nodeIndex = vars.nodeToIndex[i];
     if ( intIsNonZero( y_values[nodeIndex] ) ) {
       root = i;
#ifdef DEBUG
       assert(first);
       first = false;
#else
       break;
#endif
     }
   }

   return root;
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
