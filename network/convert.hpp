#pragma once


#include "typedefs.h"
#include <motor/memory/utility/uint8_split_merge.hpp>

namespace motor
{
    namespace network
    {
        /// register -> network byte order(nbo)
        struct to_nbo
        {
            static void_t of( uint16_t const number, uint8_t * to )
            {
                motor::memory::uint8_msb_split::of( number, to ) ;
            }

            static void_t of( uint32_t const number, uint8_t * to )
            {
                motor::memory::uint8_msb_split::of( number, to ) ;
            }

            static void_t of( uint64_t const number, uint8_t * to )
            {
                motor::memory::uint8_msb_split::of( number, to ) ;
            }
        } ;

        /// network byte order(nbo) -> register
        struct from_nbo
        {
            static uint16_t data_16( const uint8_t * from )
            {
                return motor::memory::uint8_msb_merge::uint16( from ) ;
            }

            static uint32_t data_32( const uint8_t * from )
            {
                return motor::memory::uint8_msb_merge::uint32( from ) ;
            }

            static uint64_t data_64( const uint8_t * from )
            {
                return motor::memory::uint8_msb_merge::uint64( from ) ;
            }
        } ;
    }
}
