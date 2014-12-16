#ifndef xHeinz_Solver_Solution_HPP
#define xHeinz_Solver_Solution_HPP

#include <iosfwd>
#include <vector>

#include "Graph.hpp"

namespace xHeinz {
namespace solver {

 struct OutputSolution {
   using WeightScore   = double;
   using AlphaScore    = double;
   using NodeBoolPair  = std::pair< Graph::Node, bool >;
   using SolutionSet   = std::vector< NodeBoolPair >;
   using GraphSolution = std::tuple< SolutionSet, WeightScore, AlphaScore >;

   WeightScore upperBound;
   WeightScore weight;
   AlphaScore  alpha;
   std::vector< GraphSolution > graphsSolutions;

   friend std::ostream & operator<<( std::ostream & out, OutputSolution const & sol );
 };

 inline Graph::Node       & getNode( OutputSolution::NodeBoolPair       & p ) {   return std::get< 0 >( p );   };
 inline Graph::Node const & getNode( OutputSolution::NodeBoolPair const & p ) {   return std::get< 0 >( p );   };
 inline bool              & getBool( OutputSolution::NodeBoolPair       & p ) {   return std::get< 1 >( p );   };
 inline bool const        & getBool( OutputSolution::NodeBoolPair const & p ) {   return std::get< 1 >( p );   };

 struct InputSolution {
   using SolutionSet   = std::vector< Graph::Node >;
   using GraphSolution = SolutionSet;

   std::vector< GraphSolution > graphsSolutions;

   InputSolution( char const * filename
                , LabelToGraphNodeMap const & labels0
                , LabelToGraphNodeMap const & labels1
                );

   friend std::ostream & operator<<( std::ostream & out, InputSolution const & sol );
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_Solution_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
