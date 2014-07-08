#ifndef xHeinz_Solver_HPP
#define xHeinz_Solver_HPP

#include <cassert>
#include <ostream>
#include <utility>
#include <tuple>

#include "Solver/Config.hpp"
#include "Solver/Solution.hpp"
#include "Solver/ExtChainGraph.hpp"

namespace xHeinz {

 class Solver {
  public:
   using Config = solver::Config;

   using InputSolution  = solver::InputSolution;
   using OutputSolution = solver::OutputSolution;

   using NodeBoolPair = OutputSolution::NodeBoolPair;
   using SolutionSet  = OutputSolution::SolutionSet;

  public:
   Solver( ChainGraph const & gs, Config conf );
   ~Solver() noexcept;

  public:
   void warm( InputSolution const & sol, char const * name = nullptr );

   optional< OutputSolution > solve();

  private:
   using ExtChainGraph      = solver::ExtChainGraph;
   using GraphVariables     = solver::GraphVariables;
   using LinkGraphVariables = solver::LinkGraphVariables;

   using AlphaTriple = std::tuple< OutputSolution::AlphaScore
                                 , OutputSolution::AlphaScore
                                 , OutputSolution::AlphaScore
                                 >;
  private:
   Config config;

   ExtChainGraph graphs;

   IloEnv   env;
   IloModel model;
   IloCplex cplex;

  private:
   void initGraphVariables( Graph const    & graph
                          , GraphVariables & data
                          );

   void initLinkGraphVariables( ThreeWayGraph const & twg
                              , LinkGraphVariables  & data
                              );

   void initVariables();

   void initGraphConstraints( Graph const    & graph
                            , GraphVariables & data
                            );

   void initLinkGraphConstraints( ThreeWayGraph const  & twg
                                , GraphVariables const & redData
                                , GraphVariables const & blueData
                                , LinkGraphVariables   & data
                                );
   void initConstraints();

   void initObjectiveFunction( ExtChainGraph const & graphs );

   bool setParamsAndSolve();

   OutputSolution determineSolution() const;

   SolutionSet extractSolFromCplexVars( Graph const          & graph
                                      , GraphVariables const & data
                                      ) const;

   AlphaTriple fillMappedAndComputeAlpha( ThreeWayGraph const      & twg
                                        , LinkGraphVariables const & linkVars
                                        , SolutionSet & solSet0
                                        , SolutionSet & solSet1
                                        ) const;
 };

} // namespace xHeinz

#endif // xHeinz_Solver_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
