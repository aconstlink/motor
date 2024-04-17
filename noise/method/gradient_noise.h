
#pragma once

#include "inoise.h"
#include "../permutation_table.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/utility/fn.hpp>
#include <motor/math/interpolation/interpolate.hpp>

#include <vector>

namespace motor { namespace noise
{
    /// stores a table of num_entries pseudo random values internally.
    /// generates prns in [-1,1] space
    class MOTOR_NOISE_API gradient_noise : public inoise
    {
        motor_this_typedefs( gradient_noise ) ;

    public:

        using type_t = float_t ;

    private:

        motor::noise::permutation_table_t _pt ;
        motor::vector< motor::math::vec3f_t > _noises ;

    public:

        gradient_noise( uint_t seed, uint_t bit, uint_t num_mixes=2 ) noexcept ;
        gradient_noise( this_cref_t ) = delete ;
        gradient_noise( this_rref_t rhv ) noexcept ;
        virtual ~gradient_noise( void_t ) noexcept ;

        this_ref_t operator = ( this_cref_t ) noexcept ;
        this_ref_t operator = ( this_rref_t ) noexcept ;

    private:

        float_t lattice( uint_t i0, float_t f1 ) const noexcept ;
        float_t lattice( uint_t i0, uint_t i1, float_t f1, float_t f2 ) const noexcept ;
        float_t lattice( uint_t i0, uint_t i1, uint_t i2, float_t f1, float_t f2, float_t f3 ) const noexcept ;
        
    public:

        virtual float_t noise( float_t x ) const  noexcept ;
        virtual float_t noise( float_t x, float_t y ) const noexcept ;
        virtual float_t noise( float_t x, float_t y, float_t z ) const noexcept ;

        float_t noise( motor::math::vec2f_cref_t p ) const noexcept ;
        float_t noise( motor::math::vec3f_cref_t p ) const noexcept ;

    public:

        uint_t get_num_entries( void_t ) const noexcept ;
    };
    motor_typedef( gradient_noise ) ;
} }
