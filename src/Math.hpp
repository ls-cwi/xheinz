#ifndef xHeinz_Math_HPP
#define xHeinz_Math_HPP

#include <cmath>

#include "Config.hpp"

namespace xHeinz {

	inline bool positive( double v ) {   return epsilon < v;    }
	inline bool negative( double v ) {   return v < -epsilon;   }

	inline bool isNonZero( double v ) {   return  positive( v ) ||  negative( v );   }
	inline bool isZero   ( double v ) {   return !positive( v ) && !negative( v );   }

	inline bool lessThan   ( double a, double b ) {   return a+epsilon < b;   }
	inline bool greaterThan( double a, double b ) {   return a-epsilon > b;   }

	inline bool lessThanEq   ( double a, double b ) {   return a-epsilon <= b;   }
	inline bool greaterThanEq( double a, double b ) {   return a+epsilon >= b;   }

} // namespace xHeinz

#endif // xHeinz_Math_HPP
