#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace memory
    {
        /// the most significant byte is written 
        /// first into the stream
        struct uint8_msb_split
        {
            /// split a 16 bit number into 2 bytes
            /// @todo SIMD/Vector operation
            /// @precondition to must have at least 2 byte space
            static void_t of( uint16_t const number, uint8_t * to ) noexcept
            {
                // vec2(number) >> vec2(8,0) & vec2(255)
                *(to+0) = (number >> 8) & 255 ;
                *(to+1) = (number >> 0) & 255 ;
            }

            /// split a 32 bit number into 4 bytes
            /// @todo SIMD/Vector operation
            /// @precondition to must have at least 4 byte space
            static void_t of( uint32_t const number, uint8_t * to ) noexcept
            {
                // vec4(number) >> vec4(24,16,8,0) & vec4(255)
                *(to+0) = (number >> 24) & 255 ;
                *(to+1) = (number >> 16) & 255 ;
                *(to+2) = (number >> 8) & 255 ;
                *(to+3) = (number >> 0) & 255 ;
            }

            /// split a 64 bit number into 8 bytes
            /// @todo SIMD/Vector operation
            /// @precondition to must have at least 8 byte space
            static void_t of( uint64_t const number, uint8_t * to ) noexcept
            {
                // vec4(number) >> vec4(56,48,40,32) & vec4(255)
                // vec4(number) >> vec4(24,16,8,0) & vec4(255)
                *(to+0) = (number >> 56) & 255 ;
                *(to+1) = (number >> 48) & 255 ;
                *(to+2) = (number >> 40) & 255 ;
                *(to+3) = (number >> 32) & 255 ;
                *(to+4) = (number >> 24) & 255 ;
                *(to+5) = (number >> 16) & 255 ;
                *(to+6) = (number >> 8) & 255 ;
                *(to+7) = (number >> 0) & 255 ;
            }
        };

        struct uint8_msb_merge
        {
            static uint16_t uint16( uint8_t const * from ) noexcept
            {
                uint16_t const at_1 = *(from+1) ;
                uint16_t const at_0 = *(from+0) ;

                return uint16_t( (at_0 << 8) | (at_1 << 0) ) ;
            }

            static uint32_t uint32( uint8_t const * from ) noexcept
            {
                uint32_t const at_3 = *(from+3) ;
                uint32_t const at_2 = *(from+2) ;
                uint32_t const at_1 = *(from+1) ;
                uint32_t const at_0 = *(from+0) ;

                return uint32_t( (at_0 << 24) | (at_1 << 16) | (at_2 << 8) | (at_3 << 0) ) ;
            }

            static uint64_t uint64( uint8_t const * from ) noexcept
            {
                uint64_t const at_7 = *(from+7) ;
                uint64_t const at_6 = *(from+6) ;
                uint64_t const at_5 = *(from+5) ;
                uint64_t const at_4 = *(from+4) ;
                uint64_t const at_3 = *(from+3) ;
                uint64_t const at_2 = *(from+2) ;
                uint64_t const at_1 = *(from+1) ;
                uint64_t const at_0 = *(from+0) ;

                return uint64_t( 
                    (at_0 << 56) | (at_1 << 48) | (at_2 << 40) | (at_3 << 32) | 
                    (at_4 << 24) | (at_5 << 16) | (at_6 << 8) | (at_7 << 0) ) ;
            }

        };
    }
}
