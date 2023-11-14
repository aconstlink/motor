
#pragma once

#include "typedefs.h"

#include <natus/core/macros/typedef.h>
#include <functional>
#include <array>

namespace natus
{
    namespace ntd
    {
        template< typename T >
        class insertion_sort
        {
            natus_this_typedefs( insertion_sort< T > ) ;

        public:

            typedef std::function< void_t ( size_t const a, size_t const b ) > swap_funk_t ;

            
            static void_t in_range( size_t const a, size_t const b, std::vector< T > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                return this_t::the_sort( a, b, points, funk ) ;
            }

            static void_t for_all( std::vector< T > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
            {
                return this_t::the_sort( 0, points.size(), points, funk ) ;
            }

            template< size_t s >
            static void_t for_all( std::array< T, s > & points, swap_funk_t funk = [](size_t const /*a*/, size_t const /*b*/){} )
            {
                return this_t::the_sort<s>( 0, points.size(), points, funk ) ;
            }

        private:

            static void_t the_sort( size_t const a, size_t const b, std::vector< T > & points, swap_funk_t funk = [](size_t const a, size_t const b){} )
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
