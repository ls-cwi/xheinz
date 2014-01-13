#ifndef xHeinz_Solver_Solution_HPP
#define xHeinz_Solver_Solution_HPP

#include <iosfwd>
#include <vector>

#include "Graph.hpp"

namespace xHeinz {
namespace solver {

 struct OutputSolution {
   using Score         = double;
   using SolutionSet   = std::vector< std::pair< Graph::Node, bool > >;
   using GraphSolution = std::pair< Score, SolutionSet >;

   Score totalScore;
   std::vector< double > alpha;
   std::vector< GraphSolution > graphsSolutions;

   friend std::ostream & operator<<( std::ostream & out, OutputSolution const & sol );
 };

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

#endif // xHeinz_SolverSolution_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
