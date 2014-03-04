#include "Solver/UserCutCallback.hpp"

#include <queue>
#include <ilcplex/ilocplexi.h>

#include "Math.hpp"
#include "Solver.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 UserCutCallback::UserCutCallback( IloEnv                env
                                 , Config        const & conf
                                 , ExtChainGraph const & gs
                                 , BackOff       const & bo
                                 )
   : IloCplex::UserCutCallbackI{ env }
   , Callback{ conf, gs }
 //, minCuts{}   // default is ok, will be filled in ctor body
   , nodeNumber{ 0 }
   , numCutIterations{ 0 }
   , makeAttempt{ true }
   , backOff{ bo } {
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     minCuts.emplace_back();
     for_each( graphs.digraph( i ).components(), [&]( MinCutDigraphComponent const & c ) {
       minCuts.back().emplace_back( c, c.rootNode() );
     });
   }
 }

 UserCutCallback::UserCutCallback( UserCutCallback const & other )
   : IloCplex::UserCutCallbackI{ other }
   , Callback{ other }
 //, minCuts{}   // DO NOT COPY, so default is ok, will be filled in ctor body
   , nodeNumber{ 0 }
   , numCutIterations{ 0 }
   , makeAttempt{ other.makeAttempt }
   , backOff{ other.backOff } {
   // We need to create new MaxFlow, which are per thread and we have one
   // callback per thread
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     minCuts.emplace_back();
     for_each( graphs.digraph( i ).components(), [&]( MinCutDigraphComponent const & c ) {
       minCuts.back().emplace_back( c, c.rootNode() );
     });
   }
 }

 IloCplex::CallbackI * UserCutCallback::duplicateCallback() const {
   return new (getEnv()) UserCutCallback( *this );
 }

 void UserCutCallback::main() {
   if (!isAfterCutLoop())
   {
     // first let cplex do its magic
     return;
   }

   if ( nodeNumber != getNnodes() ) {
     nodeNumber = getNnodes();
     numCutIterations = 0;
     makeAttempt = backOff.makeAttempt();
   }

   ++numCutIterations;

   if ( makeAttempt &&
        ( numCutIterations < config.maxCutIterations || nodeNumber == 0 ) ) {
     assert( graphs.numGraphs() == 2 );
     assert( minCuts.size() == 2 );
     for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
       processGraph( i );
     }
   }
 }

 void UserCutCallback::processGraph( int index ) {
   Graph const          & graph         = graphs.graph( index );
   GraphVariables const & vars          = graphs.variables( index );

   IloNumArray x_values{ getEnv(), vars.xVars.getSize() };
   getValues( x_values, vars.xVars );

   IloNumArray y_values{ getEnv(), vars.yVars.getSize() };
   getValues( y_values, vars.yVars );

   //printNonZeroX( x_values, index );
   //printNonZeroY( y_values, index );

   int nCuts = 0;
   int nBackCuts = 0;
   int nNestedCuts = 0;

   NodeSetVector nonZeroNodesComponents;
   constructNonZeroComponents( graph, vars, x_values, nonZeroNodesComponents );

   if ( nonZeroNodesComponents.size() > 1 )
   {
     NodeSet roots = determineRoots( graph, vars, y_values );
     separateConnectedComponents( graph
                                , vars
                                , roots
                                , nonZeroNodesComponents
                                , x_values, y_values
                                , nCuts
                                );
   }// else if ( nonZeroNodesComponents.size() >= 1 ){
   //  MinCutDigraph const  & digraph       = graphs.digraph( index );
   //  PerGraphMinCut       & graphMinCuts  = minCuts[index];

   //  separateMinCut( graph, digraph, graphMinCuts
   //                , vars, x_values, y_values
   //                , nCuts, nBackCuts, nNestedCuts
   //                );
   //}

   x_values.end();
   y_values.end();

   //clog << "Generated " << nCuts
   //     << " user cuts of which " << nBackCuts << " are back-cuts and "
   //     << nNestedCuts << " are nested cuts" << endl;
 }

 void UserCutCallback::separateMinCut( Graph          const & graph
                                     , MinCutDigraph  const & digraph
                                     , PerGraphMinCut       & graphMinCuts
                                     , GraphVariables const & vars
                                     , IloNumArray    const & x_values
                                     , IloNumArray    const & y_values
                                     , int                  & nCuts
                                     , int                  & nBackCuts
                                     , int                  & nNestedCuts
                                     ) {

   for ( auto const & component : graph.components() ) {
     separateMinCut( graph
                   , component
                   , digraph.component( component.index() )
                   , graphMinCuts[component.index()]
                   , x_values, y_values
                   , vars
                   , nCuts, nBackCuts, nNestedCuts
                   );
   }
 }

 void UserCutCallback::separateMinCut( Graph                  const & graph
                                     , GraphComponent const         & component
                                     , MinCutDigraphComponent const & digraph
                                     , BkMaxFlow                    & bkMaxFlow
                                     , IloNumArray                    x_values
                                     , IloNumArray                    y_values
                                     , GraphVariables const         & vars
                                     , int                          & nCuts
                                     , int                          & nNestedCuts
                                     , int                          & nBackCuts
                                     ) {
   // update capacities for current component
   digraph.updateCapacities( x_values, y_values, bkMaxFlow.capacities );

   IloExpr rhs( getEnv() );

   auto processed = component.createNodeMap< bool >( false );
   for ( GraphComponent::Node i : component.nodes() ) {
     if ( processed[i] )
     {
       continue;
     }

     double x_i_value = x_values[vars.nodeToIndex[Graph::Node( component, i )]];
     auto target = digraph.toExtNode( i ).second;
     bkMaxFlow.reset( target );

     bool first     = true;
     bool nestedCut = false;

     while ( true ) {
       bkMaxFlow.run( !first );
       if ( first ) {
         first = false;
       }

       // let's see if there's a violated constraint
       double minCutValue = bkMaxFlow.maxFlow();
       //bkMaxFlow.printFlow( clog );
       //bkMaxFlow.printCut ( clog );
       //clog << "Min Cut Value: " << minCutValue << endl;
       //clog << "xi      Value: " << x_i_value   << endl;
       if ( cutLessThanEq( x_i_value, minCutValue ) ) {
         break;
       }
       //clog << x_i_value << " <= " << minCutValue << "\t" << nCuts
       //     << "\t" << nBackCuts << "\t" << nNestedCuts << endl;

       // determine N (forward)
       set< Graph::Node > fwdS, fwdDS;
       determineFwdCutSet( component, fwdDS, fwdS, digraph, bkMaxFlow, target );

       // add violated constraints
       constructRHS( vars, fwdDS, fwdS, rhs );
       for ( Graph::Node j : fwdS ) {
         double x_j_value = x_values[vars.nodeToIndex[j]];

         if ( cutLessThan( minCutValue, x_j_value ) )
         {
           assert( isValid( graph, j, fwdDS, fwdS ));

           processed[j] = true;
           add( vars.xVars[vars.nodeToIndex[j]] <= rhs, IloCplex::UseCutPurge).end();

           ++nCuts;
           if ( nestedCut ) {
             nNestedCuts++;
           }
         }
       }

       set< Graph::Node > bwdS, bwdDS;
       determineBwdCutSet( component, bwdDS, bwdS, digraph, bkMaxFlow, target );

       if ( fwdDS.size() != bwdDS.size() || fwdS.size() != bwdS.size()
         || fwdDS != bwdDS || bwdS != fwdS
          ) {

         constructRHS( vars, bwdDS, bwdS, rhs );
         for ( Graph::Node j : bwdS ) {
           double x_j_value = x_values[vars.nodeToIndex[j]];

           if ( cutLessThan( minCutValue, x_j_value ) )
           {
             assert( isValid( graph, j, bwdDS, bwdS ));

             processed[j] = true;
             add( vars.xVars[vars.nodeToIndex[j]] <= rhs, IloCplex::UseCutPurge ).end();

             ++nCuts;
             ++nBackCuts;
             if ( nestedCut ) {
               nNestedCuts++;
             }
           }
         }
       }

       // if DS is empty, meaning that one whole component C containing i is selected
       // *and* none of the nodes in C is the root node, then we stop
       if ( fwdDS.empty() ) {
         break;
       }

       // generate nested-cuts
       for_each( fwdDS, [&]( Graph::Node const & n ) {
         nestedCut = true;
         // update the capactity to generate nested-cuts
         auto k = digraph.toExtNode( n ).first;
         bkMaxFlow.increaseCapacity( *digraph.outArcBegin( k ), 1.0 );
       });
     }
   }

   rhs.end();
 }

// void UserCutCallback::addViolatedConstraint( Graph::Node target
//                                            , set< Graph::Node > const & dS
//                                            , set< Graph::Node > const & S
//                                            , GraphVariables const & vars
//                                            ) {
//   // there is a min-cut that is non-zero and even bigger than x_i_value,
//   // therefore the cut can never ever be empty [because it would be zero then ;)]
//   assert( !(dS.empty() && S.empty()) );
//   IloExpr expr(getEnv());
//   //bool first = true;
//   //clog << getNnodes() << ": " << vars.xVars[vars.nodeToIndex[target]].getName() << " <=";
//   for_each( dS, [&]( Graph::Node const & n ) {
//     expr += vars.xVars[vars.nodeToIndex[n]];
//     //clog << (first ? " " : " + ") << vars.xVars[vars.nodeToIndex[n]].getName();
//     //first = false;
//   });
//
//   //first = true;
//   for_each( S, [&]( Graph::Node const & n ) {
//     expr += vars.yVars[vars.nodeToIndex[n]];
//     //clog << (first ? " " : " + ") << vars.yVars[vars.nodeToIndex[n]].getName();
//     //first = false;
//   });
//
//   //clog << endl;
//
//   IloConstraint constraint = vars.xVars[vars.nodeToIndex[target]] <= expr;
//   add(constraint);
//   constraint.end();
//
//   expr.end();
// }

 void UserCutCallback::determineFwdCutSet( GraphComponent const & component
                                         , set< Graph::Node > & dS
                                         , set< Graph::Node > & S
                                         , MinCutDigraphComponent const & digraph
                                         , BkMaxFlow const              & bkMaxFlow
                                         , MinCutDigraphComponent::Node const & target
                                         ) const {
   using DgArc  = MinCutDigraphComponent::Arc;
   using DgNode = MinCutDigraphComponent::Node;

   auto dfsMarkedCache = digraph.createNodeMap< bool >( false );

   list< DgNode > diS;

   queue< DgNode > queue;
   queue.push( digraph.rootNode() );
   dfsMarkedCache[digraph.rootNode()] = true;

   while ( !queue.empty() ) {
     DgNode v = queue.front();
     queue.pop();
     diS.push_back( v );

     for_each( digraph.outArcs( v ), [&]( DgArc const & a ) {
       DgNode w = digraph.target( a );

       if ( !dfsMarkedCache[w] && cutIsNonZero( bkMaxFlow.residue( a ) ) ) {
         queue.push( w );
         dfsMarkedCache[w] = true;
       }
     });

     for_each( digraph.inArcs( v ), [&]( DgArc const & a ) {
       DgNode w = digraph.source( a );

       if ( !dfsMarkedCache[w] && cutIsNonZero( bkMaxFlow.reverseResidue( a ) ) ) {
         queue.push( w );
         dfsMarkedCache[w] = true;
       }
     });
   }

   assert( dfsMarkedCache[digraph.rootNode()] != dfsMarkedCache[target] );

   for_each( diS, [&]( DgNode const & v ) {
     assert( dfsMarkedCache[v] );
     if ( v == digraph.rootNode() ) {
       for_each( digraph.outArcs( v ), [&]( DgArc const & a ) {
         DgNode w = digraph.target( a );
         Graph::Node gn{ component, digraph.toGraphNode(w) };
         if ( !dfsMarkedCache[w] ) {
           //clog << digraph.id(v) << " -> "
           //     << digraph.id(w) << " (" << component.label( digraph.toGraphNode(w) )<< ") "
           //     << bkMaxFlow.flow(a) << "/" << bkMaxFlow.capacities[a] << endl;
           S.insert( gn );
         }
       });
     } else {
       for_each( digraph.outArcs( v ), [&]( DgArc const & a ) {
         DgNode w = digraph.target( a );
         Graph::Node gn{ component, digraph.toGraphNode(w) };
         if ( !dfsMarkedCache[w] && w != target ) {
           //clog << digraph.id(v) << " (" << component.label( digraph.toGraphNode(v) )<< ")" << " -> "
           //     << digraph.id(w) << " (" << component.label( digraph.toGraphNode(w) )<< ")" << " "
           //     << bkMaxFlow.flow(a) << "/" << bkMaxFlow.capacities[a] << endl;
           dS.insert( gn );
         }
       });
     }
   });
 }

 void UserCutCallback::determineBwdCutSet( GraphComponent const &  component
                                         , set< Graph::Node > & dS
                                         , set< Graph::Node > & S
                                         , MinCutDigraphComponent const & digraph
                                         , BkMaxFlow const              & bkMaxFlow
                                         , MinCutDigraphComponent::Node const & target
                                         ) const {
   using DgArc  = MinCutDigraphComponent::Arc;
   using DgNode = MinCutDigraphComponent::Node;

   auto dfsMarkedCache = digraph.createNodeMap< bool >( false );

   list< DgNode > diS;

   queue< DgNode > queue;
   queue.push( target );
   dfsMarkedCache[target] = true;

   while ( !queue.empty() ) {
     DgNode v = queue.front();
     queue.pop();
     diS.push_back( v );

     for_each( digraph.inArcs( v ), [&]( DgArc const & a ) {
       DgNode u = digraph.source( a );

       if ( !dfsMarkedCache[u] && cutIsNonZero( bkMaxFlow.residue( a ) ) ) {
         queue.push( u );
         dfsMarkedCache[u] = true;
       }
     });

     for_each( digraph.outArcs( v ), [&]( DgArc const & a ) {
       DgNode u = digraph.target( a );

       if ( !dfsMarkedCache[u] && cutIsNonZero( bkMaxFlow.reverseResidue( a ) ) ) {
         queue.push( u );
         dfsMarkedCache[u] = true;
       }
     });
   }

   assert( dfsMarkedCache[digraph.rootNode()] != dfsMarkedCache[target] );

   for ( DgNode const & v : diS ) {
     assert( dfsMarkedCache[v] );
     if ( v == target ) {
       continue;
     }

     for_each( digraph.inArcs( v ), [&]( DgArc const & a ) {
       DgNode u = digraph.source(a);
       Graph::Node gn{ component, digraph.toGraphNode( v ) };
       if ( u == digraph.rootNode() ) {
         assert( !dfsMarkedCache[u] );
         //clog << _h.id(u) << " -> "
         //     << _h.id(v) << " "
         //     << bk.flow(a) << "/" << bk.cap(a) << endl;
         S.insert( gn );
       } else if ( !dfsMarkedCache[u] ) {
         //clog << _h.id(u) << " -> "
         //     << _h.id(v) << " "
         //     << bk.flow(a) << "/" << bk.cap(a) << endl;
         dS.insert( gn );
       }
     });
   }
 }

 void UserCutCallback::separateConnectedComponents( Graph          const & g
                                                  , GraphVariables const & vars
                                                  , NodeSet        const & roots
                                                  , NodeSetVector  const & nonZeroNodesComponents
                                                  , IloNumArray    const & x_values
                                                  , IloNumArray    const & y_values
                                                  , int                  & nCuts
                                                  ) {
   IloExpr rhs( getEnv() );

   for ( const NodeSet& S : nonZeroNodesComponents ) {

     double roots_in_S_sum = 0;
     for ( Graph::Node j : roots ) {
       if ( S.find( j ) != S.end() ) {
         roots_in_S_sum += y_values[vars.nodeToIndex[j]];
       }
     }

     if ( intGreaterThanEq( roots_in_S_sum, 1 ) ) {
       // nothing to separate here, move on to next component
       continue;
     }

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
       double x_i_value = x_values[vars.nodeToIndex[i]];
       if ( intGreaterThan( x_i_value, roots_in_S_sum ) ) {
         assert( isValid( g, i, dS, S ) );
         add( vars.xVars[vars.nodeToIndex[i]] <= rhs, IloCplex::UseCutPurge ).end();
         ++nCuts;
       }
     }
   }

   rhs.end();
 }

 std::set< Graph::Node > UserCutCallback::determineRoots( Graph          const & g
                                                        , GraphVariables const & vars
                                                        , IloNumArray    const & y_values
                                                        ) const {
   NodeSet roots;

   for ( Graph::Node const & i : g.nodes() ) {
     int nodeIndex = vars.nodeToIndex[i];
     if ( intIsNonZero( y_values[nodeIndex] ) ) {
       roots.insert( i );
     }
   }

   assert( roots.size() >= 1 );
   return roots;
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
