
#pragma once

#include <motor/base/types.hpp>
#include <motor/std/vector>

#include <functional>
#include <array>

namespace motor
{
    namespace mstd
    {
        using namespace motor::core::types ;

        template< typename CONT_T >
        class insertion_sort
        {
            using this_t = insertion_sort< CONT_T > ;
            using container_t = CONT_T ;
            using container_ref_t = container_t & ;

        public:

            using swap_funk_t = std::function< void_t ( size_t const a, size_t const b ) > ;
            using compare_funk_t = std::function< bool_t ( typename container_t::value_type const &, typename container_t::value_type const & ) > ;
            
            // calls the swap function funk when 
            // two items are swapped.
            static void_t in_range_with_swap( size_t const a, size_t const b, container_ref_t points, swap_funk_t funk = []( size_t const a, size_t const b ){} )
            {
                return this_t::the_sort( a, b, points, funk ) ;
            }

            static void_t for_all_with_swap( container_ref_t points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                return this_t::the_sort( 0, points.size(), points, funk ) ;
            }

            // calls the compare function each iteration.
            // if funk -> true the iteration breaks. Default is the < operator for ascending order
            static void_t for_all( container_ref_t points, compare_funk_t funk = []( typename container_t::value_type const & a, typename container_t::value_type const & b ){ return a < b ; } )
            {
                return this_t::the_sort_compare( 0, points.size(), points, funk ) ;
            }

        private:

            static void_t the_sort( size_t const a, size_t const b, container_ref_t points, swap_funk_t funk = []( size_t const a, size_t const b){} )
            {
                for( size_t i=a+1; i<b; ++i )
                {
                    auto tmp = points[i] ;

                    for( size_t j=i-1; j!=(a-1); --j )
                    {
                        size_t const i2 = j + 1 ;
            
                        if( points[j] < tmp ) break ;
                        points[i2] = points[j] ;
                        points[j] = tmp ;
                        funk( j - a, i2 - a ) ;
                    }
                }
            }

            static void_t the_sort_compare( size_t const a, size_t const b, container_ref_t points, compare_funk_t funk )
            {
                for( size_t i=a+1; i<b; ++i )
                {
                    auto tmp = points[i] ;

                    for( size_t j=i-1; j!=(a-1); --j )
                    {
                        size_t const i2 = j + 1 ;
            
                        if( funk( points[j], tmp ) ) break ;
                        points[i2] = points[j] ;
                        points[j] = tmp ;
                    }
                }
            }
        };
    }
}
