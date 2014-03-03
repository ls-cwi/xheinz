#include "Solver/BkCallback.hpp"

#include <queue>

#include <ilcplex/ilocplexi.h>

#include "Math.hpp"
#include "Solver.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 BkCallback::BkCallback( IloEnv env
                       , Config const & conf
                       , ExtChainGraph const & gs
                       )
   : IloCplex::LazyConstraintCallbackI{ env }
   , config( conf )
   , graphs( gs ) {
//clog << "BkCallback creation\n";
//clog.flush();
 //, minCuts{}   // default is ok, will be filled in ctor body
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     minCuts.emplace_back();
     for_each( graphs.digraph( i ).components(), [&]( MinCutDigraphComponent const & c ) {
       minCuts.back().emplace_back( c, c.rootNode() );
     });
   }
 }

 BkCallback::BkCallback( BkCallback const & other )
   : IloCplex::LazyConstraintCallbackI{ other }
   , config( other.config )
   , graphs( other.graphs ) {
//clog << "BkCallback copy\n";
//clog.flush();
 //, minCuts{}   // DO NOT COPY, so default is ok, will be filled in ctor body
   // We need to create new BkMaxFlow, which are per thread and we have one
   // BkCallback per thread
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     minCuts.emplace_back();
     for_each( graphs.digraph( i ).components(), [&]( MinCutDigraphComponent const & c ) {
       minCuts.back().emplace_back( c, c.rootNode() );
     });
   }
 }

 IloCplex::CallbackI * BkCallback::duplicateCallback() const {
//clog << "BkCallback duplicate\n";
//clog.flush();
   return new (getEnv()) BkCallback( *this );
 }

 void BkCallback::main() {
   assert( graphs.numGraphs() == 2 );
   assert( minCuts.size() == 2 );
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     processGraph( i );
   }
 }

 void BkCallback::constructNonZeroComponents( GraphComponent const         & component
                                            , MinCutDigraphComponent const & digraph
                                            , GraphVariables const         & vars
                                            , IloNumArray                    x_values
                                            , NodeMatrix                   & nodesPerNonZeroComponent
                                            ) const {
   using NodeQueue = std::queue< GraphComponent::Node >;
   NodeQueue queue;

   // -3 queued
   // -2 unvisited
   // -1 visited and zero
   // >= 0 visited and non-zero
   //auto & compMap =  digraph.getNonZeroComponentCache();
   //component.fill( compMap, -2 );
   auto compMap = component.createNodeMap< int >( -2 );

   int compIdx = 0;
   for ( GraphComponent::Node n : component.nodes() ) {
     Graph::Node gn{ component, n };
     if ( compMap[n] == -2 && intIsNonZero( x_values[vars.nodeToIndex[gn]] ) ) {
       queue.push( n );
       compMap[n] = -3;
       nodesPerNonZeroComponent.emplace_back();

       // perform bfs
       while ( !queue.empty() ) {
         GraphComponent::Node n = queue.front();
         assert( compMap[n] == -3 );

         queue.pop();
         compMap[n] = compIdx;

         nodesPerNonZeroComponent.back().push_back( n );

         for ( GraphComponent::Edge e : component.incEdges( n ) ) {
           GraphComponent::Node nn = component.oppositeNode( n, e );
           Graph::Node gnn{ component, nn };

           if ( compMap[nn] == -2 ) {
             if ( intIsNonZero( x_values[vars.nodeToIndex[gnn]] ) ) {
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

 void BkCallback::processGraph( int index ) {
   Graph const          & graph         = graphs.graph( index );
   GraphVariables const & vars          = graphs.variables( index );
   MinCutDigraph const  & digraph       = graphs.digraph( index );
   PerGraphMinCut       & graphMinCuts  = minCuts[index];

   IloNumArray x_values{ getEnv(), vars.xVars.getSize() };
   getValues( x_values, vars.xVars );

   IloNumArray y_values{ getEnv(), vars.yVars.getSize() };
   getValues( y_values, vars.yVars );

   //printNonZeroX( x_values, index );
   //printNonZeroY( y_values, index );

   int nCuts = 0;
   int nBackCuts = 0;
   int nNestedCuts = 0;
   //for_each( graph.components(), [&]( GraphComponent const & component ) {
   for ( auto const & component : graph.components() ) {
     auto & dgComponent = digraph.component( component.index() );
     auto & bkMaxFlow   = graphMinCuts[component.index()];

     NodeMatrix nonZeroNodesComponents;
     constructNonZeroComponents( component, dgComponent, vars, x_values, nonZeroNodesComponents );

     // update capacities for current component
     dgComponent.updateCapacities( x_values, y_values, bkMaxFlow.capacities );

     // now iterate over the non zero components
     for ( auto const & nonZeroNodes : nonZeroNodesComponents ) {
       assert( nonZeroNodes.size() > 0 );

       GraphComponent::Node i = nonZeroNodes.front();
       Graph::Node gn{ component, i };
       double x_i_value = x_values[vars.nodeToIndex[gn]];
       assert( intIsNonZero( x_i_value ) );
       process( component
              , nonZeroNodes
              , x_i_value
              , dgComponent
              , bkMaxFlow
              , x_values, y_values
              , vars
              , nCuts, nNestedCuts, nBackCuts
              );

       // when we consider only non-zero components (from smallest x_i value to largest)
       // then we don't need to reset capacities to their initial values
       //dgComponent.updateCapacities( x_values, y_values, bkMaxFlow.capacities );
     }
   }
   x_values.end();
   y_values.end();
   //clog << "Generated " << nCuts
   //     << " cuts of which " << nBackCuts << " are back-cuts and "
   //     << nNestedCuts << " are nested cuts" << endl;
 }

 void BkCallback::process( GraphComponent const         & component
                         , NodeVector const             & nodes
                         , double                         x_i_value
                         , MinCutDigraphComponent const & digraph
                         , BkMaxFlow                    & bkMaxFlow
                         , IloNumArray                    x_values
                         , IloNumArray                    y_values
                         , GraphVariables const         & vars
                         , int                          & nCuts
                         , int                          & nNestedCuts
                         , int                          & nBackCuts
                         ) {
   GraphComponent::Node i = nodes.front();
   auto target = digraph.toExtNode( i ).second;
   bkMaxFlow.reset( target );

   bool first     = true;
   bool nestedCut = false;
   Graph::Node gn{ component, i };

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
     // TODO: do I need int or cut here?
     if ( intLessThanEq( x_i_value, minCutValue ) ) {
       break;
     }
     //clog << x_i_value << " <= " << minCutValue << "\t" << nCuts
     //     << "\t" << nBackCuts << "\t" << nNestedCuts << endl;

     // determine N (forward)
     set< Graph::Node > fwdS, fwdDS;
     determineFwdCutSet( component, fwdDS, fwdS, digraph, bkMaxFlow, target );

     // add violated constraints
     for ( GraphComponent::Node n : nodes ) {
       Graph::Node gn{ component, n };
       addViolatedConstraint( gn, fwdDS, fwdS, vars );
     }

     nCuts++;
     if ( nestedCut ) {
       nNestedCuts++;
     }

     set< Graph::Node > bwdS, bwdDS;
     determineBwdCutSet( component, bwdDS, bwdS, digraph, bkMaxFlow, target );

     if ( fwdDS.size() != bwdDS.size() || fwdS.size() != bwdS.size()
       || fwdDS != bwdDS || bwdS != fwdS
        ) {

       for ( GraphComponent::Node n : nodes ) {
         Graph::Node gn{ component, n };
         addViolatedConstraint( gn, bwdDS, bwdS, vars );
       }

       nBackCuts++;
       nCuts++;
       if ( nestedCut ) {
         nNestedCuts++;
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

 void BkCallback::addViolatedConstraint( Graph::Node target
                                       , set< Graph::Node > const & dS
                                       , set< Graph::Node > const & S
                                       , GraphVariables const & vars
                                       ) {
   // there is a min-cut that is non-zero and even bigger than x_i_value,
   // therefore the cut can never ever be empty [because it would be zero then ;)]
   assert( !(dS.empty() && S.empty()) );
   IloExpr expr(getEnv());
   //bool first = true;
   //clog << getNnodes() << ": " << vars.xVars[vars.nodeToIndex[target]].getName() << " <=";
   for_each( dS, [&]( Graph::Node const & n ) {
     expr += vars.xVars[vars.nodeToIndex[n]];
     //clog << (first ? " " : " + ") << vars.xVars[vars.nodeToIndex[n]].getName();
     //first = false;
   });

   //first = true;
   for_each( S, [&]( Graph::Node const & n ) {
     expr += vars.yVars[vars.nodeToIndex[n]];
     //clog << (first ? " " : " + ") << vars.yVars[vars.nodeToIndex[n]].getName();
     //first = false;
   });

   //clog << endl;

   IloConstraint constraint = vars.xVars[vars.nodeToIndex[target]] <= expr;
   add(constraint);
   constraint.end();

   expr.end();
 }

 void BkCallback::determineFwdCutSet( GraphComponent const & component
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
   }

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

 void BkCallback::determineBwdCutSet( GraphComponent const &  component
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
   }

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

 void BkCallback::printNonZeroX( IloNumArray x_values, int graphIndex ) const {
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

 void BkCallback::printNonZeroY( IloNumArray y_values, int graphIndex ) const {
   for ( Graph::Node const & i : graphs.graph( graphIndex ).nodes() ) {
     int nodeIndex = graphs.variables( graphIndex ).nodeToIndex[i];
     double y_i_value = y_values[nodeIndex];
     clog << graphs.variables( graphIndex ).yVars[nodeIndex].getName()
          << " " << y_i_value;
     if ( getDirection( graphs.variables( graphIndex ).yVars[nodeIndex] ) == CPX_BRANCH_UP ) {
       clog << "*";
     }
     clog << endl;
   }
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
