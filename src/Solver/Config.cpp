#include "Solver/Config.hpp"

#include <ostream>

using namespace std;

namespace xHeinz {
namespace solver {

 ostream & operator<<( ostream & out, Config const & config ) {
   out << "-- alpha      = " << config.connectivityPercentage;
   switch( config.connectivityType ) {
    case Config::ConnectivityType::SumUnits:
      out << "\n-- omega      = 0 (Sum mapped nodes)";                   break;
    case Config::ConnectivityType::SumWeights:
      out << "\n-- omega      = 1 (Sum mapped node weights)";            break;
    case Config::ConnectivityType::SumPosUnits:
      out << "\n-- omega      = 2 (Sum positive mapped nodes)";          break;
    case Config::ConnectivityType::SumPosWeights:
      out << "\n-- omega      = 3 (Sum positive mapped node weights)";   break;
   }
   out << "\n-- size       = " << config.size
       << "\n-- time limit = " << config.timeLimit
       << "\n-- # threads  = " << config.numThreads;
   return out;
 }

} // namespace solver
} // namespace xHeinz

/* vim: set ts=8 sw=2 sts=2 et : */
