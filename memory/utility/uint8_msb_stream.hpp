
#pragma once

#include "../typedefs.h"
#include "uint8_split_merge.hpp"

namespace motor
{
    namespace memory
    {
        /// streams certain numbers to an array set in the ctor.
        /// @note not thread safe
        class uint8_msb_stream
        {
            typedef uint8_msb_stream this_t ;
            typedef this_t & this_ref_t ;

            uint8_t * _array ;
            size_t _max_len ;

            size_t _cur_pos = size_t(0) ;

        public:

            uint8_msb_stream( uint8_t * ar, size_t max_len ) noexcept : _array(ar), _max_len(max_len) {}

        public:

            this_ref_t operator << ( uint8_t const n ) noexcept
            {
                _array[_cur_pos++] = n ;
                return *this ;
            }

            this_ref_t operator << ( uint16_t const n ) noexcept
            {
                motor::memory::uint8_msb_split::of( n, &_array[_cur_pos] ) ;
                _cur_pos += 2 ;
                return *this ;
            }

            this_ref_t operator << ( uint32_t const n ) noexcept
            {
                motor::memory::uint8_msb_split::of( n, &_array[_cur_pos] ) ;
                _cur_pos += 4 ;
                return *this ;
            }

            this_ref_t operator << ( uint64_t const n ) noexcept
            {
                motor::memory::uint8_msb_split::of( n, &_array[_cur_pos] ) ;
                _cur_pos += 8 ;
                return *this ;
            }

        };

        class uint8c_msb_stream
        {
            typedef uint8c_msb_stream this_t ;
            typedef this_t & this_ref_t ;

            uint8_t const * _array ;
            size_t _max_len ;

            size_t _cur_pos = size_t(0) ;

        public:

            uint8c_msb_stream( uint8_t const * ar, size_t max_len ) noexcept : _array(ar), _max_len(max_len) {}

        public:

            this_ref_t operator >> ( uint8_t & n ) noexcept
            {
                n = _array[_cur_pos++] ;
                return *this ;
            }

            this_ref_t operator >> ( uint16_t & n ) noexcept
            {
                n = motor::memory::uint8_msb_merge::uint16( &_array[_cur_pos] ) ;
                _cur_pos += 2 ;
                return *this ;
            }

            this_ref_t operator >> ( uint32_t & n ) noexcept
            {
                n = motor::memory::uint8_msb_merge::uint32( &_array[_cur_pos] ) ;
                _cur_pos += 4 ;
                return *this ;
            }

            this_ref_t operator >> ( uint64_t & n ) noexcept
            {
                n = motor::memory::uint8_msb_merge::uint64( &_array[_cur_pos] ) ;
                _cur_pos += 8 ;
                return *this ;
            }

        };
    }
}
