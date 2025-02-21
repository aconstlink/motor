
#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        class time
        {
        public:

            struct time_info
            {
                size_t day ;
                size_t hour ;
                size_t min ;
                size_t sec ;
                size_t mil ;
            };

        public:

            static size_t to_milli( size_t const min, size_t const sec, size_t const milli ) noexcept
            {
                size_t const min_to_sec = min * 60 ;
                size_t const sec_to_milli = (min_to_sec + sec) * 1000 ;

                return sec_to_milli + milli ;
            }

            static void_t milli_to( size_t const milli, time_info & ret ) noexcept
            {
                size_t const sec = milli / 1000 ;
                size_t const min = sec / 60 ;
                size_t const hour = min / 60 ;
                size_t const day = hour / 24 ;

                ret.mil = milli % 1000 ;
                ret.sec = sec % 60 ;
                ret.min = min % 60 ; 
                ret.hour = hour % 60 ;
                ret.day = day ;
            }
        };
    }
}