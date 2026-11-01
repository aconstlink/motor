// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"
#include <motor/math/vector/vector3.hpp>
#include <motor/math/utility/3d/transformation.hpp>

namespace motor::collide::c3d
{
template < typename type_t >
class aabb
{
    motor_this_typedefs( aabb< type_t > ) ;

    motor_typedefs( motor::math::vector3< type_t >, vec3 ) ;
    
    typedef motor::math::vec3b_t vec3b_t;

  private:

    vec3_t _max;
    vec3_t _min;

  public:

    aabb( void_t ) {}

    aabb( vec3_cref_t max, vec3_cref_t min )
    {
        _max = max;
        _min = min;
    }

    aabb( vec3_cref_t center, type_t radius )
    {
        const vec3_t vmin = center - vec3_t( radius );
        const vec3_t vmax = center + vec3_t( radius );

        _max = vmax;
        _min = vmin;
    }

    aabb( this_cref_t rhv )
    {
        ( *this ) = rhv;
    }

  public: // test

    /// is point inside the aabb.
    inline bool_t is_inside( vec3_cref_t p ) const
    {
        const vec3b_t test_max = p.less_than( _max );
        const vec3b_t test_min = p.greater_than( _min );

        return test_max.all() && test_min.all();
    }

  public: // transformation

    inline this_ref_t translate( vec3_cref_t translate )
    {
        this->set_min( _min + translate );
        this->set_max( _max + translate );
        return *this;
    }

    inline this_t translated( vec3_cref_t translate ) const
    {
        return this_t( *this ).translate();
    }

    void translate( vec3_cref_t translate, this_ref_t box_out ) const
    {
        box_out.set_max( this_t::get_max() + translate );
        box_out.set_min( this_t::get_min() + translate );
    }

    this_ref_t translate_to_position( vec3_t const & position )
    {
        vec3_t center = center();
        this_t::set_min( this_t::get_min() - center + position );
        this_t::set_max( this_t::get_max() - center + position );
        return *this;
    }

  public:

    inline this_ref_t operator()( vec3_cref_t max, vec3_cref_t min )
    {
        _max = max;
        _min = min;
        return *this;
    }

    inline this_ref_t operator=( this_cref_t rhv )
    {
        return ( *this )( rhv.get_max(), rhv.get_min() );
    }

  public:

    inline vec3_cref_t get_max( void_t ) const
    {
        return _max;
    }
    inline vec3_cref_t get_min( void_t ) const
    {
        return _min;
    }
    inline vec3_ref_t get_max( void_t )
    {
        return _max;
    }
    inline vec3_ref_t get_min( void_t )
    {
        return _min;
    }

    inline void_t set_max( vec3_cref_t max )
    {
        _max = max;
    }
    inline void_t set_min( vec3_cref_t min )
    {
        _min = min;
    }

    inline vec3_t get_center( void_t ) const
    {
        return ( this_t::get_max() - this_t::get_min() ) * type_t( 0.5 ) + this_t::get_min();
    }

    vec3_t get_extend( void_t ) const
    {
        return ( ( this_t::get_max() - this_t::get_min() ) * type_t( 0.5 ) );
    }

    std::array< this_t::vec3_t, 8 > points( void_t ) const noexcept
    {
        return 
        { 
            this_t::vec3_t( _min.x(), _min.y(), _min.z() ),
            this_t::vec3_t( _min.x(), _max.y(), _min.z() ),
            this_t::vec3_t( _max.x(), _max.y(), _min.z() ),
            this_t::vec3_t( _max.x(), _min.y(), _min.z() ),

            this_t::vec3_t( _min.x(), _min.y(), _max.z() ),
            this_t::vec3_t( _min.x(), _max.y(), _max.z() ),
            this_t::vec3_t( _max.x(), _max.y(), _max.z() ),
            this_t::vec3_t( _max.x(), _min.y(), _max.z() )
        } ;
    }
};
} // namespace motor::collide::c3d

namespace motor::collide
{
    motor_typedefs( motor::collide::c3d::aabb< float_t >, aabb3f ) ;
}
