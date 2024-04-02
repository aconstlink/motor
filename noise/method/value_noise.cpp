
#include "value_noise.h"

#include "../permutation_table.h"

#include <motor/math/interpolation/interpolate.hpp>
#include <motor/math/utility/fn.hpp>

using namespace motor::noise ;

//***********************************************************************************
value_noise::value_noise( uint_t const seed, uint_t bit, uint_t const num_mixes ) noexcept : _pt( seed, bit, num_mixes )
{
    _noises.resize( this_t::get_num_entries() ) ;

    bit = _pt.get_bit() ;
    uint_t const value_limit = (1u<<bit)-1u ;
    
    for( uint_t i=0; i<this_t::get_num_entries(); ++i )
    {
        uint_t const ptv = _pt.permute_at(i) ;

        _noises[i] = ((float_t)(( ptv & value_limit)/(double_t)value_limit))*2.0f - 1.0f  ;
    }
}

//***********************************************************************************
value_noise::value_noise( this_rref_t rhv ) noexcept : _pt( std::move( rhv._pt ) )
{
    _noises = std::move( rhv._noises ) ;
}

//***********************************************************************************
value_noise::~value_noise( void_t ) noexcept
{
}

//***********************************************************************************
float_t value_noise::lattice( uint_t i0 ) const noexcept
{
    return _noises[_pt.permute_at(i0)] ;
}

//***********************************************************************************
float_t value_noise::lattice( uint_t i0, uint_t i1 ) const noexcept
{
    return _noises[_pt.permute_at(i0, i1)] ;
}

//***********************************************************************************
float_t value_noise::lattice( uint_t i0, uint_t i1, uint_t i2 ) const noexcept
{
    return _noises[_pt.permute_at(i0, i1, i2)] ;
}

//***********************************************************************************
float_t value_noise::noise( motor::math::vec2f_cref_t p ) const noexcept
{
    return this_t::noise( p.x(), p.y() ) ;
}

//***********************************************************************************
float_t value_noise::noise( motor::math::vec3f_cref_t p ) const noexcept
{
    return this_t::noise( p.x(), p.y(), p.z() ) ;
}

//***********************************************************************************
float_t value_noise::noise( float_t f0 ) const noexcept
{
    int_t const i0 = int_t(std::floor(f0)) ;
    float_t const frac = f0 - float_t(i0) ;

    float_t const l0 = this_t::lattice(i0+0) ;
    float_t const l1 = this_t::lattice(i0+1) ;

    return motor::math::interpolation<float_t>::linear
        ( l0, l1, motor::math::fn<float_t>::smooth_step(frac) ) ;
}

//***********************************************************************************
float_t value_noise::noise( float_t f0, float_t f1 ) const noexcept
{
    int_t const i0 = int_t(std::floor(f0)) ;
    float_t const frac0 = f0 - float_t(i0) ;

    int_t const i1 = int_t(std::floor(f1)) ;
    float_t const frac1 = f1 - float_t(i1) ;

    float_t const l00 = this_t::lattice(uint_t(i0+0), uint_t(i1+0)) ;
    float_t const l01 = this_t::lattice(uint_t(i0+1), uint_t(i1+0)) ;
    float_t const l10 = this_t::lattice(uint_t(i0+0), uint_t(i1+1)) ;
    float_t const l11 = this_t::lattice(uint_t(i0+1), uint_t(i1+1)) ;
            
    float_t const v0 = motor::math::interpolation<float_t>::linear
        ( l00, l01, motor::math::fn<float_t>::smooth_step(frac0) ) ;

    float_t const v1 = motor::math::interpolation<float_t>::linear
        ( l10, l11, motor::math::fn<float_t>::smooth_step(frac0) ) ;

    return motor::math::interpolation<float_t>::linear
        ( v0, v1, motor::math::fn<float_t>::smooth_step(frac1) ) ;
}

//***********************************************************************************
float_t value_noise::noise( float_t /*f0*/, float_t /*f1*/, float_t /*f2*/ ) const noexcept
{
    return 0.0f ;
}

//***********************************************************************************
uint_t value_noise::get_num_entries( void_t ) const noexcept
{ 
    return _pt.get_num_entries() ; 
}