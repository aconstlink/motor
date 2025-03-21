#pragma once

#include "inoise.h"
#include "../permutation_table.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>

#include <motor/std/vector>

namespace motor { namespace noise
{
    /// stores a table of num_entries pseudo random values internally.
    /// generates prns in [-1,1] space
    class MOTOR_NOISE_API value_noise : public inoise
    {
        motor_this_typedefs( value_noise ) ;

    private:

        motor::noise::permutation_table_t _pt ;
        motor::vector< float_t > _noises ;

    public:

        value_noise( void_t ) noexcept ;
        value_noise( uint_t const seed, uint_t bit, uint_t num_mixes=2 ) noexcept ;
        value_noise( this_rref_t )  noexcept;
        virtual ~value_noise( void_t ) noexcept ;

    public:

        this_ref_t operator = ( this_cref_t ) noexcept ;
        this_ref_t operator = ( this_rref_t ) noexcept ;

    public:

        float_t lattice( uint_t i0 ) const  noexcept;
        float_t lattice( uint_t i0, uint_t i1 ) const noexcept;
        float_t lattice( uint_t i0, uint_t i1, uint_t i2 ) const noexcept;

    public:

        /// @return value in [-1,1]
        virtual float_t noise( float_t f0 ) const noexcept;

        /// returns a noise value for a 2d input.
        /// @return value in [-1,1]
        virtual float_t noise( float_t f0, float_t f1 ) const noexcept;

        virtual float_t noise( float_t f0, float_t f1, float_t f2 ) const noexcept ;

        float_t noise(motor::math::vec2f_cref_t p) const noexcept ;
        float_t noise(motor::math::vec3f_cref_t p) const noexcept ;

    public:

        uint_t get_num_entries( void_t ) const noexcept;
    };
    motor_typedef( value_noise ) ;
} }