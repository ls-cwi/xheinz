#include "Solver/RoundingCallback.hpp"

#include <ilcplex/ilocplexi.h>
#include <queue>

#include "Math.hpp"
#include "Solver.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 RoundingCallback::RoundingCallback( IloEnv env
                                   , Config const & conf
                                   , ExtChainGraph const & gs
                                   )
   : IloCplex::HeuristicCallbackI{ env }
   , config( conf )
   , graphs( gs )
 //, componentMapVector{} // default initialization ok
 //, degreeMapVector{}   // default initialization ok
   , solutionVar{ env, 0 }
   , solutionVal{ env, 0 } {
   assert( gs.numGraphs() == 2 );

   GraphVariables const & redVars  = graphs.variables( 0 );
   GraphVariables const & blueVars = graphs.variables( 1 );
   LinkGraphVariables const & linkVars = graphs.linkVariables( 0 );

   for ( int i = 0, e = gs.numGraphs(); i != e; ++i ) {
     componentMapVector.emplace_back( gs.graph( i ) );
     //degreeMapVector.emplace_back( gs.graph( i ) );
   }

   solutionVar.add( redVars.xVars );
   solutionVar.add( redVars.yVars );
   solutionVar.add( blueVars.xVars );
   solutionVar.add( blueVars.yVars );
   if ( config.connectivityPercentage != 0 ) {
     for ( int i = 0; i < linkVars.zVars.getSize(); ++i ) {
       solutionVar.add( linkVars.zVars[i] );
     }
     solutionVar.add( linkVars.mVars );
     solutionVar.add( redVars.mmVars );
     solutionVar.add( blueVars.mmVars );
   }

   solutionVal.setSize( solutionVar.getSize() );
 }

 RoundingCallback::RoundingCallback( RoundingCallback const & other )
   : IloCplex::HeuristicCallbackI{ other }
   , config( other.config )
   , graphs( other.graphs )
 //, componentMapVector{}   // DO NOT COPY, so default is ok, will be filled in ctor body
   , solutionVar{ getEnv(), 0 }
   , solutionVal{ getEnv(), 0 } {
   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     componentMapVector.emplace_back( graphs.graph( i ) );
   }
 }

 RoundingCallback::~RoundingCallback() {
   solutionVar.end();
   solutionVal.end();
 }

 IloCplex::CallbackI * RoundingCallback::duplicateCallback() const {
   return new (getEnv()) RoundingCallback( *this );
 }

 void RoundingCallback::main() {
   // first determine components per graph
   std::vector< NodeMatrix > nodesPerComponentVector;

   for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
     assert( 0 <= i && i < 2 );

     nodesPerComponentVector.emplace_back();
     determineComponents( i, nodesPerComponentVector.back() );

     //std::cout << nodesPerComponentVector.back().size();
     //for ( auto & p : nodesPerComponentVector.back() ) {
     //  std::cout << " " << p.first << " (" << p.second.size() << ")";
     //}
     //std::cout << std::endl;
   }

   // now determine best component pairing that satisfies alpha
   IndexPair indices = determinePair( 0
                                    , nodesPerComponentVector[0]
                                    , nodesPerComponentVector[1]
                                    );
   if ( indices.first != -1 && indices.second != -1 ) {
     GraphVariables const & redVars  = graphs.variables( 0 );
     GraphVariables const & blueVars = graphs.variables( 1 );

     double solutionWeight = nodesPerComponentVector[0][indices.first].first
         + nodesPerComponentVector[1][indices.second].first;
     //clog << "Found feasible solution: " << solutionWeight << endl;
     //std::cout << "Graph 1: " << nodesPerComponentVector[0][indices.first].first << std::endl;
     //for ( Graph::Node n : nodesPerComponentVector[0][indices.first].second ) {
     //  std::cout << graphs.graph(0).label( n ) << " " << graphs.graph(0).weight( n ) << std::endl;
     //}
     //std::cout << "Graph 2: " << nodesPerComponentVector[1][indices.second].first << std::endl;
     //for ( Graph::Node n : nodesPerComponentVector[1][indices.second].second ) {
     //  std::cout << graphs.graph(1).label( n ) << " " << graphs.graph(1).weight( n ) << std::endl;
     //}

     obtainSolution( 0
                   , 0
                   , nodesPerComponentVector[0][indices.first].second
                   );

     obtainSolution( 1
                   , redVars.xVars.getSize() + redVars.yVars.getSize()
                   , nodesPerComponentVector[1][indices.second].second
                   );

     if ( config.connectivityPercentage != 0 ) {
       obtainSolution( 0
                     , 2 * blueVars.xVars.getSize() + 2 * redVars.xVars.getSize()
                     , nodesPerComponentVector[0][indices.first].second
                     , nodesPerComponentVector[1][indices.second].second
                     );
     }

     // check LB and UB
     //for ( int i = 0; i < solutionVal.getSize(); ++i ) {
     //  if (!( getLB( solutionVar[i] ) <= solutionVal[i] && solutionVal[i] <= getUB( solutionVar[i]) )) {
     //    std::cout << getLB( solutionVar[i] )
     //              << " <= " << solutionVar[i].getName()
     //              << " = "  << solutionVal[i]
     //              << " <= " << getUB( solutionVar[i] )
     //              << ", LP val: " << getValue( solutionVar[i] ) << std::endl;
     //  }
     //  //if ( solution[i] == 1 ) {
     //  //  std::cout << solutionVar[i].getName() << " = " << solution[i] << "\t" << getValue(solutionVar[i]) << std::endl;
     //  //}
     //  //else if ( isNonZero( getValue(solutionVar[i]) ) ) {
     //  //  std::cout << "* " << solutionVar[i].getName() << " = " << solution[i] << "\t" << getValue(solutionVar[i]) << std::endl;
     //  //}
     //}

     setSolution( solutionVar, solutionVal, solutionWeight );

     //solutionVar.end();
     //solution.end();
     //solutionLB.end();
     //solutionUB.end();
   }
 }

 void RoundingCallback::obtainSolution( int index
                                      , int offset
                                      , NodeVector const & comp1
                                      , NodeVector const & comp2
                                      ) {
   GraphVariables const & redVars  = graphs.variables( index );
   GraphVariables const & blueVars = graphs.variables( index + 1 );
   LinkGraphVariables const & linkVars = graphs.linkVariables( index );

   ThreeWayGraph const & twg = graphs.threeWay( index );
   ThreeWayGraph::BpGraph const & link = twg.link();

   int g1_offset = 0;
   int g2_offset = redVars.xVars.getSize() + redVars.yVars.getSize();
   int z_offset = offset;

   // set z
   for ( ThreeWayGraph::BpEdgeIt e( link ); e != lemon::INVALID; ++e, ++z_offset) {
     int zIdx = linkVars.edgeToIndex[e];
     int x1 = redVars.nodeToIndex[twg.toRegularNode( link.redNode(e) )];
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

 void RoundingCallback::obtainSolution( int index
                                      , int offset
                                      , NodeVector const & comp
                                      ) {
   Graph const & graph = graphs.graph( index );
   GraphVariables const & vars = graphs.variables( index );
   int x_card = static_cast<int>( vars.xVars.getSize() );

   // set x and y to 0
   for ( int i = 0; i < 2 * x_card; ++i ) {
     solutionVal[offset + i] = 0;
   }

   // set x to 1
   int smallest_i = std::numeric_limits<int>::max();
   for ( auto n : comp ) {
     int i = vars.nodeToIndex[n];
     if ( i < smallest_i && graph.weight( n ) > 0 ) {
       smallest_i = i;
     }
     solutionVal[offset + i] = 1;
   }

   // set y to 1
   if ( smallest_i != std::numeric_limits<int>::max() ) {
     solutionVal[offset + x_card + smallest_i] = 1;
   }
 }

 void RoundingCallback::determineComponents( int index
                                           , NodeMatrix & nodesPerComponent
                                           ) {
   Graph const          & graph   = graphs.graph( index );
   GraphVariables const & vars    = graphs.variables( index );
   IntNodeMap           & compMap = componentMapVector[index];
   //IntNodeMap           & degMap  = degreeMapVector[index];

   IloNumArray xVal{ getEnv(), vars.xVars.getSize() };
   getValues( xVal, vars.xVars );

   // -3 queued
   // -2 unvisited
   // -1 visited and w_i < .5
   // >= 0 visited and w_i >= .5
   graph.fill( compMap , -2 );
   //graph.fill( degMap, 0 );

   using NodeQueue = std::queue< Graph::Node >;
   NodeQueue queue;

   int compIdx = 0;
   for ( Graph::Node n : graph.nodes() ) {
     //if ( isNonZero( xVal[vars.nodeToIndex[n]] ) ) {
     //  std::cout << graph.label(n)
     //            << " " << xVal[vars.nodeToIndex[n]]
     //            << " " << graph.weight(n) << std::endl;
     //}

     if ( compMap[n] == -2 && xVal[vars.nodeToIndex[n]] >= 0.5 ) {
       queue.push( n );
       compMap[n] = -3;
       nodesPerComponent.emplace_back( std::make_pair( -std::numeric_limits< double >::max(), NodeVector() ) );

       double weight = 0;
       while ( !queue.empty() ) {
         Graph::Node n = queue.front();
         assert( compMap[n] == -3 );

         //std::cout << graph.label(n)
         //          << " " << compIdx
         //          << " " << xVal[vars.nodeToIndex[n]]
         //          << " " << graph.weight(n) << std::endl;

         queue.pop();
         compMap[n] = compIdx;
         weight += graph.weight( n );
         nodesPerComponent.back().second.push_back( n );

         for ( Graph::Edge e : graph.incEdges( n ) ) {
           Graph::Node nn = graph.oppositeNode( n, e );
           int compMap_nn = compMap[nn];
           double x_val_nn = xVal[vars.nodeToIndex[nn]];
           //if ( isNonZero( x_val_nn ) && ( compMap_nn == -2 || compMap_nn == -3 ) ) {
           //  degMap[n]++;
           //  degMap[nn]++;
           //}
           if ( compMap_nn == -2 ) {
             if ( intIsNonZero( x_val_nn ) ) {
               queue.push( nn );
               compMap[nn] = -3;
             }
             else {
               compMap[nn] = -1;
             }
           }
         }
       }

       nodesPerComponent.back().first = weight;
       ++compIdx;
     }
   }

   xVal.end();
 }

 RoundingCallback::IndexPair RoundingCallback::determinePair( int index
                                                            , NodeMatrix const & nodesPerComponent1
                                                            , NodeMatrix const & nodesPerComponent2
                                                            ) {
   IndexPair res(-1, -1 );
   double maxWeight = hasIncumbent() ? getIncumbentObjValue() : -std::numeric_limits< double >::max();
   //std::cout << "maxWeight : " << maxWeight << std::endl;

   for ( int i = 0; i < static_cast<int>( nodesPerComponent1.size() ); ++i ) {
     for ( int j = 0; j < static_cast<int>( nodesPerComponent2.size() ); ++j ) {
       double w = nodesPerComponent1[i].first + nodesPerComponent2[j].first;
       bool sizeOK = config.size < 0 ||
           ( ( static_cast<int>( nodesPerComponent1[i].second.size() ) <= config.size ) &&
             ( static_cast<int>( nodesPerComponent2[j].second.size() ) <= config.size ) );
       if ( w > maxWeight && sizeOK ) {
         double alpha = computeAlpha( index
                                    , nodesPerComponent1[i].second
                                    , nodesPerComponent2[j].second
                                    );
         //std::cout << "i = " << i << ", j = " << j << ", alpha = " << alpha << ", w = " << w << std::endl;
         if ( alpha >= config.connectivityPercentage ) {
           maxWeight = w;
           res = std::make_pair( i, j );
         }
       }
     }
   }

   return res;
 }

 double RoundingCallback::computeAlpha( int index
                                      , NodeVector const & comp1
                                      , NodeVector const & comp2
                                      ) {
   Graph const         & red  = graphs.graph( index );
   Graph const         & blue = graphs.graph( index + 1 );
   ThreeWayGraph const & twg  = graphs.threeWay( index );

   LinkGraphVariables const & linkVars = graphs.linkVariables( index );

   // present will list if both red and blue nodes
   // are present for a particular link component
   // 0 red
   // 1 blue
   // 2 red & blue
   std::map< int, int > present;
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

   // TODO remove code copy (inside switch branches, and with Solver.cpp)
   double alpha = 0;
   switch ( config.connectivityType ) {
    case Config::ConnectivityType::SumUnits: {
     int x_card = static_cast<int>( comp1.size() + comp2.size() );
     int m_card = 0;

     for ( Graph::Node n : comp1 ) {
       auto rn = twg.toRedNode( n );
       if ( rn != lemon::INVALID && present[linkVars.nodeToIndex[rn]] == 2 ) {
         m_card++;
       }
     }
     for ( Graph::Node n : comp2 ) {
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2 ) {
         m_card++;
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
         m_weight += red.weight( n );
       }
     }
     for ( Graph::Node n : comp2 ) {
       x_weight += blue.weight( n );
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2 ) {
         m_weight += blue.weight( n );
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
         m_card++;
       }
     }
     for ( Graph::Node n : comp2 ) {
       auto bn = twg.toBlueNode( n );
       if ( bn != lemon::INVALID && present[linkVars.nodeToIndex[bn]] == 2
         && twg.blue().weight( n ) >= 0.0
          ) {
         m_card++;
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

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
