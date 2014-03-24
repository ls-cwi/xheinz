#ifndef xHeinz_Solver_UserCutCallback_HPP
#define xHeinz_Solver_UserCutCallback_HPP

#include <cstring>
#include <ilcplex/ilocplexi.h>

#include "Solver/Callback.hpp"
#include "Solver/Backoff.hpp"

namespace xHeinz {

namespace solver {

 class UserCutCallback
   : public IloCplex::UserCutCallbackI
   , public Callback {
  public:
   UserCutCallback( IloEnv                        env
                  , Config                const & conf
                  , solver::ExtChainGraph const & gs
                  , BackOff               const & bo
                  );

  public:
   void main() override;
   IloCplex::CallbackI * duplicateCallback() const override;

   // I need to access IloCplex::UserCutCallbackI, which is protected, in Callback
   //friend class Callback;

  private:
   using PerGraphMinCut = std::vector< BkMaxFlow >;        // 1 per component
   using GraphsMinCut   = std::vector< PerGraphMinCut >;   // 1 per graph

  private:
   GraphsMinCut minCuts;
   int nodeNumber;
   int numCutIterations;
   bool makeAttempt;
   BackOff backOff;
   int timeStamp;

  private:
   UserCutCallback( UserCutCallback const & other );

  private:
   void processGraph( int index );

   void separateMinCut( Graph          const & graph
                      , MinCutDigraph  const & digraph
                      , PerGraphMinCut       & graphMinCuts
                      , GraphVariables const & vars
                      , IloNumArray    const & x_values
                      , IloNumArray    const & y_values
                      , int                  & nCuts
                      , int                  & nBackCuts
                      , int                  & nNestedCuts
                      );

   void separateMinCut( Graph                  const & graph
                      , GraphComponent const         & component
                      , MinCutDigraphComponent const & digraph
                      , BkMaxFlow                    & bkMaxFlow
                      , IloNumArray                    x_values
                      , IloNumArray                    y_values
                      , GraphVariables         const & vars
                      , int                          & nCuts
                      , int                          & nNestedCuts
                      , int                          & nBackCuts
                      );

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

   NodeSet determineRoots( Graph          const & g
                         , GraphVariables const & vars
                         , IloNumArray    const & y_values
                         ) const;

   void separateConnectedComponents( Graph          const & g
                                   , GraphVariables const & vars
                                   , NodeSet        const & roots
                                   , NodeSetVector  const & nonZeroNodesComponents
                                   , IloNumArray    const & x_values
                                   , IloNumArray    const & y_values
                                   , int                  & nCuts
                                   );

   //void addViolatedConstraint( Graph::Node                     target
   //                          , std::set< Graph::Node > const & dS
   //                          , std::set< Graph::Node > const & S
   //                          , GraphVariables const & vars
   //                          );
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_UserCutCallback_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
