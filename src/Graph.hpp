#ifndef xHeinz_Graph_HPP
#define xHeinz_Graph_HPP

#include <cassert>

#include <string>
#include <vector>

#include <lemon/adaptors.h>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>

#include "GraphComponent.hpp"

namespace xHeinz {

 class GraphBuilder;

 class Graph
   : private impl::GraphBasicTypes< lemon::SmartGraph > {
  private:
   using MyBase = impl::GraphBasicTypes< lemon::SmartGraph >;

  public:
   using ComponentIt      = std::vector< GraphComponent >::iterator;
   using ConstComponentIt = std::vector< GraphComponent >::const_iterator;

   using ComponentRange      = iterator_range< ComponentIt      >;
   using ConstComponentRange = iterator_range< ConstComponentIt >;

   template< typename ImplType >
   struct DataStruct {
    public:
     DataStruct()
       : comp{ nullptr } {
     //, data{}   // default is ok
     }

     DataStruct( GraphComponent const & c, ImplType d )
       : comp{ &c }
       , data{ std::move( d ) } {
     }

    public:
     GraphComponent const & component() const {   assert( comp );   return *comp;   }

     int componentIndex() const {   return component().index();   }

     operator ImplType() const {   return data;   }

     friend bool operator==( DataStruct const & lhs, DataStruct const & rhs ) {
       return lhs.data == rhs.data;
     }

     friend bool operator<( DataStruct const & lhs, DataStruct const & rhs ) {
       return lhs.data < rhs.data;
     }

    private:
     GraphComponent const * comp;
     ImplType data;
   };
   using Arc  = DataStruct< GraphComponent::Arc  >;
   using Edge = DataStruct< GraphComponent::Edge >;
   using Node = DataStruct< GraphComponent::Node >;

   template< typename Pol >
   struct StandardIter
     : public iterator_facade< StandardIter< Pol >
                             , typename Pol::ValueType const
                             , forward_traversal_tag
                             > {
    public:
     static struct ValidTag_t   {   } ValidTag;
     static struct InvalidTag_t {   } InvalidTag;

    public:
     StandardIter( Graph const & g, ValidTag_t )
       : graph{ &g }
       , currentCompIdx{ 0 }
       , compPolTypeIt{ Pol::begin( component() ) } {
     //, data{}   // will be initialized in checkEndIterator()
       checkEndIterator();
     }

     StandardIter( Graph const & g, InvalidTag_t )
       : graph{ &g }
       , currentCompIdx{ -1 }
       , compPolTypeIt{ Pol::end( component() ) } {
     //, data{}   // invalid data for invalid iter
     }

    public:
     int componentIndex() const {   return currentCompIdx;   }

     GraphComponent const & component() const {   return graph->component( componentIndex() );   }

    private:
     friend class boost::iterator_core_access;
     friend class Graph;

    private:
     Graph const * graph;
     int currentCompIdx;
     typename Pol::ItType compPolTypeIt;
     typename Pol::ValueType data;

    private:
     void increment() {
       ++compPolTypeIt;
       checkEndIterator();
     }

     bool equal( StandardIter const & rhs ) const {
       return currentCompIdx == rhs.currentCompIdx   // first check idx for fast end check
           && graph == rhs.graph && compPolTypeIt == rhs.compPolTypeIt;
     }

     typename Pol::ValueType const & dereference() const {
       assert( currentCompIdx != -1 );
       return data;
     }

    private:
     void checkEndIterator() {
       while ( compPolTypeIt == Pol::end( component() ) ) {
         if ( (++currentCompIdx) == graph->numComponents() ) {
           currentCompIdx = -1;   // set end
           compPolTypeIt = typename Pol::ItType{};
           return;
         } else {
           compPolTypeIt = Pol::begin( component() );
         }
       }
       assert( compPolTypeIt != Pol::end( component() ) );
       data = typename Pol::ValueType{ component(), *compPolTypeIt };
     }
   };

   struct ArcItPol {
     using ValueType = Arc ; using ItType = GraphComponent::ArcIt;
     static ItType begin( GraphComponent const & c ) {   return c.arcBegin();    }
     static ItType end  ( GraphComponent const & c ) {   return c.arcEnd  ();    }
   };
   struct EdgeItPol {
     using ValueType = Edge; using ItType = GraphComponent::EdgeIt;
     static ItType begin( GraphComponent const & c ) {   return c.edgeBegin();   }
     static ItType end  ( GraphComponent const & c ) {   return c.edgeEnd  ();   }
   };
   struct NodeItPol {
     using ValueType = Node; using ItType = GraphComponent::NodeIt;
     static ItType begin( GraphComponent const & c ) {   return c.nodeBegin();   }
     static ItType end  ( GraphComponent const & c ) {   return c.nodeEnd  ();   }
   };
   using ArcIt  = StandardIter< ArcItPol  >;
   using EdgeIt = StandardIter< EdgeItPol >;
   using NodeIt = StandardIter< NodeItPol >;


   template< typename Pol >
   struct SpecialIter
     : public iterator_facade< SpecialIter< Pol >
                             , typename Pol::ValueType const
                             , forward_traversal_tag
                             > {
    public:
     static struct ValidTag_t   {   } ValidTag;
     static struct InvalidTag_t {   } InvalidTag;

    public:
     SpecialIter( Graph const & g, Node n, ValidTag_t )
       : graph{ &g }
       , startNode{ std::move( n ) }
       , compPolTypeIt{ Pol::begin( component(), startNode ) } {
     //, data{}   // will be initialized in checkEndIterator()
       checkEndIterator();
     }

     SpecialIter( Graph const & g, Node n, InvalidTag_t )
       : graph{ &g }
       , startNode{ std::move( n ) }
       , compPolTypeIt{ Pol::end( component(), startNode ) } {
     //, data{}   // invalid data for invalid iter
     }

    public:
     int componentIndex() const {   return component().index();   }

     GraphComponent const & component() const {   return startNode.component();   }

    private:
     friend class boost::iterator_core_access;
     friend class Graph;

    private:
     Graph const * graph;
     Node startNode;
     typename Pol::ItType compPolTypeIt;
     typename Pol::ValueType data;

    private:
     void increment() {
       ++compPolTypeIt;
       checkEndIterator();
     }

     bool equal( SpecialIter const & rhs ) const {
       return graph == rhs.graph && startNode == rhs.startNode
                && compPolTypeIt == rhs.compPolTypeIt;
     }

     typename Pol::ValueType const & dereference() const {
       return data;
     }

    private:
     void checkEndIterator() {
       if ( compPolTypeIt != Pol::end( component(), startNode ) ) {
         data = typename Pol::ValueType{ component(), *compPolTypeIt };
       }
     }
   };

   struct InArcItPol {
     using ValueType = Arc; using ItType = GraphComponent::InArcIt;
     static ItType begin( GraphComponent const & c, Node const & n ) {   return c.inArcBegin( n );   }
     static ItType end  ( GraphComponent const & c, Node const & n ) {   return c.inArcEnd  ( n );   }
   };
   struct OutArcItPol {
     using ValueType = Arc; using ItType = GraphComponent::OutArcIt;
     static ItType begin( GraphComponent const & c, Node const & n ) {   return c.outArcBegin( n );   }
     static ItType end  ( GraphComponent const & c, Node const & n ) {   return c.outArcEnd  ( n );   }
   };
   struct IncEdgeItPol {
     using ValueType = Edge; using ItType = GraphComponent::IncEdgeIt;
     static ItType begin( GraphComponent const & c, Node const & n ) {   return c.incEdgeBegin( n );   }
     static ItType end  ( GraphComponent const & c, Node const & n ) {   return c.incEdgeEnd  ( n );   }
   };
   using InArcIt   = SpecialIter< InArcItPol   >;
   using OutArcIt  = SpecialIter< OutArcItPol  >;
   using IncEdgeIt = SpecialIter< IncEdgeItPol >;

   using ArcRange  = iterator_range< ArcIt  >;
   using EdgeRange = iterator_range< EdgeIt >;
   using NodeRange = iterator_range< NodeIt >;

   using InArcRange   = iterator_range< InArcIt   >;
   using OutArcRange  = iterator_range< OutArcIt  >;
   using IncEdgeRange = iterator_range< IncEdgeIt >;

   template< typename T, template< typename > class Policy >
   struct Map {
    private:
     using Pol = Policy< T >;

    public:
     using Key   = typename Pol::Key;
     using Value = typename Pol::Value;
     using Reference      = typename Pol::Reference;
     using ConstReference = typename Pol::ConstReference;

    public:
     template< typename ... Args >
     Map( Graph const & g, Args && ... args ) {
     //: maps{}   // default std::vector is ok
       int numComp = g.numComponents();
       maps.reserve( numComp );
       for ( int i = 0; i != numComp; ++i ) {
         maps.push_back( Pol::createMap( g.component( i ), std::forward< Args >( args )... ) );
       }
     }

    public:
     Reference      operator[]( Key const & k )       {   return maps[k.componentIndex()][k];   }
     ConstReference operator[]( Key const & k ) const {   return maps[k.componentIndex()][k];   }

     void set( Key const & k, Value const & v ) {   maps[k.componentIndex()].set( k, v );   }

     void fill( Graph const & g, Value const & v ) {
       int numComp = g.numComponents();
       for ( int i = 0; i != numComp; ++i ) {
         g.component( i ).fill( maps[i], v );
       }
     }

    private:
     std::vector< typename Pol::MapType > maps;
   };
   template< typename Map, typename KeyType >
   struct MapPol {
     using MapType = Map;
     using Key   = KeyType;
     using Value = typename MapType::Value;
     using Reference      = typename MapType::Reference;
     using ConstReference = typename MapType::ConstReference;
   };
   template< typename T >
   struct ArcMapPol
     : public MapPol< GraphComponent::ArcMap < T >, Arc  > {
     template< typename ... Args >
     static GraphComponent::ArcMap < T > createMap( GraphComponent const & c, Args && ... args ) {
         return c.createArcMap < T >( std::forward< Args >( args )... );
     }
   };
   template< typename T >
   struct EdgeMapPol
     : public MapPol< GraphComponent::EdgeMap< T >, Edge > {
     template< typename ... Args >
     static GraphComponent::EdgeMap< T > createMap( GraphComponent const & c, Args && ... args ) {
         return c.createEdgeMap< T >( std::forward< Args >( args )... );
     }
   };
   template< typename T >
   struct NodeMapPol
     : public MapPol< GraphComponent::NodeMap< T >, Node > {
     template< typename ... Args >
     static GraphComponent::NodeMap< T > createMap( GraphComponent const & c, Args && ... args ) {
         return c.createNodeMap< T >( std::forward< Args >( args )... );
     }
   };

   template< typename T >
   using ArcMap  = Map< T, ArcMapPol  >;
   template< typename T >
   using EdgeMap = Map< T, EdgeMapPol >;
   template< typename T >
   using NodeMap = Map< T, NodeMapPol >;

  public:
   ComponentRange components() {
     return make_iterator_range( componentsVector.begin()
                               , componentsVector.end()
                               );
   }
   ConstComponentRange components() const {
     return make_iterator_range( componentsVector.begin()
                               , componentsVector.end()
                               );
   }

   int numComponents() const {   return static_cast< int >(componentsVector.size());   }

   GraphComponent       & component( int i )       {   return componentsVector[i];    }
   GraphComponent const & component( int i ) const {   return componentsVector[i];    }

   template< typename T, typename ... P >
   ArcMap < T > createArcMap ( P && ... p ) const {
     return ArcMap < T >{ *this, std::forward< P >( p )... };
   }
   template< typename T, typename ... P >
   EdgeMap< T > createEdgeMap( P && ... p ) const {
     return EdgeMap< T >{ *this, std::forward< P >( p )... };
   }
   template< typename T, typename ... P >
   NodeMap< T > createNodeMap( P && ... p ) const {
     return NodeMap< T >{ *this, std::forward< P >( p )... };
   }

   template< typename T >
   void fill( T & t, typename T::Value const & v ) const {   t.fill( *this, v );   }

   std::string const & label ( Node const & n ) const {   return n.component().label ( n );   }
   double              weight( Node const & n ) const {   return n.component().weight( n );   }

   ArcIt  arcBegin() const  {   return ArcIt { *this, ArcIt ::ValidTag };   }
   EdgeIt edgeBegin() const {   return EdgeIt{ *this, EdgeIt::ValidTag };   }
   NodeIt nodeBegin() const {   return NodeIt{ *this, NodeIt::ValidTag };   }
   InArcIt   inArcBegin  ( Node const & n ) const {   return InArcIt  { *this, n, InArcIt  ::ValidTag };   }
   OutArcIt  outArcBegin ( Node const & n ) const {   return OutArcIt { *this, n, OutArcIt ::ValidTag };   }
   IncEdgeIt incEdgeBegin( Node const & n ) const {   return IncEdgeIt{ *this, n, IncEdgeIt::ValidTag };   }

   ArcIt  arcEnd () const {   return ArcIt { *this, ArcIt ::InvalidTag };   }
   EdgeIt edgeEnd() const {   return EdgeIt{ *this, EdgeIt::InvalidTag };   }
   NodeIt nodeEnd() const {   return NodeIt{ *this, NodeIt::InvalidTag };   }
   InArcIt   inArcEnd  ( Node const & n ) const {   return InArcIt  { *this, n, InArcIt  ::InvalidTag };   }
   OutArcIt  outArcEnd ( Node const & n ) const {   return OutArcIt { *this, n, OutArcIt ::InvalidTag };   }
   IncEdgeIt incEdgeEnd( Node const & n ) const {   return IncEdgeIt{ *this, n, IncEdgeIt::InvalidTag };   }

   ArcRange  arcs () const {
     return make_iterator_range( arcBegin (), arcEnd () );
   }
   EdgeRange edges() const {
     return make_iterator_range( edgeBegin(), edgeEnd() );
   }
   NodeRange nodes() const {
     return make_iterator_range( nodeBegin(), nodeEnd() );
   }

   InArcRange   inArcs  ( Node const & n ) const {
     return make_iterator_range( inArcBegin  ( n ), inArcEnd  ( n ) );
   }
   OutArcRange  outArcs ( Node const & n ) const {
     return make_iterator_range( outArcBegin ( n ), outArcEnd ( n ) );
   }
   IncEdgeRange incEdges( Node const & n ) const {
     return make_iterator_range( incEdgeBegin( n ), incEdgeEnd( n ) );
   }

   int numArcs () const {   return numOfArcs;    }
   int numEdges() const {   return numOfEdges;   }
   int numNodes() const {   return numOfNodes;   }

   Node u( Edge const & e ) const {
     GraphComponent const & c = e.component();
     return Node{ c, c.u( e ) };
   }

   Node v( Edge const & e ) const {
     GraphComponent const & c = e.component();
     return Node{ c, c.v( e ) };
   }

   Node oppositeNode( Node const & n, Edge const & e ) const {
     GraphComponent const & c = n.component();
     return Node{ c, c.oppositeNode( n, e ) };
   }

   optional< Arc > lookUpArc( Node const & u, Node const & v ) const {
     if ( u.componentIndex() == v.componentIndex() ) {
       GraphComponent const & c = u.component();
       BaseArc a = c.lookUpArc( u, v );

       if ( a == lemon::INVALID ) {
         return none;
       } else {
         return Arc{ c, a };
       }
     }
     else {
       return none;
     }
   }

  public:
   friend std::ostream & operator<<( std::ostream & out, Graph const & g ) {
     int number = 0;
     bool first = true;
     for_each( g.componentsVector, [&]( GraphComponent const & gc ) {
       if ( !first ) {
         out << "\n";
       } else {
         first = false;
       }
       out << "##Component " << ++number << ":\n" << gc;
     });
     return out;
   }

  private:
   friend class GraphBuilder;

  private:
   std::vector< GraphComponent > componentsVector;
   int numOfArcs, numOfEdges, numOfNodes;

  private:
   Graph( lemon::ListGraph const                         & originTopology
        , lemon::ListGraph::NodeMap< std::string > const & originLabels
        , lemon::ListGraph::NodeMap< double > const      & originWeights
        )
   //: componentsVector{}   // vector will be filled in ctor body
     : numOfArcs { 0 }      // initialize to 0 and increment in ctor body
     , numOfEdges{ 0 }      // initialize to 0 and increment in ctor body
     , numOfNodes{ 0 } {    // initialize to 0 and increment in ctor body
     lemon::ListGraph::NodeMap< int > comps( originTopology );
     int numComps = lemon::connectedComponents( originTopology, comps );

     lemon::ListGraph::NodeMap< bool > filter( originTopology );
     lemon::FilterNodes< lemon::ListGraph const > subGraph( originTopology, filter );
     for ( int i = 0; i < numComps; ++i ) {
       for ( lemon::ListGraph::NodeIt n( originTopology ); n != lemon::INVALID; ++n ) {
           if ( comps[n] == i ) {
             filter[n] = true;
           } else {
             filter[n] = false;
           }
       }
       // cannot emplace_back due to private ctor of GraphComponent
       componentsVector.push_back( GraphComponent{ i, subGraph, originLabels, originWeights } );

       numOfArcs  += componentsVector.back().numArcs ();
       numOfEdges += componentsVector.back().numEdges();
       numOfNodes += componentsVector.back().numNodes();
     }
   }
 };

 using LabelToGraphNodeMap = std::map< std::string, Graph::Node >;

} // namespace xHeinz

#endif // xHeinz_Graph_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
