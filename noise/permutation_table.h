#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/std/vector>

#include <algorithm>

namespace motor { namespace noise
{
    /// contains random values. It's primary usage is to 
    /// generate indices.
    /// there will be 2^bit values.
    class MOTOR_NOISE_API permutation_table
    {
        motor_this_typedefs( permutation_table ) ;

    private:

        uint_t _bit ;
        uint_t _seed ;
        uint_t _upper_bound ;

        motor::vector< uint_t > _indices ;

    public:

        permutation_table( uint_t const seed, uint_t const bit, uint_t num_mixes ) noexcept ;
        permutation_table( this_cref_t rhv ) noexcept ;
        permutation_table( this_rref_t rhv ) noexcept ;
        virtual ~permutation_table( void_t ) noexcept ;

        this_ref_t operator = ( this_cref_t ) noexcept ;
        this_ref_t operator = ( this_rref_t ) noexcept ;

    public:

        uint_t permute_at( uint_t const i0 ) const noexcept ;
        uint_t permute_at( uint_t const i0, uint_t const i1 ) const noexcept ;
        uint_t permute_at( uint_t const i0, uint_t const i1, uint_t const i2 ) const noexcept ;
        uint_t get_num_entries( void_t ) const noexcept ;
        uint_t get_bit( void_t ) const noexcept ;
        uint_t get_upper_bound( void_t ) const noexcept ;

    private:

        /// not checked if in range. 
        /// since this is a private method, check i before using it.
        inline uint_t perm( uint_t i ) const noexcept ;
    };
    motor_typedef( permutation_table ) ;

} }
