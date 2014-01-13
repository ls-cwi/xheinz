#ifndef xHeinz_Solver_LinkGraphVariables_HPP
#define xHeinz_Solver_LinkGraphVariables_HPP

#include <ilcplex/ilocplex.h>
#include <algorithm>
#include <vector>

#include "ThreeWayGraph.hpp"

namespace xHeinz {
namespace solver {

 struct LinkGraphVariables {
  public:
   using IndexPair            = std::pair< int, int >;
   using BpEdgeToIndexPairMap = ThreeWayGraph::BpEdgeMap< IndexPair >;
   using BpEdgeVector         = std::vector< ThreeWayGraph::BpEdge >;
   using BpEdgeMatrix         = std::vector< BpEdgeVector >;

  public:
   BpEdgeToIndexPairMap        edgeToIndexPair;
   ThreeWayGraph::BpIntEdgeMap edgeToIndex;
   ThreeWayGraph::BpIntNodeMap nodeToIndex;
   BpEdgeMatrix                edgesPerComponent;

   // TODO: make BpGraph abstraction and move numComponents into there
   int numComponents;

   IloBoolVarArray             mVars;
   IloArray< IloBoolVarArray > zVars;

  private:
   ThreeWayGraph::BpGraph const & link; // needed for copy constructor

  public:
   LinkGraphVariables( ThreeWayGraph::BpGraph const & link )
     : edgeToIndexPair{ link, std::make_pair( -1, -1 ) }
     , edgeToIndex{ link, -1 }
     , nodeToIndex{ link, -1 }
   //, edgesPerComponent{}    // default initialization ok
     , numComponents{ -1 }
   //, mVars{}                // default initialization ok
   //, zVars{}                // default initialization ok
     , link( link ) {
   }

   LinkGraphVariables( LinkGraphVariables const & other )
     : edgeToIndexPair{ other.link }
     , edgeToIndex{ other.link }
     , nodeToIndex{ other.link }
     , edgesPerComponent{ other.edgesPerComponent }
     , numComponents{ other.numComponents }
     , mVars{ other.mVars }
     , zVars{ other.zVars }
     , link( other.link ) {
     assert(false);
     lemon::mapCopy( link, other.edgeToIndexPair, edgeToIndexPair );
     lemon::mapCopy( link, other.nodeToIndex, nodeToIndex );
   }
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_LinkGraphVariables_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
