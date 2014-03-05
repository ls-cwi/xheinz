#ifndef xHeinz_Solver_LazyConstraintCallback_HPP
#define xHeinz_Solver_LazyConstraintCallback_HPP

#include <cstring>
#include <ilcplex/ilocplexi.h>

#include "Solver/Callback.hpp"

namespace xHeinz {

namespace solver {

 class LazyConstraintCallback
   : public IloCplex::LazyConstraintCallbackI
   , public Callback {
  public:
   LazyConstraintCallback( IloEnv env
                         , Config const & conf
                         , solver::ExtChainGraph const & gs
                         );

  public:
   void main() override;
   IloCplex::CallbackI * duplicateCallback() const override;

  private:
   LazyConstraintCallback( LazyConstraintCallback const & other );

  private:
   void processGraph( int index );

   Graph::Node determineRoot( Graph          const & g
                            , GraphVariables const & vars
                            , IloNumArray    const & y_values
                            ) const;

   void separateConnectedComponents( Graph          const & g
                                   , GraphVariables const & vars
                                   , Graph::Node    const & root
                                   , NodeSetVector  const & nonZeroNodesComponents
                                   , int                  & nCuts
                                   );
 };

} // namespace solver
} // namespace xHeinz

#endif // xHeinz_Solver_LazyConstraintCallback_HPP

/* vim: set ts=8 sw=2 sts=2 et : */
