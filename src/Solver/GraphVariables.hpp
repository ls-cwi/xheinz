#ifndef xHeinz_Solver_GraphVariables_HPP
#define xHeinz_Solver_GraphVariables_HPP

#include <ilcplex/ilocplex.h>

#include "Graph.hpp"

namespace xHeinz {
namespace solver {

 struct GraphVariables {
  public:
   Graph::NodeMap< int > nodeToIndex;
   std::vector< Graph::Node > indexToNode;

   IloBoolVarArray xVars;
   IloBoolVarArray yVars;
   IloBoolVarArray mmVars;

  public:
   GraphVariables( Graph const & g )
     : nodeToIndex{ g.createNodeMap< int >() } {
   }
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_GraphVariables_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
