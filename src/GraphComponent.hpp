#ifndef xHeinz_GraphComponent_HPP
#define xHeinz_GraphComponent_HPP

#include <string>
#include <memory>

#include <lemon/smart_graph.h>

#include "BaseGraphTypes.hpp"

namespace xHeinz {

 class GraphComponent
   : public impl::GraphBasicTypes< lemon::SmartGraph > {
  public:
   int index() const {   return componentIndex;   }

   std::string const & label ( BaseNode const & n ) const {   return data->labels [n];   }
   double              weight( BaseNode const & n ) const {   return data->weights[n];   }

   ArcIt  arcBegin () const {
     return BaseArcIt { data->topology };
   }
   EdgeIt edgeBegin() const {
     return BaseEdgeIt{ data->topology };
   }
   NodeIt nodeBegin() const {
     return BaseNodeIt{ data->topology };
   }
   InArcIt   inArcBegin  ( BaseNode const & n ) const {
     return BaseInArcIt  { data->topology, n };
   }
   OutArcIt  outArcBegin ( BaseNode const & n ) const {
     return BaseOutArcIt { data->topology, n };
   }
   IncEdgeIt incEdgeBegin( BaseNode const & n ) const {
     return BaseIncEdgeIt{ data->topology, n };
   }

   ArcIt  arcEnd () const {
     return BaseArcIt { lemon::INVALID };
   }
   EdgeIt edgeEnd() const {
     return BaseEdgeIt{ lemon::INVALID };
   }
   NodeIt nodeEnd() const {
     return BaseNodeIt{ lemon::INVALID };
   }
   InArcIt   inArcEnd  ( BaseNode const & n ) const {
     return BaseInArcIt  { lemon::INVALID };
   }
   OutArcIt  outArcEnd ( BaseNode const & n ) const {
     return BaseOutArcIt { lemon::INVALID };
   }
   IncEdgeIt incEdgeEnd( BaseNode const & n ) const {
     return BaseIncEdgeIt{ lemon::INVALID };
   }

   ArcRange  arcs () const {
     return make_iterator_range( arcBegin (), arcEnd () );
   }
   EdgeRange edges() const {
     return make_iterator_range( edgeBegin(), edgeEnd() );
   }
   NodeRange nodes() const {
     return make_iterator_range( nodeBegin(), nodeEnd() );
   }

   InArcRange   inArcs  ( BaseNode const & n ) const {
     return make_iterator_range( inArcBegin  ( n ), inArcEnd  ( n ) );
   }
   OutArcRange  outArcs ( BaseNode const & n ) const {
     return make_iterator_range( outArcBegin ( n ), outArcEnd ( n ) );
   }
   IncEdgeRange incEdges( BaseNode const & n ) const {
     return make_iterator_range( incEdgeBegin( n ), incEdgeEnd( n ) );
   }

   BaseNode u( BaseEdge const & e ) const {   return data->topology.u( e );   }
   BaseNode v( BaseEdge const & e ) const {   return data->topology.v( e );   }

   BaseNode oppositeNode( BaseNode const & n, BaseEdge const & e ) const {
     return data->topology.oppositeNode( n, e );
   }

   int id( BaseArc const  & a ) const {   return data->topology.id( a );   }
   int id( BaseEdge const & e ) const {   return data->topology.id( e );   }
   int id( BaseNode const & n ) const {   return data->topology.id( n );   }

   bool valid( BaseArc const  & a ) const {   return data->topology.valid( a );   }
   bool valid( BaseEdge const & e ) const {   return data->topology.valid( e );   }
   bool valid( BaseNode const & n ) const {   return data->topology.valid( n );   }

   int numArcs () const {   return data->numOfArcs;    }
   int numEdges() const {   return data->numOfEdges;   }
   int numNodes() const {   return data->numOfNodes;   }

   template< typename T, typename ... P >
   ArcMap < T > createArcMap ( P && ... p ) const {
     return ArcMap < T >{ data->topology, std::forward< P >( p )... };
   }

   template< typename T, typename ... P >
   EdgeMap< T > createEdgeMap( P && ... p ) const {
     return EdgeMap< T >{ data->topology, std::forward< P >( p )... };
   }

   template< typename T, typename ... P >
   NodeMap< T > createNodeMap( P && ... p ) const {
     return NodeMap< T >{ data->topology, std::forward< P >( p )... };
   }

   template< typename T >
   void copy( T const & t ) const {   t.copy( data->topology );   }

   template< typename T >
   void fill( T & t, typename T::Value const & v ) const {   t.fill( data->topology, v );   }

   BaseArc lookUpArc( BaseNode const & n0, BaseNode const & n1 ) const {
     return data->arcLookUp( n0, n1 );
   }

  public:
   friend std::ostream & operator<<( std::ostream & out, GraphComponent const & gc ) {
     bool first = true;
     int number = 0;
     for_each( gc.nodes(), [&]( Node const & n ) {
       if ( !first ) {
         out << "\n";
       } else {
         first = false;
       }
       out << ++number << "\t:" << gc.label( n ) << " " << gc.weight( n );
     });
     return out;
   }

  private:
   friend class Graph;

  private:
   // used to make sure base topology is *copyed* (lemon::graphCopy) before
   // creation of the other GraphData fields
   struct BaseGraphData {
    public:
     mutable BaseGraph          topology;
     BaseNodeMap< std::string > labels;
     BaseNodeMap< double >      weights;

    public:
     template< typename LemonGraphType >
     BaseGraphData( LemonGraphType const                                                  & originTopology
                  , typename LemonGraphType::Graph::template NodeMap< std::string > const & originLabels
                  , typename LemonGraphType::Graph::template NodeMap< double > const      & originWeights
                  )
     //: topology{}   // default is ok
       : labels{ topology }
       , weights{ topology } {
       lemon::graphCopy( originTopology, topology )
               .nodeMap( originLabels  , labels   )
               .nodeMap( originWeights , weights  )
               .run();
     }
   };

   struct GraphData
     : public BaseGraphData {
    public:
     int numOfArcs, numOfEdges, numOfNodes;
     BaseArcLookUp arcLookUp;

    public:
     template< typename LemonGraphType >
     GraphData( LemonGraphType const                                                  & originTopology
              , typename LemonGraphType::Graph::template NodeMap< std::string > const & originLabels
              , typename LemonGraphType::Graph::template NodeMap< double > const      & originWeights
              )
       : BaseGraphData{ originTopology, originLabels, originWeights }
       , numOfArcs { lemon::countArcs ( topology ) }
       , numOfEdges{ lemon::countEdges( topology ) }
       , numOfNodes{ lemon::countNodes( topology ) }
       , arcLookUp{ topology } {
     }
   };

  private:
   int componentIndex;
   shared_ptr< GraphData > data;

  private:
   template< typename LemonGraphType >
   GraphComponent( int componentIdx
                 , LemonGraphType const                                                  & originTopology
                 , typename LemonGraphType::Graph::template NodeMap< std::string > const & originLabels
                 , typename LemonGraphType::Graph::template NodeMap< double > const      & originWeights
                 )
     : componentIndex{ componentIdx }
     , data{ make_shared< GraphData >( originTopology, originLabels, originWeights ) } {
   }

  private:
   BaseGraph       & getImpl()       {   return data->topology;   }
   BaseGraph const & getImpl() const {   return data->topology;   }
   BaseNodeMap< std::string >       & getLabels()       {   return data->labels;   }
   BaseNodeMap< std::string > const & getLabels() const {   return data->labels;   }
   BaseNodeMap< double >       & getWeights()       {   return data->weights;   }
   BaseNodeMap< double > const & getWeights() const {   return data->weights;   }
 };

} // namespace xHeinz

#endif // xHeinz_GraphComponent_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
