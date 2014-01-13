#include "Solver/MinCutDigraphComponent.hpp"

#include "Math.hpp"

using namespace std;

namespace xHeinz {
namespace solver {

 std::ostream & operator<<( std::ostream & out, MinCutDigraphComponent const & comp ) {
   for_each( comp.arcs(), [&]( MinCutDigraphComponent::Arc const & a ) {
     auto const & source = comp.source( a );
     auto const & target = comp.target( a );

     assert( target != comp.rootNode() );
     if ( source == comp.rootNode() ) {
       out << 'r';
     } else {
       auto const & orgSource = comp.toGraphNode( source );
       auto const & extPair   = comp.toExtNode( orgSource );

       out << comp.graph.label( orgSource );

       if ( source == extPair.first ) {
         out << "_1";
       } else {
         out << "_2";
       }
     }
     out << " -> ";
     auto const & orgTarget = comp.toGraphNode( target );
     auto const & extPair   = comp.toExtNode( orgTarget );

     out << comp.graph.label( orgTarget );

     if ( target == extPair.first ) {
       out << "_1";
     } else {
       out << "_2";
     }

     out << " (" <<  comp.initCapacities[a] << ')';
   });
   return out;
 }


 MinCutDigraphComponent::MinCutDigraphComponent( GraphComponent const & g
                                               , GraphVariables const & va
                                               )
   : vars( va )
   , graph( g )
   , h{ make_shared< BaseDigraph >() }
 //, root{}         // default is ok
 //, numOfArcs{}    // initialized at the end of ctor
 //, numOfNodes{}   // initialized at the end of ctor
   , h2g{ *h }
   , g2h{ g.createNodeMap< ExtNode >() }
   , g2hRootArc{ g.createNodeMap< BaseDigraph::Arc >() }
   , initCapacities{ *h } {
   // we initialize h:
   // - for every node i, there will be two nodes i1 and i2
   //   connected by an arc from i1 to i2
   // - for every edge (i,j) there are two arcs
   //   in h: (i2,j1) and (j2,i1) with capacties 1
   root = h->addNode();
   for_each( g.nodes(), [&]( GraphComponent::Node const & n ) {
     auto i1 = h->addNode();
     auto i2 = h->addNode();
     g2h[n] = make_pair( i1, i2 );
     h2g[i1] = n;
     h2g[i2] = n;

     auto i1i2 = h->addArc( i1, i2 );
     initCapacities[i1i2] = 0;

     auto ri1 = h->addArc( root, i1 );
     g2hRootArc[n] = ri1;
     initCapacities[ri1] = 1;
   });

   for_each( g.edges(), [&]( GraphComponent::Edge const & e ) {
     auto n0 = g.u( e ), n1 = g.v( e );

     auto i1 = g2h[n0].first;
     auto i2 = g2h[n0].second;
     auto j1 = g2h[n1].first;
     auto j2 = g2h[n1].second;

     auto i2j1 = h->addArc( i2, j1 );
     auto j2i1 = h->addArc( j2, i1 );
     initCapacities[i2j1] = initCapacities[j2i1] = 1;
   });

   numOfArcs  = lemon::countArcs ( *h );
   numOfNodes = lemon::countNodes( *h );
 }

 MinCutDigraphComponent::MinCutDigraphComponent( MinCutDigraphComponent const & other )
   : vars( other.vars )
   , graph( other.graph )
   , h{ other.h }
   , root{ other.root }
   , numOfArcs{ other.numOfArcs }
   , numOfNodes{ other.numOfNodes }
   , h2g{ other.h2g }
   , g2h{ other.g2h }
   , g2hRootArc{ other.g2hRootArc }
   , initCapacities{ other.initCapacities } {
 }

 MinCutDigraphComponent::CapacitiesMap MinCutDigraphComponent::createCapacitiesMap() const {
   return copy( initCapacities );
 }

 namespace {

  static const double forceMinCardinalityValue = 10.0 * epsilon;

  inline double GetMinimalVal( double val ) {
    return isZero( val ) ? forceMinCardinalityValue : val;
  }

 } // namespace <anonymous>

 void MinCutDigraphComponent::updateCapacities( IloNumArray x_values
                                              , IloNumArray y_values
                                              , CapacitiesMap & capacities
                                              ) const {
   for_each( graph.nodes(), [&]( GraphComponent::Node const & n ) {
     Graph::Node gn{ graph, n };
     int nodeIndex = vars.nodeToIndex[gn];

     // cap((i,j)) = x_i
     BaseDigraph::OutArcIt arc{ *h, g2h[gn].first };   // only one arc by ction
     capacities[arc] = GetMinimalVal( x_values[nodeIndex] );

     // cap((r,i)) = y_i
     capacities[g2hRootArc[gn]] = GetMinimalVal( y_values[nodeIndex] );
   });
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
