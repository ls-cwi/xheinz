#ifndef xHeinz_Solver_HeuristicCallback_HPP
#define xHeinz_Solver_HeuristicCallback_HPP

#include <cstring>
#include <vector>
#include <algorithm>

#include <ilcplex/ilocplexi.h>

#include "Solver/Config.hpp"
#include "Solver/ExtChainGraph.hpp"

namespace xHeinz {

 struct SolverConfig;

namespace solver {

 class HeuristicCallback
   : public IloCplex::HeuristicCallbackI {
  public:
   HeuristicCallback( IloEnv env
                    , Config const & conf
                    , ExtChainGraph const & gs
                    );
   ~HeuristicCallback();

  public:
   void main() override;
   IloCplex::CallbackI * duplicateCallback() const override;

  private:
   using IntNodeMap       = Graph::NodeMap< int >;
   using IntNodeMapVector = std::vector< IntNodeMap >;

   using NodeVector = std::vector< Graph::Node >;
   using NodeMatrix = std::vector< std::pair< double, NodeVector > >;

   using IndexPair = std::pair< int, int >;

  private:
   Config const & config;

   ExtChainGraph const & graphs;

   IntNodeMapVector componentMapVector;
   //IntNodeMapVector degreeMapVector;

   IloBoolVarArray solutionVar;
   IloNumArray solutionVal;

  private:
   HeuristicCallback( HeuristicCallback const & other );

  private:
   void determineComponents( int index
                           , NodeMatrix & nodesPerComponent
                           );

   IndexPair determinePair( int index
                          , NodeMatrix const & nodesPerComponent1
                          , NodeMatrix const & nodesPerComponent2
                          );

   double computeAlpha( int index
                      , NodeVector const & comp1
                      , NodeVector const & comp2
                      );

   void obtainSolution( int index
                      , int offset
                      , NodeVector const & comp1
                      , NodeVector const & comp2
                      );

   void obtainSolution( int index
                      , int offset
                      , NodeVector const & comp
                      );
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_HeuristicCallback_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
