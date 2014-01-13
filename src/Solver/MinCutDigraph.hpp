#ifndef xHeinz_Solver_MinCutDigraph_HPP
#define xHeinz_Solver_MinCutDigraph_HPP

#include <vector>

#include <lemon/smart_graph.h>

#include "Solver/MinCutDigraphComponent.hpp"

namespace xHeinz {
namespace solver {

 class MinCutDigraph
   : public impl::DigraphBasicTypes< lemon::SmartDigraph > {
  private:
   using MyBase = impl::DigraphBasicTypes< lemon::SmartDigraph >;

  public:
   using Component     = MinCutDigraphComponent;
   using ComponentsVec = std::vector< Component >;

   using CompRange      = iterator_range< ComponentsVec::iterator       >;
   using ConstCompRange = iterator_range< ComponentsVec::const_iterator >;

  public:
   MinCutDigraph( Graph const & gg, GraphVariables & vv )
     : g( gg ) {
   //, digraphs{}   // vector is filled in ctor body
     for_each( g.components(), [&]( GraphComponent const & c ) {
       digraphs.emplace_back( c, vv );
     });
   }

   int numComponents() const {   return static_cast< int >(digraphs.size());   }

   Component       & component( int i )       {   return digraphs[i];   }
   Component const & component( int i ) const {   return digraphs[i];   }

   CompRange      components()       {
     return make_iterator_range( digraphs.begin (), digraphs.end () );
   }
   ConstCompRange components() const {
     return make_iterator_range( digraphs.cbegin(), digraphs.cend() );
   }

  private:
   Graph const    & g;
   ComponentsVec digraphs;
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_MinCutDigraph_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
