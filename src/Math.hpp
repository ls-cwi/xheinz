#ifndef xHeinz_Math_HPP
#define xHeinz_Math_HPP

#include <cmath>

#include "Config.hpp"

namespace xHeinz {

  inline bool cutPositive( double v ) {   return cut_epsilon < v;    }
  inline bool cutNegative( double v ) {   return v < -cut_epsilon;   }

  inline bool cutIsNonZero( double v ) {   return  cutPositive( v ) ||  cutNegative( v );   }
  inline bool cutIsZero   ( double v ) {   return !cutPositive( v ) && !cutNegative( v );   }

  inline bool cutLessThan   ( double a, double b ) {   return a+cut_epsilon < b;   }
  inline bool cutGreaterThan( double a, double b ) {   return a-cut_epsilon > b;   }

  inline bool cutLessThanEq   ( double a, double b ) {   return a-cut_epsilon <= b;   }
  inline bool cutGreaterThanEq( double a, double b ) {   return a+cut_epsilon >= b;   }

  inline bool intPositive( double v ) {   return int_epsilon < v;    }
  inline bool intNegative( double v ) {   return v < -int_epsilon;   }

  inline bool intIsNonZero( double v ) {   return  intPositive( v ) ||  intNegative( v );   }
  inline bool intIsZero   ( double v ) {   return !intPositive( v ) && !intNegative( v );   }

  inline bool intLessThan   ( double a, double b ) {   return a+int_epsilon < b;   }
  inline bool intGreaterThan( double a, double b ) {   return a-int_epsilon > b;   }

  inline bool intLessThanEq   ( double a, double b ) {   return a-int_epsilon <= b;   }
  inline bool intGreaterThanEq( double a, double b ) {   return a+int_epsilon >= b;   }

} // namespace xHeinz

#endif // xHeinz_Math_HPP
