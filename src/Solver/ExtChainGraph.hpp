#ifndef xHeinz_Solver_ExtChainGraph_HPP
#define xHeinz_Solver_ExtChainGraph_HPP

#include <cassert>

#include <memory>

#include <ilconcert/ilothread.h>

#include "ChainGraph.hpp"

#include "Solver/GraphVariables.hpp"
#include "Solver/LinkGraphVariables.hpp"
#include "Solver/MinCutDigraph.hpp"

namespace xHeinz {
namespace solver {

 class ExtChainGraph {
  public:
   int numGraphs() const {
     assert( graphs.numGraphs() <= std::numeric_limits< int >::max() );
     return static_cast< int >(graphs.numGraphs());
   }

 //Graph       & graph( int i )       {
 //  assert( i >= 0 );
 //  return graphs.graph( i );
 //}
   Graph const & graph( int i ) const {
     assert( i >= 0 );
     return graphs.graph( i );
   }

 //ThreeWayGraph       & threeWay( int i )       {
 //  assert( i >= 0 );
 //  return graphs.threeWay( i );
 //}
   ThreeWayGraph const & threeWay( int i ) const {
     assert( i >= 0 );
     return graphs.threeWay( i );
   }

   GraphVariables       & variables( int i )       {
     assert( i >= 0 );
     assert( static_cast< unsigned int >( i ) < vars.size() );
     return vars[i];
   }
   GraphVariables const & variables( int i ) const {
     assert( i >= 0 );
     assert( static_cast< unsigned int >( i ) < vars.size() );
     return vars[i];
   }

   LinkGraphVariables       & linkVariables( int i )       {
     assert( i >= 0 );
     assert( static_cast< unsigned int >( i ) < linkVars.size() );
     return linkVars[i];
   }
   LinkGraphVariables const & linkVariables( int i ) const {
     assert( i >= 0 );
     assert( static_cast< unsigned int >( i ) < linkVars.size() );
     return linkVars[i];
   }

   MinCutDigraph       & digraph( int i )       {
     assert( i >= 0 );
     assert( static_cast< unsigned int >( i ) < digraphs.size() );
     return digraphs[i];
   }
   MinCutDigraph const & digraph( int i ) const {
     assert( i >= 0 );
     assert( static_cast< unsigned int >( i ) < digraphs.size() );
     return digraphs[i];
   }

  public:
   ExtChainGraph( ChainGraph const & cg )
     : graphs( cg ) {
   //, vars{}       // vector is filled in ctor body
   //, digraphs{}   // vector is filled in ctor body
     // We need to first construct the variables since we later take references to thoses
     // and the vector can be resized during its filling
     for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
       Graph const & g = graphs.graph( i );
       vars.emplace_back( g );
     }
     // Here take those references
     for ( int i = 0, e = graphs.numGraphs(); i != e; ++i ) {
       Graph const & g = graphs.graph( i );
       digraphs.emplace_back( g, vars[i] );
     }

     for ( int i = 0, e = graphs.numGraphs() - 1; i != e; ++i ) {
       ThreeWayGraph::BpGraph const & link = graphs.threeWay( i ).link();
       linkVars.emplace_back( link );
     }
   }

  private:
   ChainGraph const & graphs;

   std::vector< GraphVariables > vars;
   std::vector< LinkGraphVariables > linkVars;
   std::vector< MinCutDigraph  > digraphs;
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_ExtChainGraph_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
