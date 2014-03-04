#ifndef xHeinz_Solver_Callback_HPP
#define xHeinz_Solver_Callback_HPP

#include <set>
#include <vector>

#include <cstring>
#include <ilcplex/ilocplexi.h>

#include "Solver/Config.hpp"
#include "Solver/BkMaxFlow.hpp"
#include "Solver/MinCutDigraph.hpp"

namespace xHeinz {

 struct SolverConfig;

namespace solver {

 class ExtChainGraph;
 struct ConstExtGraph;

 class Callback {
  public:
   Callback( Config                const & conf
           , solver::ExtChainGraph const & gs
           );

  protected:
   using NodeSet       = std::set< Graph::Node >;
   using NodeSetVector = std::vector< NodeSet >;

  protected:
   Config const & config;

   ExtChainGraph const & graphs;

  protected:
   Callback( Callback const & other );

  protected:
   void constructRHS( GraphVariables const & vars
                    , NodeSet        const & dS
                    , NodeSet        const & S
                    , IloExpr              & rhs
                    ) const;

   bool isValid( Graph       const & g
               , Graph::Node const & i
               , NodeSet     const & dS
               , NodeSet     const & S
               ) const;

   void constructNonZeroComponents( Graph          const & g
                                  , GraphVariables const & vars
                                  , IloNumArray            x_values
                                  , NodeSetVector        & nonZeroNodesComponents                                   ) const;

   void printNonZeroX(IloNumArray x_values, int graphIndex) const;
   void printNonZeroY(IloNumArray y_values, int graphIndex) const;

 };


} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_Callback_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
