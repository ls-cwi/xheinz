#ifndef xHeinz_ThreeWayGraph_HPP
#define xHeinz_ThreeWayGraph_HPP

#include <map>
#include <set>
#include <string>
#include <memory>
#include <algorithm>

#include "Graph.hpp"

namespace xHeinz {

 struct ThreeWayGraph {
  public:
   using BpGraph     = lemon::SmartBpGraph;

   using BpArc       = BpGraph::Arc;
   using BpEdge      = BpGraph::Edge;
   using BpNode      = BpGraph::Node;
   using BpArcIt     = BpGraph::ArcIt;
   using BpInArcIt   = BpGraph::InArcIt;
   using BpOutArcIt  = BpGraph::OutArcIt;
   using BpEdgeIt    = BpGraph::EdgeIt;
   using BpIncEdgeIt = BpGraph::IncEdgeIt;
   using BpNodeIt    = BpGraph::NodeIt;

   template< typename T >
   using BpArcMap    = BpGraph::ArcMap < T >;
   template< typename T >
   using BpEdgeMap   = BpGraph::EdgeMap< T >;
   template< typename T >
   using BpNodeMap   = BpGraph::NodeMap< T >;

   using RedNode    = BpGraph::RedNode;
   using RedNodeIt  = BpGraph::RedNodeIt;
   using BlueNode   = BpGraph::BlueNode;
   using BlueNodeIt = BpGraph::BlueNodeIt;

   template< typename T >
   using BpRedNodeMap  = BpGraph::RedNodeMap < T >;
   template< typename T >
   using BpBlueNodeMap = BpGraph::BlueNodeMap< T >;

   using IncEdgeIt = BpGraph::IncEdgeIt;

   using NodeToRedNodeMap    = Graph::NodeMap< RedNode >;
   using NodeToBlueNodeMap   = Graph::NodeMap< BlueNode >;
   using BpRedNodeToNodeMap  = BpRedNodeMap < Graph::Node >;
   using BpBlueNodeToNodeMap = BpBlueNodeMap< Graph::Node >;
   using BpIntNodeMap        = BpGraph::NodeMap< int >;
   using BpIntEdgeMap        = BpGraph::EdgeMap< int >;

  public:
   ThreeWayGraph( Graph & lhs, Graph & rhs )
     : redGraph ( lhs )
     , blueGraph( rhs )
     , data{ make_unique< LinkUncopyableData >() }
     , regularNodeToRedNode { redGraph .createNodeMap< RedNode  >( lemon::INVALID ) }
     , regularNodeToBlueNode{ blueGraph.createNodeMap< BlueNode >( lemon::INVALID ) } {
   }

  public:
   Graph       & red ()       {   return redGraph;    }
   Graph const & red () const {   return redGraph;    }

   Graph       & blue()       {   return blueGraph;   }
   Graph const & blue() const {   return blueGraph;   }

   BpGraph       & link()       {   return data->topology;   }
   BpGraph const & link() const {   return data->topology;   }

   Graph::Node       & toRegularNode( RedNode const  & rn )       {
     return data->redNodeToRegularNode [rn];
   }
   Graph::Node const & toRegularNode( RedNode const  & rn ) const {
     return data->redNodeToRegularNode [rn];
   }

   Graph::Node       & toRegularNode( BlueNode const & bn )       {
     return data->blueNodeToRegularNode[bn];
   }
   Graph::Node const & toRegularNode( BlueNode const & bn ) const {
     return data->blueNodeToRegularNode[bn];
   }

   RedNode        & toRedNode ( Graph::Node const & n )       {   return regularNodeToRedNode [n];   }
   RedNode const  & toRedNode ( Graph::Node const & n ) const {   return regularNodeToRedNode [n];   }

   BlueNode       & toBlueNode( Graph::Node const & n )       {   return regularNodeToBlueNode[n];   }
   BlueNode const & toBlueNode( Graph::Node const & n ) const {   return regularNodeToBlueNode[n];   }

   void setNodeCorrespondence( Graph::Node const & n, RedNode const  & rn ) {
     assert( rn != lemon::INVALID );
     data->redNodeToRegularNode [rn] = n;
     regularNodeToRedNode [n]        = rn;
   }
   void setNodeCorrespondence( Graph::Node const & n, BlueNode const & bn ) {
     assert( bn != lemon::INVALID );
     data->blueNodeToRegularNode[bn] = n;
     regularNodeToBlueNode[n]        = bn;
   }

  private:
   struct LinkUncopyableData {
     BpGraph             topology;
     BpRedNodeToNodeMap  redNodeToRegularNode;
     BpBlueNodeToNodeMap blueNodeToRegularNode;

     LinkUncopyableData()
     //: topology{}   // default is ok
       : redNodeToRegularNode { topology }
       , blueNodeToRegularNode{ topology } {
     }
   };

  private:
   Graph & redGraph;
   Graph & blueGraph;
   std::unique_ptr< LinkUncopyableData > data;
   NodeToRedNodeMap  regularNodeToRedNode;
   NodeToBlueNodeMap regularNodeToBlueNode;
 };

} // namespace xHeinz

#endif // xHeinz_ThreeWayGraph_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
