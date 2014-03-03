#include "Solver/BkMaxFlow.hpp"

namespace xHeinz {
namespace solver {

 BkMaxFlow::BkMaxFlow( DgComponent const & dg, DgComponent::Node const & src )
   : digraph( dg )
   , capacities{ digraph.createCapacitiesMap() }
   , source{ src }
   , target{ lemon::INVALID }
   , dgArcToBkArc{ digraph.createArcMap< BkArc * >( nullptr ) }
   , dgNodeToIndex{ digraph.createNodeMap< int >( -1 ) }
   , bkGraph{ make_unique< BkGraph >( digraph.numNodes(), digraph.numArcs() ) }
   , flowValue{ 0.0 } {
   // pre-condition: there are no self loops in dg
   bkGraph->add_node( digraph.numNodes() );

   // copy nodes
   int i = 0;
   for_each( digraph.nodes(), [&]( DgComponent::Node const & n ) {
     dgNodeToIndex[n] = i++;
   });

   // copy arcs
   for_each( digraph.arcs(), [&]( DgComponent::Arc const & a ) {
     auto v = digraph.source( a );
     auto w = digraph.target( a );

     dgArcToBkArc[a] = bkGraph->add_edge( dgNodeToIndex[v]
                                        , dgNodeToIndex[w]
                                        , capacities[a]
                                        , 0
                                        );
   });
 }

 double BkMaxFlow::run( bool reuse ) {
   return (flowValue = bkGraph->maxflow( reuse ));
 }

 double BkMaxFlow::maxFlow() const {
   return flowValue;
 }

 bool BkMaxFlow::cut( DgComponent::Arc const & a ) const {
   return cut( digraph.source( a ) ) && !cut( digraph.target( a ) );
 }

 bool BkMaxFlow::cut( DgComponent::Node const & v ) const {
   return bkGraph->what_segment( dgNodeToIndex[v] ) == BkGraph::SOURCE;
 }

 double BkMaxFlow::flow( DgComponent::Arc const & a ) const {
   return capacities[a] - dgArcToBkArc[a]->r_cap;
 }

 void BkMaxFlow::increaseCapacity( DgComponent::Arc const & a, double c ) {
   BkArc * pBkArc = dgArcToBkArc[a];
   pBkArc->r_cap += c;
   capacities[a] += c;

   bkGraph->mark_node( dgNodeToIndex[digraph.source( a )] );
 }

 double BkMaxFlow::residue( DgComponent::Arc const & a ) const {
   return dgArcToBkArc[a]->r_cap;
 }

 double BkMaxFlow::reverseResidue( DgComponent::Arc const & a ) const {
   return dgArcToBkArc[a]->sister->r_cap;
 }

 void BkMaxFlow::printFlow( std::ostream& out ) const {
   for_each( digraph.arcs(), [&]( DgComponent::Arc const & a ) {
     auto v = digraph.source( a );
     auto w = digraph.target( a );
     out << digraph.id( v ) << " -> " << digraph.id( w )
         << (cut( a ) ? " * " : "   ")
         << flow( a ) << "/" << capacities[a]
         << std::endl;
   });
 }

 void BkMaxFlow::printCut( std::ostream& out ) const {
   for_each( digraph.nodes(), [&]( DgComponent::Node const & n ) {
     out << digraph.id( n ) << (cut( n ) ? ": source" : ": target") << std::endl;
   });
 }

 void BkMaxFlow::reset( DgComponent::Node const & tgt ) {
   target = tgt;

   for_each( digraph.arcs(), [&]( DgComponent::Arc const & a ) {
     double cap_a = capacities[a];
     BkArc * pBkArc = dgArcToBkArc[a];

     pBkArc->r_cap = cap_a;
     pBkArc->sister->r_cap = 0;
   });

   for_each( digraph.nodes(), [&]( DgComponent::Node const & n ) {
     bkGraph->set_trcap( dgNodeToIndex[n], 0 );
   });

   const double max = std::numeric_limits<double>::max();
   assert( source != lemon::INVALID );
   bkGraph->add_tweights( dgNodeToIndex[source], max, 0 );
   assert( target != lemon::INVALID );
   bkGraph->add_tweights( dgNodeToIndex[target], 0, max );
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
