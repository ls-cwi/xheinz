#ifndef xHeinz_ChainGraph_HPP
#define xHeinz_ChainGraph_HPP

#include "CppUtils.hpp"
#include "ThreeWayGraph.hpp"

namespace xHeinz {

 struct ChainGraph {
  public:
   optional< ThreeWayGraph & > addGraph( Graph g ) {
     graphs.push_back( std::move( g ) );
     size_t sz = graphs.size();
     if ( sz == 1 ) {
       return none;
     }
     twgraphs.emplace_back( graphs[sz-2], graphs[sz-1] );
     return twgraphs.back();
   }

  public:
   int numGraphs() const {   return static_cast< int >( graphs.size() );   }

   Graph       & graph( int i )       {   return graphs[i];   }
   Graph const & graph( int i ) const {   return graphs[i];   }

   ThreeWayGraph       & threeWay( int i )       {   return twgraphs[i];   }
   ThreeWayGraph const & threeWay( int i ) const {   return twgraphs[i];   }

  public:
   friend std::ostream & operator<<( std::ostream & out, ChainGraph const & cg ) {
     int number = 0;
     bool first = true;
     for_each( cg.graphs, [&]( Graph const & g ) {
       if ( !first ) {
         out << "\n\n";
       } else {
         first = false;
       }
       out << "#Graph " << number++ << ":\n" << g;
     });
     return out;
   }

  private:
   std::vector< Graph > graphs;
   std::vector< ThreeWayGraph > twgraphs;
 };

} // namespace xHeinz

#endif // xHeinz_ChainGraph_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
