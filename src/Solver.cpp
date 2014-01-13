#include "Solver.hpp"

#include <limits>

#include "Solver/BkCallback.hpp"
#include "Solver/RoundingCallback.hpp"

#include "Math.hpp"
#include "Verbosity.hpp"

using namespace std;

namespace xHeinz {

 using namespace solver;

 Solver::Solver( ChainGraph const & gs, Config conf )
   : config{ std::move( conf ) }
   , graphs{ gs }
 //, env{}     // default initialization ok
   , model{ env }
   , cplex{ model } {
   initVariables();
   initConstraints();
   initObjectiveFunction( graphs );
 }

 Solver::~Solver() {
   env.end();
 }

 optional< solver::OutputSolution > Solver::solve() {
   if ( g_verbosity < VERBOSE_NON_ESSENTIAL ) {
     cplex.setOut    ( env.getNullStream() );
     cplex.setWarning( env.getNullStream() );
     cplex.setError  ( env.getNullStream() );
   } else {
     cplex.setOut    ( clog );
     cplex.setWarning( clog );
     cplex.setError  ( cerr );
     if ( g_verbosity >= VERBOSE_DEBUG ) {
       cplex.setParam( IloCplex::MIPInterval, 1 );
       cplex.setParam( IloCplex::MIPDisplay , 3 );
     }
   }

   //cplex.exportModel( "model.lp" );

   if ( !setParamsAndSolve() ) {
     if ( cplex.getStatus() != IloAlgorithm::Infeasible ) {
       if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
         clog << "Optimization error, CPLEX status code: " 
              << cplex.getStatus() << endl;
       }
     }
     return none;
   }

   // print m
   //if ( config.connectedPercentage ) {
   //  int numRedNodes  = graphs.threeWay( 0 ).red ().numNodes();
   //  int numBlueNodes = graphs.threeWay( 0 ).blue().numNodes();
   //  for ( int i = 0; i < numRedNodes + numBlueNodes; ++i ) {
   //    cout << mVars[i].getName() << " = "
   //         << cplex.getValue( mVars[i] ) << endl;
   //  }
   //}

   assert( graphs.numGraphs() == 2 );
   Graph const & g1 = graphs.graph( 0 );
   Graph const & g2 = graphs.graph( 1 );
   GraphVariables const & d1 = graphs.variables( 0 );
   GraphVariables const & d2 = graphs.variables( 1 );
   OutputSolution sol;
   NodeVector module1;
   NodeVector module2;
   //double score1 = determineSolution( graphs.graph( 0 ), graphs.variables( 0 ), module1 );
   //double score2 = determineSolution( graphs.graph( 1 ), graphs.variables( 1 ), module2 );
   double score1 = determineSolution( g1, d1, module1 );
   double score2 = determineSolution( g2, d2, module2 );

   OutputSolution::SolutionSet sol1;
   OutputSolution::SolutionSet sol2;

   double alpha = computeAlpha( graphs.threeWay( 0 )
                              , graphs.linkVariables( 0 )
                              , module1, module2
                              , sol1, sol2
                              );

   sol.totalScore = score1 + score2;
   sol.graphsSolutions.emplace_back( score1, std::move( sol1 ) );
   sol.graphsSolutions.emplace_back( score2, std::move( sol2 ) );
   sol.alpha.emplace_back( alpha );

   return sol;
 }

 double Solver::determineSolution( Graph const          & graph
                                 , GraphVariables const & data
                                 , NodeVector           & solution
                                 ) {
   double score = 0;

   solution.clear();
   for ( Graph::Node n : graph.nodes() ) {
     int nodeIndex = data.nodeToIndex[n];
     double x_i_value = cplex.getValue( data.xVars[nodeIndex] );
     if ( isNonZero( x_i_value ) ) {
       score += graph.weight( n );
       solution.push_back( n );
     }
   }

   sort( solution.begin(), solution.end(),
     []( Graph::Node const & a, Graph::Node const & b ) {
         // invert the order, we want maximaly scoring first
         return a.component().weight( a ) > b.component().weight( b );
   } );

   return score;
 }

 void Solver::initGraphVariables( Graph const & graph, GraphVariables & data ) {
   int numNodes = graph.numNodes();

   data.xVars  = IloBoolVarArray{ env, numNodes };
   data.yVars  = IloBoolVarArray{ env, numNodes };
   data.mmVars = IloBoolVarArray{ env, numNodes };

   int i = 0;
   for_each( graph.nodes(), [&]( Graph::Node const & n ) {
     char buf[1024];

     // x_i = 0 if node i is not in the subgraph
     // x_i = 1 if node i is the subgraph
     snprintf( buf, 1024, "x_%s", graph.label( n ).c_str() );
     data.xVars[i].setName( buf );

     snprintf( buf, 1024, "y_%s", graph.label( n ).c_str() );
     data.yVars[i].setName( buf );

     snprintf( buf, 1024, "mm_%s", graph.label( n ).c_str() );
     data.mmVars[i].setName( buf );

     data.nodeToIndex[n] = static_cast< int >(data.indexToNode.size());
     data.indexToNode.push_back( n );

     ++i;
   });
 }

 void Solver::initLinkGraphVariables( ThreeWayGraph const & twg
                                    , LinkGraphVariables  & data
                                    ) {
   Graph const & red  = twg.red ();
   Graph const & blue = twg.blue();
   ThreeWayGraph::BpGraph const & link = twg.link();

   int numBpRedNodes = lemon::countRedNodes( twg.link() );

   data.zVars = IloArray< IloBoolVarArray >( env, numBpRedNodes );
   int i = 0;
   int zIdx = 0;
   for ( ThreeWayGraph::RedNodeIt rn( link )
       ; rn != lemon::INVALID; ++rn, ++i
       ) {
     int numIncEdges = 0;
     for ( ThreeWayGraph::BpIncEdgeIt e( link, rn )
         ; e != lemon::INVALID; ++e
         ) {
       ++numIncEdges;   // count the number of edges incident to rn
     }

     data.zVars[i] = IloBoolVarArray( env, numIncEdges );

     int j = 0;
     for ( ThreeWayGraph::BpIncEdgeIt e( link, rn )
         ; e != lemon::INVALID; ++e, ++j, ++zIdx
         ) {
       ThreeWayGraph::BlueNode bn = link.blueNode( e );
       char buf[1024];
       snprintf( buf, 1024, "z_%s_%s"
               , red .label( twg.toRegularNode( rn ) ).c_str()
               , blue.label( twg.toRegularNode( bn ) ).c_str()
               );
       data.zVars[i][j].setName( buf );
       data.edgeToIndexPair[e] = std::make_pair( i, j );
       data.edgeToIndex[e] = zIdx;
     }
   }

   // first we determine the number of connected components in link
   data.numComponents = lemon::connectedComponents( link, data.nodeToIndex );

   if ( g_verbosity >= VERBOSE_ESSENTIAL ) {
     cout << "-- # components in link graph: " << data.numComponents << endl;
   }

   data.edgesPerComponent = LinkGraphVariables::BpEdgeMatrix( data.numComponents );
   for ( ThreeWayGraph::BpEdgeIt edgeIt( link )
       ; edgeIt != lemon::INVALID; ++edgeIt ) {
     assert( link.u( edgeIt ) != link.v( edgeIt ) &&
             data.nodeToIndex[link.u( edgeIt )] == data.nodeToIndex[link.v( edgeIt )] );
     data.edgesPerComponent[ data.nodeToIndex[link.u( edgeIt )] ].push_back( edgeIt );
   }

   data.mVars = IloBoolVarArray( env, data.numComponents );
   for ( int i = 0; i < data.numComponents ; ++i ) {
     char buf[1024];
     snprintf( buf, 1024, "m_%d", i);
     data.mVars[i].setName( buf );
   }
 }

 void Solver::initGraphConstraints( Graph const & graph, GraphVariables & data ) {
   int numNodes = graph.numNodes();
   IloExpr expr( env );

   // there is at most one root node
   // \sum_{i \in V} y_i <= 1
   for ( int i = 0; i < numNodes; ++i ) {
     expr += data.yVars[i];
   }

   IloConstraint c1;
   model.add( c1 = (expr <= 1) );
   c1.setName( "one_root" );

   // the root node has to be one of the selected nodes
   // in the final graph
   // y_i <= x_i for all nodes i in V
   for ( int i = 0; i < numNodes; ++i ) {
     IloConstraint c2;
     model.add( c2 = (data.yVars[i] <= data.xVars[i]) );
     c2.setName( "root_in_x" );
   }

   for ( int i = 0; i < numNodes; ++i ) {
     double weight = graph.weight( data.indexToNode[i] );
     if ( weight < 0 ) {
       // root node has to be positive
       model.add( data.yVars[i] == 0 );
     } else {
       // there is a root node if one x with positive weight is in the solution
       model.add( expr >= data.xVars[i]);
     }
   }

   // if you pick a node then it must be the root node
   // or at least one of its direct neighbors must be part
   // of the solution as well
   for ( Graph::NodeIt nodeIt = graph.nodeBegin(), nodeEnd = graph.nodeEnd()
       ; nodeIt != nodeEnd; ++nodeIt ) {
     expr.clear();
     for ( Graph::IncEdgeIt edgeIt  = graph.incEdgeBegin( *nodeIt )
                          , edgeEnd = graph.incEdgeEnd  ( *nodeIt )
         ; edgeIt != edgeEnd; ++edgeIt
         ) {
       auto otherEndNode = graph.oppositeNode( *nodeIt, *edgeIt );
       expr += data.xVars[data.nodeToIndex[otherEndNode]];

       // if i is negative then its positive neighbors must be in
       if ( graph.weight( *nodeIt ) < 0 && graph.weight( otherEndNode ) > 0 ) {
         model.add( data.xVars[data.nodeToIndex[*nodeIt]] <= data.xVars[data.nodeToIndex[otherEndNode]] );
       }
     }
     expr += data.yVars[data.nodeToIndex[*nodeIt]];
     model.add( data.xVars[data.nodeToIndex[*nodeIt]] <= expr );
   }

   // if you pick a negative node, then at least one of its direct neighbors
   // must be part of the solution as well
   for ( Graph::NodeIt nodeIt = graph.nodeBegin(), nodeEnd = graph.nodeEnd()
       ; nodeIt != nodeEnd; ++nodeIt
       ) {
     if ( graph.weight( *nodeIt ) <= 0 ) {
       expr.clear();
       for ( Graph::IncEdgeIt edgeIt  = graph.incEdgeBegin( *nodeIt )
                            , edgeEnd = graph.incEdgeEnd  ( *nodeIt )
           ; edgeIt != edgeEnd; ++edgeIt
           ) {
         auto otherEndNode = graph.oppositeNode( *nodeIt, *edgeIt );
         expr += data.xVars[data.nodeToIndex[otherEndNode]];
       }
       model.add( data.xVars[data.nodeToIndex[*nodeIt]] <= expr );
     } else {
       // symmetry breaking
       for ( Graph::NodeIt otherNodeIt = graph.nodeBegin()
           ; otherNodeIt != nodeIt; ++otherNodeIt
           ) {
         if ( graph.weight( *otherNodeIt ) < 0 ) {
           continue;
         }
         model.add( data.yVars[data.nodeToIndex[*nodeIt]]
                 <= 1 - data.xVars[data.nodeToIndex[*otherNodeIt]]
                  );
       }
     }
   }

   // size constraint
   if ( config.size >= 0 ) {
     expr.clear();
     for ( Graph::Node n : graph.nodes() ) {
       expr += data.xVars[data.nodeToIndex[n]];
     }
     model.add( expr <= config.size );
   }
 }

 void Solver::initVariables() {
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     initGraphVariables( graphs.graph( i ), graphs.variables( i ) );
   }

   for ( int i = 0, e = graphs.numGraphs() - 1; i != e; ++i ) {
     initLinkGraphVariables( graphs.threeWay( i ), graphs.linkVariables( i ) );
   }
 }

 void Solver::initLinkGraphConstraints( ThreeWayGraph const  & twg
                                      , GraphVariables const & redData
                                      , GraphVariables const & blueData
                                      , LinkGraphVariables   & data
                                      ) {
   auto const & red  = twg.red ();
   auto const & blue = twg.blue();
   auto const & link = twg.link();

   // objective must be positive, not really helping...
   IloExpr expr( env );
   //for ( int i = 0; i < numRedNodes ; ++i ) {
   //  expr += redData.xVars[i] * red.weight( redData.indexToNode[i] );
   //}
   //for ( int j = 0; j < numBlueNodes ; ++j ) {
   //  expr += blueData.xVars[j] * blue.weight( blueData.indexToNode[j] );
   //}
   //model.add( expr >= 0 );
   //expr.clear();

   if ( !config.connectivityPercentage ) {
     return;
   }

   for ( ThreeWayGraph::BpEdgeIt e( link )
       ; e != lemon::INVALID; ++e
       ) {
     auto & ij = data.edgeToIndexPair[e];
     auto rn = twg.toRegularNode( link.redNode ( e ) );
     auto bn = twg.toRegularNode( link.blueNode( e ) );

     model.add( data.zVars[ij.first][ij.second]
                <= redData .xVars[redData.nodeToIndex[rn]]  );
     model.add( data.zVars[ij.first][ij.second]
                <= blueData.xVars[blueData.nodeToIndex[bn]] );
     model.add( data.zVars[ij.first][ij.second]
                >= redData .xVars[redData.nodeToIndex[rn]]
                   + blueData.xVars[blueData.nodeToIndex[bn]] - 1 );
   }

   for ( ThreeWayGraph::BpEdgeIt edgeIt( link )
       ; edgeIt != lemon::INVALID; ++edgeIt
       ) {
     auto node = link.u( edgeIt );
     auto & ij = data.edgeToIndexPair[edgeIt];

     // m_cog(i,j) >= zij
     model.add( data.mVars[data.nodeToIndex[node]]
                >= data.zVars[ij.first][ij.second] );
   }

   // m_K <= sum_{ij in K} z_ij
   for ( int i = 0; i < data.numComponents; ++i ) {
     for ( auto edgeIt = data.edgesPerComponent[i].begin()
         ; edgeIt != data.edgesPerComponent[i].end()
         ; ++edgeIt ) {
       auto & ij = data.edgeToIndexPair[*edgeIt];
       expr += data.zVars[ij.first][ij.second];
     }
     model.add( data.mVars[i] <= expr );
     expr.clear();
   }

   // mm_i <= m_comp(i)
   // mm_i <= x_i
   for ( Graph::Node node : red.nodes() ) {
     int i = redData.nodeToIndex[node];
     auto bpNode = twg.toRedNode( node );
     if ( bpNode != lemon::INVALID ) {
       int comp_i = data.nodeToIndex[bpNode];

       model.add( redData.mmVars[i] <= data.mVars[comp_i] );
       model.add( redData.mmVars[i] <= redData.xVars[i] );
     } else {
       if ( config.connectivityPercentage == 1 ) {
         model.add( redData.xVars[i] == 0 );
       }
       model.add( redData.mmVars[i] == 0 );
     }
   }
   for ( Graph::Node node : blue.nodes() ) {
     int j = blueData.nodeToIndex[node];
     auto bpNode = twg.toBlueNode( node );
     if ( bpNode != lemon::INVALID ) {
       int comp_j = data.nodeToIndex[bpNode];

       model.add( blueData.mmVars[j] <= data.mVars[comp_j] );
       model.add( blueData.mmVars[j] <= blueData.xVars[j] );
     } else {
       if ( config.connectivityPercentage == 1 ) {
         model.add( blueData.xVars[j] == 0 );
       }
       model.add( blueData.mmVars[j] == 0 );
     }
   }

   IloExpr sumRedWeights( env )         , sumBlueWeights( env );
   IloExpr sumRedConnectedWeights( env ), sumBlueConnectedWeights( env );
   int i = 0;
   for_each( red.nodes(), [&]( Graph::Node const & node ) {
     switch( config.connectivityType ) {
      case Config::ConnectivityType::SumUnits: {
       sumRedWeights += redData.xVars[i];
       sumRedConnectedWeights += redData.mmVars[i];
      } break;
      case Config::ConnectivityType::SumWeights: {
       sumRedWeights += redData.xVars[i] * red.weight( node );
       sumRedConnectedWeights += redData.mmVars[i] * red.weight( node );
      } break;
      case Config::ConnectivityType::SumPosUnits: {
       if ( red.weight( node ) >= 0.0 ) {
         sumRedWeights += redData.xVars[i];
         sumRedConnectedWeights += redData.mmVars[i];
       }
      } break;
      case Config::ConnectivityType::SumPosWeights: {
       if ( red.weight( node ) >= 0.0 ) {
         sumRedWeights += redData.xVars[i] * red.weight( node );
         sumRedConnectedWeights += redData.mmVars[i] * red.weight( node );
       }
      } break;
     }
     ++i;
   });
   i = 0;
   for_each( blue.nodes(), [&]( Graph::Node const & node ) {
     switch( config.connectivityType ) {
      case Config::ConnectivityType::SumUnits: {
       sumBlueWeights += blueData.xVars[i];
       sumBlueConnectedWeights += blueData.mmVars[i];
      } break;
      case Config::ConnectivityType::SumWeights: {
       sumBlueWeights += blueData.xVars[i] * blue.weight( node );
       sumBlueConnectedWeights += blueData.mmVars[i] * blue.weight( node );
      } break;
      case Config::ConnectivityType::SumPosUnits: {
       if ( blue.weight( node ) >= 0.0 ) {
         sumBlueWeights += blueData.xVars[i];
         sumBlueConnectedWeights += blueData.mmVars[i];
       }
      } break;
      case Config::ConnectivityType::SumPosWeights: {
       if ( blue.weight( node ) >= 0.0 ) {
         sumBlueWeights += blueData.xVars[i] * blue.weight( node );
         sumBlueConnectedWeights += blueData.mmVars[i] * blue.weight( node );
       }
      } break;
     }
     ++i;
   });
   model.add( (sumRedConnectedWeights + sumBlueConnectedWeights)
              >= config.connectivityPercentage*(sumRedWeights + sumBlueWeights)
            );
 }

 void Solver::initConstraints() {
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     initGraphConstraints( graphs.graph( i ), graphs.variables( i ) );
   }

   for ( int i = 0, e = graphs.numGraphs() - 1; i != e; ++i ) {
     initLinkGraphConstraints( graphs.threeWay( i )
                             , graphs.variables( i )
                             , graphs.variables( i + 1 )
                             , graphs.linkVariables( i )
                             );
   }
 }

 void Solver::initObjectiveFunction( solver::ExtChainGraph const & graphs ) {
   IloExpr expr( env );
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     Graph const & graph = graphs.graph( i );
     GraphVariables const & data  = graphs.variables( i );

     for ( int j = 0; j < graph.numNodes(); ++j ) {
       expr += data.xVars[j] * graph.weight( data.indexToNode[j] );
     }
   }
   model.add( IloObjective( env, expr, IloObjective::Maximize ) );
 }

 bool Solver::setParamsAndSolve() {
   // I (Mohammed) find this a bit dangerous, we don't know what cplex is doing
   // exactly under the hood and our epsilon is much smaller than cplex'
   //cplex.setParam( IloCplex::EpLin, epsilon );

   if ( config.timeLimit > 0 ) {
     cplex.setParam( IloCplex::TiLim, config.timeLimit );
   }

   if ( config.numThreads > 1 ) {
     cplex.setParam( IloCplex::ParallelMode, -1 );
     cplex.setParam( IloCplex::Threads, config.numThreads );
   }

   cplex.setParam( IloCplex::HeurFreq      , -1 );
   cplex.setParam( IloCplex::Cliques       , -1 );
   cplex.setParam( IloCplex::Covers        , -1 );
   cplex.setParam( IloCplex::FlowCovers    , -1 );
   cplex.setParam( IloCplex::GUBCovers     , -1 );
   cplex.setParam( IloCplex::FracCuts      , -1 );
   cplex.setParam( IloCplex::MIRCuts       , -1 );
   cplex.setParam( IloCplex::FlowPaths     , -1 );
   cplex.setParam( IloCplex::ImplBd        , -1 );
   cplex.setParam( IloCplex::DisjCuts      , -1 );
   cplex.setParam( IloCplex::ZeroHalfCuts  , -1 );
   cplex.setParam( IloCplex::MCFCuts       , -1 );
   cplex.setParam( IloCplex::AggFill       ,  0 );
   cplex.setParam( IloCplex::PreInd        ,  0 );
   cplex.setParam( IloCplex::RelaxPreInd   ,  0 );
   cplex.setParam( IloCplex::PreslvNd      , -1 );
   cplex.setParam( IloCplex::RepeatPresolve,  0 );

   IloCplex::Callback cutCallback( new (env) BkCallback( env, config, graphs ) );
   IloCplex::Callback roundingCallback( new (env) RoundingCallback( env, config, graphs ) );

   cplex.use( cutCallback );
   cplex.use( roundingCallback );

   bool res = cplex.solve();

   cutCallback.end();
   roundingCallback.end();

   return res;
 }

 double Solver::computeAlpha( ThreeWayGraph const         & twg
                            , LinkGraphVariables const    & linkVars
                            , NodeVector const            & comp1
                            , NodeVector const            & comp2
                            , OutputSolution::SolutionSet & out1
                            , OutputSolution::SolutionSet & out2
                            ) {
   Graph const & red  = twg.red();
   Graph const & blue = twg.blue();
   out1.clear();
   out2.clear();
   out1.reserve( comp1.size() );
   out2.reserve( comp2.size() );

   // present will list if both red and blue nodes
   // are present for a particular link component
   // 0 red
   // 1 blue
   // 2 red & blue
   map< int, int > present;
   for ( Graph::Node n : comp1 ) {
     auto rn = twg.toRedNode( n );
     if ( rn != lemon::INVALID ) {
       int compIdx = linkVars.nodeToIndex[rn];
       present[compIdx] = 0;
     }
   }
   for ( Graph::Node n : comp2 ) {
     auto bn = twg.toBlueNode( n );
     if ( bn != lemon::INVALID ) {
       int compIdx = linkVars.nodeToIndex[bn];
       if ( present.find( compIdx ) == present.end()  ) {
         present[compIdx] = 1;
       }
       else if ( present[compIdx] == 0 ) {
         present[compIdx] = 2;
       }
     }
   }

   // TODO remove code copy (inside switch branches, and with RoundingCallback.cpp)
   double alpha = 0;
   switch( config.connectivityType ) {
    case Config::ConnectivityType::SumUnits: {
     int x_card = static_cast<int>( comp1.size() + comp2.size() );
     int m_card = 0;

     for ( Graph::Node n : comp1 ) {
       auto rn = twg.toRedNode( n );
       if ( rn != lemon::INVALID && present[linkVars.nodeToIndex[rn]] == 2 ) {
         out1.emplace_back( n, true );
         m_card++;
       } else {
         out1.emplace_back( n, false );
       }
     }
     for ( Graph::Node n : comp2 ) {
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2 ) {
         out2.emplace_back( n, true );
         m_card++;
       } else {
         out2.emplace_back( n, false );
       }
     }
     alpha = static_cast<double>( m_card ) / static_cast<double>( x_card );
    } break;
    case Config::ConnectivityType::SumWeights: {
     double x_weight = 0;
     double m_weight = 0;

     for ( Graph::Node n : comp1 ) {
       x_weight += red.weight( n );
       auto rn = twg.toRedNode( n );
       if ( rn != lemon::INVALID && present[linkVars.nodeToIndex[rn]] == 2 ) {
         out1.emplace_back( n, true );
         m_weight += red.weight( n );
       } else {
         out1.emplace_back( n, false );
       }
     }
     for ( Graph::Node n : comp2 ) {
       x_weight += blue.weight( n );
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2 ) {
         out2.emplace_back( n, true );
         m_weight += blue.weight( n );
       } else {
         out2.emplace_back( n, false );
       }
     }
     alpha = m_weight / x_weight;
    } break;
    case Config::ConnectivityType::SumPosUnits: {
     int x_card = static_cast<int>( comp1.size() + comp2.size() );
     int m_card = 0;

     for ( Graph::Node n : comp1 ) {
       auto rn = twg.toRedNode( n );
       if ( rn != lemon::INVALID && present[linkVars.nodeToIndex[rn]] == 2
         && twg.red().weight( n ) >= 0.0
          ) {
         out1.emplace_back( n, true );
         m_card++;
       } else {
         out1.emplace_back( n, false );
       }
     }
     for ( Graph::Node n : comp2 ) {
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2
         && twg.blue().weight( n ) >= 0.0
          ) {
         out2.emplace_back( n, true );
         m_card++;
       } else {
         out2.emplace_back( n, false );
       }
     }
     alpha = static_cast<double>( m_card ) / static_cast<double>( x_card );
    } break;
    case Config::ConnectivityType::SumPosWeights: {
     double x_weight = 0;
     double m_weight = 0;

     for ( Graph::Node n : comp1 ) {
       x_weight += red.weight( n );
       auto rn = twg.toRedNode( n );
       if ( rn != lemon::INVALID && present[linkVars.nodeToIndex[rn]] == 2
         && twg.red().weight( n ) >= 0.0
          ) {
         m_weight += red.weight( n );
       }
     }
     for ( Graph::Node n : comp2 ) {
       x_weight += blue.weight( n );
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2
         && twg.blue().weight( n ) >= 0.0
          ) {
         m_weight += blue.weight( n );
       }
     }
     alpha = m_weight / x_weight;
    } break;
   }
   return alpha;
 }

 namespace {

  void SetBpGraphCplexVarsValues( Graph const & red
                                , GraphVariables const & redVars
                                , Graph const & blue
                                , GraphVariables const & blueVars
                                , ThreeWayGraph const & twg
                                , LinkGraphVariables const & linkVars
                                , InputSolution::GraphSolution const & sol0
                                , InputSolution::GraphSolution const & sol1
                                , int offset
                                , IloNumVarArray & solutionVar
                                , IloNumArray    & solutionVal
                                ) {
    ThreeWayGraph::BpGraph const & link = twg.link();

    int g1_offset = 0;
    int g2_offset = redVars.xVars.getSize() + redVars.yVars.getSize();
    int z_offset  = offset;

    // set z
    for ( ThreeWayGraph::BpEdgeIt e( link ); e != lemon::INVALID; ++e, ++z_offset) {
      int zIdx = linkVars.edgeToIndex[e];
      int x1 = redVars .nodeToIndex[twg.toRegularNode( link.redNode (e) )];
      int x2 = blueVars.nodeToIndex[twg.toRegularNode( link.blueNode(e) )];
      solutionVal[offset + zIdx] = solutionVal[g1_offset + x1] * solutionVal[g2_offset + x2];
    }

    // set mm to 0
    int m_offset = z_offset + static_cast<int>( linkVars.edgesPerComponent.size() );
    for ( int i = 0; i < static_cast<int>( redVars.mmVars.getSize() + blueVars.mmVars.getSize() ); ++i ) {
      solutionVal[m_offset + i] = 0;
    }

    // set m
    for ( int i = 0, e = static_cast<int>( linkVars.edgesPerComponent.size() ); i < e; ++i) {
      bool found = false;
      for ( ThreeWayGraph::BpEdge edge : linkVars.edgesPerComponent[i] ) {
        int x1 = redVars.nodeToIndex[twg.toRegularNode( link.redNode( edge ) )];
        int x2 = blueVars.nodeToIndex[twg.toRegularNode( link.blueNode( edge ) )];

        if ( solutionVal[g1_offset + x1] * solutionVal[g2_offset + x2] == 1 ) {
          found = true;
          // set mm
          solutionVal[m_offset + x1] = 1;
          solutionVal[m_offset + static_cast<int>( redVars.mmVars.getSize() ) + x2] = 1;
        }
      }

      if ( found ) {
        solutionVal[z_offset + i] = 1;
      } else {
        solutionVal[z_offset + i] = 0;
      }
    }
  }

  void SetGraphCplexVarsValue( Graph const & graph
                             , GraphVariables const & vars
                             , InputSolution::GraphSolution const & sol
                             , int offset
                             , IloNumVarArray & solutionVar
                             , IloNumArray    & solutionVal
                             ) {
    int x_card = static_cast<int>( vars.xVars.getSize() );

    // set x and y to 0
    for ( int i = 0; i < 2 * x_card; ++i ) {
      solutionVal[offset + i] = 0;
    }

    // set x to 1
    int smallest_i = numeric_limits< int >::max();
    for_each( sol, [&]( Graph::Node const & n ) {
      int i = vars.nodeToIndex[n];
      solutionVal[offset + i] = 1;
      if ( i < smallest_i && graph.weight( n ) > 0 ) {
        smallest_i = i;
      }
    });

    // set y to 1
    if ( smallest_i != std::numeric_limits< int >::max() ) {
      solutionVal[offset + x_card + smallest_i] = 1;
    }
  }

 } // namespace <anonymous>

 void Solver::warm( InputSolution const & sol, char const * name ) {
   assert( graphs.numGraphs() == 2 );

   Graph const & red  = graphs.graph( 0 );
   Graph const & blue = graphs.graph( 1 );
   GraphVariables const & redVars  = graphs.variables( 0 );
   GraphVariables const & blueVars = graphs.variables( 1 );
   LinkGraphVariables const & linkVars = graphs.linkVariables( 0 );

   // Create Cplex variables
   IloNumVarArray solutionVars{ env, 0 };
   IloNumArray solutionVarValues{ env, 0 };

   solutionVars.add( redVars.xVars.toNumVarArray() );
   solutionVars.add( redVars.yVars.toNumVarArray() );
   solutionVars.add( blueVars.xVars.toNumVarArray() );
   solutionVars.add( blueVars.yVars.toNumVarArray() );
   if ( config.connectivityPercentage != 0 ) {
     for ( int i = 0; i < linkVars.zVars.getSize(); ++i ) {
       solutionVars.add( linkVars.zVars[i].toNumVarArray() );
     }
     solutionVars.add( linkVars.mVars.toNumVarArray() );
     solutionVars.add( redVars.mmVars.toNumVarArray() );
     solutionVars.add( blueVars.mmVars.toNumVarArray() );
   }
   solutionVarValues.setSize( solutionVars.getSize() );

   // Set Cplex variables values
   SetGraphCplexVarsValue( red, redVars
                         , sol.graphsSolutions[0]
                         , 0
                         , solutionVars, solutionVarValues
                         );
   SetGraphCplexVarsValue( blue, blueVars
                         , sol.graphsSolutions[1]
                         , redVars.xVars.getSize() + redVars.yVars.getSize()
                         , solutionVars, solutionVarValues
                         );
   if ( config.connectivityPercentage != 0 ) {
     SetBpGraphCplexVarsValues( red, redVars, blue, blueVars
                              , graphs.threeWay( 0 ), graphs.linkVariables( 0 )
                              , sol.graphsSolutions[0], sol.graphsSolutions[1]
                              , 2 * blueVars.xVars.getSize() + 2 * redVars.xVars.getSize()
                              , solutionVars, solutionVarValues
                              );
   }

   cplex.addMIPStart( solutionVars, solutionVarValues, IloCplex::MIPStartAuto, name );

   cplex.setParam( IloCplex::AdvInd, 2 );
 }

} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
