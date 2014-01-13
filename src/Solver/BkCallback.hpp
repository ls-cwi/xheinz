#ifndef xHeinz_Solver_BkCallback_HPP
#define xHeinz_Solver_BkCallback_HPP

#include <set>

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

 class BkCallback
   : public IloCplex::LazyConstraintCallbackI {
  public:
   BkCallback( IloEnv env
             , Config const & conf
             , solver::ExtChainGraph const & gs
             );

  public:
   void main() override;
   IloCplex::CallbackI * duplicateCallback() const override;

  private:
   using PerGraphMinCut = std::vector< BkMaxFlow >;        // 1 per component
   using GraphsMinCut   = std::vector< PerGraphMinCut >;   // 1 per graph

   using NodeVector = std::vector< GraphComponent::Node >;
   using NodeMatrix = std::vector< NodeVector >;

  private:
   Config const & config;

   ExtChainGraph const & graphs;

   GraphsMinCut minCuts;

  private:
   BkCallback( BkCallback const & other );

  private:
   void processGraph( int index );

   void constructNonZeroComponents( GraphComponent const         & component
                                  , MinCutDigraphComponent const & digraph
                                  , GraphVariables         const & vars
                                  , IloNumArray                    x_values
                                  , NodeMatrix                   & nodesPerNonZeroComponent
                                  ) const;

   void determineFwdCutSet( GraphComponent const &  component
                          , std::set< Graph::Node > & dS
                          , std::set< Graph::Node > & S
                          , MinCutDigraphComponent const & digraph
                          , BkMaxFlow const              & bkMaxFlow
                          , MinCutDigraphComponent::Node const & target
                          ) const;
   void determineBwdCutSet( GraphComponent const &  component
                          , std::set< Graph::Node > & dS
                          , std::set< Graph::Node > & S
                          , MinCutDigraphComponent const & digraph
                          , BkMaxFlow const              & bkMaxFlow
                          , MinCutDigraphComponent::Node const & target
                          ) const;

   void addViolatedConstraint( Graph::Node                     target
                             , std::set< Graph::Node > const & dS
                             , std::set< Graph::Node > const & S
                             , GraphVariables const & vars
                             );

   void process( GraphComponent const         & component
               , NodeVector const             & nodes
               , double                         x_i_value
               , MinCutDigraphComponent const & digraph
               , BkMaxFlow                    & bkMaxFlow
               , IloNumArray                    x_values
               , IloNumArray                    y_values
               , GraphVariables         const & vars
               , int                          & nCuts
               , int                          & nNestedCuts
               , int                          & nBackCuts
               );

   void printNonZeroX(IloNumArray x_values, int graphIndex) const;
   void printNonZeroY(IloNumArray y_values, int graphIndex) const;
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_BkCallback_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
