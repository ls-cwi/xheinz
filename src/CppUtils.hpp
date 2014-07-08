#ifndef xHeinz_CppUtils_HPP
#define xHeinz_CppUtils_HPP

#include <cmath>

#include <algorithm>
#include <memory>

#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/iterator_range.hpp>

namespace xHeinz {

	using std::unique_ptr;
	template< typename T, typename ... Args >
	std::unique_ptr< T > make_unique( Args && ... args ) {
			return unique_ptr< T >( new T( std::forward< Args >( args )... ) );
	}

	using std::shared_ptr;
	using std::make_shared;
	using std::weak_ptr;
	template< typename T >
	std::weak_ptr< T > make_weak( shared_ptr< T > const & ptr ) {
			return weak_ptr< T >( ptr );
	}

	using std::for_each;
	using std::sort;
	using std::accumulate;
	using boost::for_each;
	using boost::sort;
	using boost::accumulate;

	using boost::optional;
	using boost::none;

	using boost::iterator_facade;
	using boost::forward_traversal_tag;

	using boost::iterator_range;
	using boost::make_iterator_range;

} // namespace xHeinz

#endif // xHeinz_CppUtils_HPP
