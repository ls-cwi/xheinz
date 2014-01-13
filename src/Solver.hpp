#ifndef xHeinz_Solver_HPP
#define xHeinz_Solver_HPP

#include <cassert>
#include <ostream>

#include "Solver/Config.hpp"
#include "Solver/Solution.hpp"
#include "Solver/ExtChainGraph.hpp"

namespace xHeinz {

 class Solver {
  public:
   using Config = solver::Config;

   using InputSolution  = solver::InputSolution;
   using OutputSolution = solver::OutputSolution;

  public:
   Solver( ChainGraph const & gs, Config conf );
   ~Solver();

  public:
   void warm( InputSolution const & sol, char const * name = nullptr );

   optional< OutputSolution > solve();

  private:
   using NodeVector = std::vector< Graph::Node >;
   using BoolVector = std::vector< bool >;

   using ExtChainGraph      = solver::ExtChainGraph;
   using GraphVariables     = solver::GraphVariables;
   using LinkGraphVariables = solver::LinkGraphVariables;

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

   double computeAlpha( ThreeWayGraph const         & twg
                      , LinkGraphVariables const    & linkVars
                      , NodeVector  const           & comp1
                      , NodeVector  const           & comp2
                      , OutputSolution::SolutionSet & out1
                      , OutputSolution::SolutionSet & out2
                      );

   double determineSolution( Graph const          & graph
                           , GraphVariables const & data
                           , NodeVector           & solution
                           );
 };

} // namespace xHeinz

#endif // xHeinz_Solver_Hpp

/* vim: set ts=8 sw=2 sts=2 et : */
