
#include "gradient_noise.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/utility/fn.hpp>
#include <motor/math/utility/constants.hpp>

using namespace motor::noise ;

//***********************************************************************************
gradient_noise::gradient_noise( uint_t seed, uint_t bit, uint_t num_mixes ) noexcept : _pt( seed, bit, num_mixes )
{
    _noises.resize( this_t::get_num_entries() ) ; 

    bit = _pt.get_bit() ;

    uint_t const value_limit = ( 1u << bit ) - 1u ;

    for( uint_t i=0; i<get_num_entries(); ++i )
    {
        uint_t const ptv = _pt.permute_at(i) ;

        float_t const z = -((float_t)(( ptv & value_limit)/(double_t)value_limit)) *
            2.0f + 1.0f ;

        float_t const r = motor::math::fn<float_t>::sqrt( 1.0f - z*z ) ;
        float_t const theta = 2.0f * motor::math::constants<float_t>::pi() * 
            ((float_t)(( ptv & value_limit)/(double_t)value_limit));

        _noises[i] = motor::math::vec3f_t( 
            r*motor::math::fn<float_t>::cos(theta), 
            r*motor::math::fn<float_t>::sin(theta), 
            z ) ;
    }
}

//***********************************************************************************
gradient_noise::gradient_noise( this_rref_t rhv ) noexcept : 
    _pt( std::move( rhv._pt ) ), _noises ( std::move( rhv._noises ) )
{}

//***********************************************************************************
gradient_noise::~gradient_noise( void_t ) noexcept
{
}

//***********************************************************************************
gradient_noise::this_ref_t gradient_noise::operator = ( this_cref_t rhv ) noexcept
{
    _pt = rhv._pt ;
    _noises = rhv._noises ;

    return *this ;
}

//***********************************************************************************
gradient_noise::this_ref_t gradient_noise::operator = ( this_rref_t rhv ) noexcept
{
    _pt = std::move( rhv._pt ) ;
    _noises = std::move( rhv._noises ) ;

    return *this ;
}

//***********************************************************************************
float_t gradient_noise::lattice( uint_t i0, float_t f1 ) const  noexcept
{
    motor::math::vec3f_t const n = _noises[_pt.permute_at(i0)] ;
    return n.x() * f1  ;
}

//***********************************************************************************
float_t gradient_noise::lattice( uint_t i0, uint_t i1, float_t f1, float_t f2 ) const noexcept
{
    motor::math::vec3f_t const n = _noises[_pt.permute_at(i0, i1)] ;
    return n.x() * f1 + n.y() * f2 ;
}

//***********************************************************************************
float_t gradient_noise::lattice( uint_t i0, uint_t i1, uint_t i2, float_t f1, float_t f2, float_t f3 ) const noexcept
{
    motor::math::vec3f_t const n = _noises[_pt.permute_at(i0, i1, i2)] ;
    return n.x() * f1 + n.y() * f2 + n.z() * f3 ;
}

//***********************************************************************************
float_t gradient_noise::noise( motor::math::vec2f_cref_t p ) const noexcept 
{
    return this_t::noise( p.x(), p.y() ) ;
}

//***********************************************************************************
float_t gradient_noise::noise( motor::math::vec3f_cref_t p ) const noexcept 
{
    return this_t::noise( p.x(), p.y(), p.z() ) ;
}

//***********************************************************************************
float_t gradient_noise::noise( float_t x ) const   noexcept
{
    const uint_t ix = (uint_t)motor::math::fn<float_t>::floor(x) ;

    const float_t fx0 = x - (int_t)ix ;

    const float_t fx1 = fx0 - 1.0f ;

    const float_t wx = motor::math::fn<float_t>::smooth_step(fx0) ;  

    const float_t g00 = this_t::lattice( ix, fx0 ) ;
    const float_t g10 = this_t::lattice( ix+1, fx1 ) ;

    return motor::math::fn<float_t>::mix(wx, g00, g10) ;
}

//***********************************************************************************
float_t gradient_noise::noise( float_t x, float_t y ) const noexcept
{
    const uint_t ix = (uint_t)motor::math::fn<float_t>::floor(x) ;
    const uint_t iy = (uint_t)motor::math::fn<float_t>::floor(y) ;

    const float_t fx0 = x - (int_t)ix ;
    const float_t fy0 = y - (int_t)iy ;

    const float_t fx1 = fx0 - 1.0f ;
    const float_t fy1 = fy0 - 1.0f ;

    const float_t wx = motor::math::fn<float_t>::smooth_step(fx0) ;            
    const float_t wy = motor::math::fn<float_t>::smooth_step(fy0) ;

    const float_t g00 = this_t::lattice( ix, iy, fx0, fy0 ) ;
    const float_t g10 = this_t::lattice( ix+1, iy, fx1, fy0 ) ;
    const float_t g01 = this_t::lattice( ix, iy+1, fx0, fy1 ) ;
    const float_t g11 = this_t::lattice( ix+1, iy+1, fx1, fy1 ) ;
            
    const float_t l1 = motor::math::fn<float_t>::mix(wx, g00, g10) ;
    const float_t l2 = motor::math::fn<float_t>::mix(wx, g01, g11) ;
    return motor::math::fn<float_t>::mix(wy, l1, l2) ;
}

//***********************************************************************************
float_t gradient_noise::noise( float_t x, float_t y, float_t z ) const noexcept
{
    const uint_t ix = (uint_t)motor::math::fn<float_t>::floor(x) ;
    const uint_t iy = (uint_t)motor::math::fn<float_t>::floor(y) ;
    const uint_t iz = (uint_t)motor::math::fn<float_t>::floor(z) ;

    const float_t fx0 = x - (int_t)ix ;
    const float_t fy0 = y - (int_t)iy ;
    const float_t fz0 = z - (int_t)iz ;

    const float_t fx1 = fx0 - 1.0f ;
    const float_t fy1 = fy0 - 1.0f ;
    const float_t fz1 = fz0 - 1.0f ;

    const float_t wx = motor::math::fn<float_t>::smooth_step(fx0) ;            
    const float_t wy = motor::math::fn<float_t>::smooth_step(fy0) ;
    const float_t wz = motor::math::fn<float_t>::smooth_step(fz0) ;

    const float_t g000 = this_t::lattice( ix, iy, iz, fx0, fy0, fz0 ) ;
    const float_t g100 = this_t::lattice( ix+1, iy, iz, fx1, fy0, fz0 ) ;
    const float_t g010 = this_t::lattice( ix, iy+1, iz, fx0, fy1, fz0 ) ;
    const float_t g110 = this_t::lattice( ix+1, iy+1, iz, fx1, fy1, fz0 ) ;
    const float_t g001 = this_t::lattice( ix, iy, iz+1, fx0, fy0, fz1 ) ;
    const float_t g101 = this_t::lattice( ix+1, iy, iz+1, fx1, fy0, fz1 ) ;
    const float_t g011 = this_t::lattice( ix, iy+1, iz+1, fx0, fy1, fz1 ) ;
    const float_t g111 = this_t::lattice( ix+1, iy+1, iz+1, fx1, fy1, fz1 ) ;

    const float_t l1 = motor::math::fn<float_t>::mix(wx, g000, g100) ;
    const float_t l2 = motor::math::fn<float_t>::mix(wx, g010, g110) ;
    const float_t l3 = motor::math::fn<float_t>::mix(wx, g001, g101) ;
    const float_t l4 = motor::math::fn<float_t>::mix(wx, g011, g111) ;

    const float_t l5 = motor::math::fn<float_t>::mix(wy, l1, l2) ;
    const float_t l6 = motor::math::fn<float_t>::mix(wy, l3, l4) ;

    return motor::math::fn<float_t>::mix(wz, l5, l6) ;
}

//***********************************************************************************
uint_t gradient_noise::get_num_entries( void_t ) const noexcept
{ 
    return _pt.get_num_entries() ; 
}

