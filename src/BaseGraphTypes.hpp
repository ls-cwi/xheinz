#ifndef xHeinz_BaseGraphTypes_H
#define xHeinz_BaseGraphTypes_H

#include <lemon/core.h>
#include <lemon/maps.h>

#include "CppUtils.hpp"

namespace xHeinz {

 namespace impl {

  template< typename BaseType >
  class DigraphBasicTypes {
   public:
    using BaseDigraph = BaseType;

   protected:
    using BaseArc  = typename BaseDigraph::Arc;
    using BaseNode = typename BaseDigraph::Node;

    using BaseArcIt    = typename BaseDigraph::ArcIt;
    using BaseInArcIt  = typename BaseDigraph::InArcIt;
    using BaseOutArcIt = typename BaseDigraph::OutArcIt;
    using BaseNodeIt   = typename BaseDigraph::NodeIt;

    template< typename T > using BaseArcMap  = typename BaseDigraph::template ArcMap < T >;
    template< typename T > using BaseNodeMap = typename BaseDigraph::template NodeMap< T >;

    using BaseArcLookUp = lemon::ArcLookUp< BaseType >;

   public:
    using Arc  = BaseArc;
    using Node = BaseNode;

    template< typename It >
    struct Iter
      : public iterator_facade< Iter< It >
                                    , It const
                                    , forward_traversal_tag
                                    > {
     public:
      Iter() {   }

      Iter( It i ): it{ std::move( i ) } {   }

      Iter( Iter const & other ): it{ other.it } {   }

     private:
      friend class boost::iterator_core_access;

     private:
      It it = lemon::INVALID;

     private:
      void increment() {   ++it;   }

      bool equal( Iter const & rhs ) const {   return it == rhs.it;   }

      It const & dereference() const {   return it;   }
    };
    using ArcIt     = Iter< BaseArcIt     >;
    using NodeIt    = Iter< BaseNodeIt    >;
    using InArcIt   = Iter< BaseInArcIt   >;
    using OutArcIt  = Iter< BaseOutArcIt  >;

    using ArcRange    = iterator_range< ArcIt     >;
    using NodeRange   = iterator_range< NodeIt    >;
    using InArcRange  = iterator_range< InArcIt   >;
    using OutArcRange = iterator_range< OutArcIt  >;

    template< typename M >
    struct Map {
     public:
      using Key   = typename M::Key;
      using Value = typename M::Value;
      using Reference      = typename M::Reference;
      using ConstReference = typename M::ConstReference;

     public:
      template< typename ...Args >
      Map( BaseDigraph const & topology, Args ...args )
        : impl{ make_shared< M >( topology, std::forward< Args >( args )... ) } {
      }

      Map copy( BaseDigraph const & top ) const {
        Map ret{ top };
        lemon::mapCopy( top, *impl, *ret.impl );
        return ret;
      }

      void fill( BaseDigraph const & top, Value const & v ) const {
        lemon::mapFill( top, *impl, v );
      }

     public:
      Reference      operator[]( Key const & k )       {   return (*impl)[k];   }
      ConstReference operator[]( Key const & k ) const {   return (*impl)[k];   }

      void set( Key const & k, Value const & v ) {   impl->set( k, v );   }

     private:
      shared_ptr< M > impl;
    };
    template< typename T >
    using ArcMap  = Map< BaseArcMap< T > >;
    template< typename T >
    using NodeMap = Map< BaseNodeMap< T > >;
  };

  template< typename BaseType >
  class GraphBasicTypes
    : public DigraphBasicTypes< BaseType > {
   private:
    using MyBase = DigraphBasicTypes< BaseType >;

   public:
    using BaseGraph = typename MyBase::BaseDigraph;

   protected:
    using BaseEdge      = typename BaseType::Edge;
    using BaseEdgeIt    = typename BaseType::EdgeIt;
    using BaseIncEdgeIt = typename BaseType::IncEdgeIt;

    template< typename T > using BaseEdgeMap = typename BaseType::template EdgeMap< T >;

   public:
    using Edge = BaseEdge;

    using EdgeIt    = typename MyBase::template Iter< BaseEdgeIt    >;
    using IncEdgeIt = typename MyBase::template Iter< BaseIncEdgeIt >;

    using EdgeRange    = iterator_range< EdgeIt    >;
    using IncEdgeRange = iterator_range< IncEdgeIt >;

    template< typename T >
    using EdgeMap = typename MyBase::template Map< BaseEdgeMap< T > >;
  };

 } // namespace impl

} // namespace xHeinz

#endif // xHeinz_BaseGraphTypes_H

/* vim: set ts=8 sw=2 sts=2 et : */
