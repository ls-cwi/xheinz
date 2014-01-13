#ifndef xHeinz_GraphBuilder_HPP
#define xHeinz_GraphBuilder_HPP

#include <string>

#include "Graph.hpp"

namespace xHeinz {

 class GraphBuilder
   : public impl::GraphBasicTypes< lemon::ListGraph > {
  public:
   GraphBuilder()
   //: topology{}   // default is ok
     : labels { topology }
     , weights{ topology } {
   }

  public:
   std::string const & label ( BaseNode const & n ) const {   return labels [n];   }
   double              weight( BaseNode const & n ) const {   return weights[n];   }

   ArcIt  arcBegin () const {   return BaseArcIt { topology };   }
   EdgeIt edgeBegin() const {   return BaseEdgeIt{ topology };   }
   NodeIt nodeBegin() const {   return BaseNodeIt{ topology };   }
   InArcIt   inArcBegin  ( BaseNode const & n ) const {   return BaseInArcIt  { topology, n };   }
   OutArcIt  outArcBegin ( BaseNode const & n ) const {   return BaseOutArcIt { topology, n };   }
   IncEdgeIt incEdgeBegin( BaseNode const & n ) const {   return BaseIncEdgeIt{ topology, n };   }

   ArcIt  arcEnd () const {   return BaseArcIt {  lemon::INVALID };   }
   EdgeIt edgeEnd() const {   return BaseEdgeIt{  lemon::INVALID };   }
   NodeIt nodeEnd() const {   return BaseNodeIt{  lemon::INVALID };   }
   InArcIt   inArcEnd  ( BaseNode const & n ) const {   return BaseInArcIt  { lemon::INVALID };   }
   OutArcIt  outArcEnd ( BaseNode const & n ) const {   return BaseOutArcIt { lemon::INVALID };   }
   IncEdgeIt incEdgeEnd( BaseNode const & n ) const {   return BaseIncEdgeIt{ lemon::INVALID };   }

   ArcRange  arcs () const {   return make_iterator_range( arcBegin (), arcEnd () );   }
   EdgeRange edges() const {   return make_iterator_range( edgeBegin(), edgeEnd() );   }
   NodeRange nodes() const {   return make_iterator_range( nodeBegin(), nodeEnd() );   }

   InArcRange   inArcs  ( BaseNode const & n ) const {
     return make_iterator_range( inArcBegin  ( n ), inArcEnd  ( n ) );
   }
   OutArcRange  outArcs ( BaseNode const & n ) const {
     return make_iterator_range( outArcBegin ( n ), outArcEnd ( n ) );
   }
   IncEdgeRange incEdges( BaseNode const & n ) const {
     return make_iterator_range( incEdgeBegin( n ), incEdgeEnd( n ) );
   }

   BaseNode u( BaseEdge const & e ) const {   return topology.u( e );   }
   BaseNode v( BaseEdge const & e ) const {   return topology.v( e );   }

   BaseNode oppositeNode( BaseNode const & n, BaseEdge const & e ) const {
     return topology.oppositeNode( n, e );
   }

   int id( BaseArc const  & a ) const {   return topology.id( a );   }
   int id( BaseEdge const & e ) const {   return topology.id( e );   }
   int id( BaseNode const & n ) const {   return topology.id( n );   }

   bool valid( BaseArc const  & a ) const {   return topology.valid( a );   }
   bool valid( BaseEdge const & e ) const {   return topology.valid( e );   }
   bool valid( BaseNode const & n ) const {   return topology.valid( n );   }

   BaseNode addNode() {   return topology.addNode();   }

   void setLabel( BaseNode const & n, std::string s ) {
     labels.set( n, std::move( s ) );
   }

   void setWeight( BaseNode const & n, double w ) {
     weights.set( n, w );
   }

   BaseEdge addEdge( BaseNode const & u, Node const & v ) {
     return topology.addEdge( u, v );
   }

   Graph build() const {
     return Graph{ topology, labels, weights };
   }

  private:
   BaseGraph                  topology;
   BaseNodeMap< std::string > labels;
   BaseNodeMap< double >      weights;

  private:
   BaseGraph       & getImpl()       {   return topology;   }
   BaseGraph const & getImpl() const {   return topology;   }
   BaseNodeMap< std::string >       & getLabels()       {   return labels;   }
   BaseNodeMap< std::string > const & getLabels() const {   return labels;   }
   BaseNodeMap< double >       & getWeights()       {   return weights;   }
   BaseNodeMap< double > const & getWeights() const {   return weights;   }
 };

} // namespace xHeinz

#endif // xHeinz_GraphBuilder_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
