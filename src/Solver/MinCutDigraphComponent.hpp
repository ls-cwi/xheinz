#ifndef xHeinz_Solver_MinCutDigraphComponent_HPP
#define xHeinz_Solver_MinCutDigraphComponent_HPP

#include <vector>

#include <lemon/smart_graph.h>

#include "GraphComponent.hpp"
#include "Solver/GraphVariables.hpp"

namespace xHeinz {
namespace solver {

 class MinCutDigraphComponent
   : public impl::DigraphBasicTypes< lemon::SmartDigraph > {
  public:
   using CapacitiesMap = ArcMap< double >;

   using ExtNode = std::pair< Node, Node >;

  public:
   MinCutDigraphComponent( GraphComponent const & g
                         , GraphVariables const & vars
                         );
   MinCutDigraphComponent( MinCutDigraphComponent const & other );

  public:
   Node rootNode() const {   return root;   }

   ArcIt  arcBegin () const {   return BaseArcIt { *h };   }
   NodeIt nodeBegin() const {   return BaseNodeIt{ *h };   }
   InArcIt  inArcBegin ( Node const & n ) const {   return BaseInArcIt { *h, n };   }
   OutArcIt outArcBegin( Node const & n ) const {   return BaseOutArcIt{ *h, n };   }

   ArcIt  arcEnd () const {   return ArcIt {};   }
   NodeIt nodeEnd() const {   return NodeIt{};   }
   InArcIt  inArcEnd ( Node const & n ) const {   return InArcIt {};   }
   OutArcIt outArcEnd( Node const & n ) const {   return OutArcIt{};   }

   ArcRange  arcs () const {   return ArcRange { arcBegin (), arcEnd () };   }
   NodeRange nodes() const {   return NodeRange{ nodeBegin(), nodeEnd() };   }

   InArcRange  inArcs ( Node const & n ) const {
     return InArcRange { inArcBegin ( n ), inArcEnd ( n ) };
   }
   OutArcRange outArcs( Node const & n ) const {
     return OutArcRange{ outArcBegin( n ), outArcEnd( n ) };
   }

   Node source( Arc const & a ) const {   return h->source( a );   }
   Node target( Arc const & a ) const {   return h->target( a );   }

   GraphComponent::Node toGraphNode( Node const & n ) const {   return h2g[n];   }
   ExtNode toExtNode( GraphComponent::Node const & n ) const {   return g2h[n];   }

   int id( Arc const  & a ) const {   return h->id( a );   }
   int id( Node const & n ) const {   return h->id( n );   }

   int numArcs () const {   return numOfArcs;    }
   int numNodes() const {   return numOfNodes;   }

   CapacitiesMap createCapacitiesMap() const;

   void updateCapacities( IloNumArray x_values
                        , IloNumArray y_values
                        , CapacitiesMap & capacities
                        ) const;

   template< typename T, typename ... P >
   ArcMap < T > createArcMap ( P && ... p ) const {
     return ArcMap < T >{ *h, std::forward< P >( p )... };
   }

   template< typename T, typename ... P >
   NodeMap< T > createNodeMap( P && ... p ) const {
     return NodeMap< T >{ *h, std::forward< P >( p )... };
   }

   template< typename T >
   T copy( T const & t ) const {   return t.copy( *h );   }

   template< typename T >
   void fill( T & t, typename T::Value const & v ) const {   t.fill( *h, v );   }

  public:
   friend std::ostream & operator<<( std::ostream & out, MinCutDigraphComponent const & comp );

  private:
   GraphVariables const & vars;
   GraphComponent const & graph;

   std::shared_ptr< BaseDigraph > h;
   Node root;
   int numOfArcs;
   int numOfNodes;

   NodeMap< GraphComponent::Node > h2g;

   GraphComponent::NodeMap< ExtNode > g2h;
   GraphComponent::NodeMap< Arc     > g2hRootArc;

   CapacitiesMap initCapacities;

  private:
//   BaseDigraph       & getImpl()       {   return *h;   }
//   BaseDigraph const & getImpl() const {   return *h;   }
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_MinCutDigraphComponent_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
