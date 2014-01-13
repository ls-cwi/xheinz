#ifndef xHeinz_Solver_BkMaxFlow_HPP
#define xHeinz_Solver_BkMaxFlow_HPP

// BIG FAT WARNING: reverse arcs not supported
// (uncomment ArcLookUp to add support)

#include <limits>
#include <ostream>

#include <lemon/core.h>

#include "Solver/MinCutDigraphComponent.hpp"

#include "maxflow-v3.01/graph.h"

namespace xHeinz {
namespace solver {

 class BkMaxFlow {
  private:
   using DgComponent = MinCutDigraphComponent;

  public:
   DgComponent const & digraph;

   DgComponent::CapacitiesMap capacities;

  public:
   BkMaxFlow( DgComponent const & dg, DgComponent::Node const & source );

  public:
   void reset( DgComponent::Node const & target );

   double run( bool reuse );

   double flow( DgComponent::Arc const & a ) const;
   double residue( DgComponent::Arc const & a ) const;

   bool cut( DgComponent::Arc const & a ) const;
   bool cut( DgComponent::Node const & v ) const;

   double maxFlow() const;

   void increaseCapacity( DgComponent::Arc const & a, double c );

   void printFlow( std::ostream & out ) const;
   void printCut( std::ostream & out ) const;

 private:
   using BkGraph = ::Graph< double, double, double >;

   using BkArc = BkGraph::arc;

   DgComponent::Node source;
   DgComponent::Node target;

   DgComponent::ArcMap < BkArc * > dgArcToBkArc;
   DgComponent::NodeMap< int     > dgNodeToIndex;

   // bkGraph does not support copy even if the copy ctor is advertised
   unique_ptr< BkGraph > bkGraph;

   double flowValue;
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_BkMaxFlow_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
