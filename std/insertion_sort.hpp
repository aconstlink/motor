
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

        template< typename T >
        class insertion_sort
        {
            using this_t = insertion_sort< T > ;

        public:

            typedef std::function< void_t ( size_t const a, size_t const b ) > swap_funk_t ;

            
            static void_t in_range( size_t const a, size_t const b, motor::vector< T > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                return this_t::the_sort( a, b, points, funk ) ;
            }

            static void_t for_all( motor::vector< T > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                return this_t::the_sort( 0, points.size(), points, funk ) ;
            }

            template< size_t s >
            static void_t for_all( std::array< T, s > & points, swap_funk_t funk = [](size_t const /*a*/, size_t const /*b*/){} )
            {
                return this_t::the_sort<s>( 0, points.size(), points, funk ) ;
            }

        private:

            static void_t the_sort( size_t const a, size_t const b, motor::vector< T > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                for( size_t i=a+1; i<b; ++i )
                {
                    T tmp = points[i] ;

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

            template< size_t s >
            static void_t the_sort( size_t const a, size_t const b, std::array< T, s > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                for( size_t i=a+1; i<b; ++i )
                {
                    T tmp = points[i] ;

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
        };
    }
}
