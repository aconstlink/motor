#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

namespace motor
{
namespace gfx
{

enum class light_type
{
    invalid,
    directional,
    point,
    spot,
    area
};

struct ishadow_data
{
    virtual ~ishadow_data( void_t ) noexcept {}
};
motor_typedef( ishadow_data );

struct single_shadow_data : public ishadow_data
{
    virtual ~single_shadow_data( void_t ) noexcept {}

    single_shadow_data(
        motor::math::mat4f_cref_t p, motor::math::mat4f_cref_t v, motor::string_cref_t sm ) noexcept
        : proj( p ), view( v ), shadow_map( sm )
    {
    }

    // transforms in light projection space
    motor::math::mat4f_t proj;
    // transforms into light view space
    motor::math::mat4f_t view;

    // the shadow map pre-rendered.
    motor::string_t shadow_map;
};
motor_typedef( single_shadow_data );

class light
{
    motor_this_typedefs( light );

  private:

    motor::math::vec4f_t _color;
    float_t _intensity;

    motor::gfx::ishadow_data_mtr_t _shadow_data = nullptr;

  public:

    light( void_t ) noexcept {}
    virtual ~light( void_t ) noexcept
    {
        motor::release( motor::move( _shadow_data ) );
    }

    virtual light_type get_light_type( void_t ) const noexcept = 0;

    template < typename T >
    T * borrow_shader_data( void_t ) noexcept
    {
        return dynamic_cast< T * >( _shadow_data );
    }
};
motor_typedef( light );

//****************************************************
class directional_light : public light
{
    motor_this_typedefs( directional_light );

  private:

    motor::math::vec3f_t _direction = motor::math::vec3f_t( 0.0f, 0.0f, 1.0f );

  public:

    directional_light( void_t ) noexcept {}
    directional_light( motor::math::vec3f_in_t dir ) noexcept : _direction( dir ) {}
    virtual ~directional_light( void_t ) noexcept {}

    static light_type get_light_type_static( void_t ) noexcept
    {
        return light_type::directional;
    }

    virtual light_type get_light_type( void_t ) const noexcept
    {
        return this_t::get_light_type_static();
    }

    motor::math::vec3f_cref_t get_direction( void_t ) const noexcept
    {
        return _direction;
    }

    void_t set_direction( motor::math::vec3f_in_t dir ) noexcept
    {
        _direction = dir;
    }
};
motor_typedef( directional_light );

//****************************************************
class point_light : public light
{
    motor_this_typedefs( point_light );

  private:

    motor::math::vec3f_t _position;
    float_t _radius = 1.0f;

  public:

    point_light( void_t ) noexcept {}
    point_light( motor::math::vec3f_in_t pos, float_t const radius ) noexcept
        : _position( pos ), _radius( radius )
    {
    }

    virtual ~point_light( void_t ) noexcept {}

    static light_type get_light_type_static( void_t ) noexcept
    {
        return light_type::point;
    }

    virtual light_type get_light_type( void_t ) const noexcept
    {
        return this_t::get_light_type_static();
    }

    motor::math::vec3f_cref_t get_position( void_t ) const noexcept
    {
        return _position;
    }

    float_t get_radius( void_t ) const noexcept
    {
        return _radius;
    }

    void_t set_radius( float_t const r ) noexcept
    {
        _radius = r;
    }

    void_t set_position( motor::math::vec3f_in_t position ) noexcept
    {
        _position = position;
    }
};
motor_typedef( point_light );

//****************************************************
class spot_light : public light
{
    motor_this_typedefs( spot_light );

  private:

    motor::math::vec3f_t _position;
    motor::math::vec3f_t _direction;

  public:

    spot_light( void_t ) noexcept {}
    virtual ~spot_light( void_t ) noexcept {}

    static light_type get_light_type_static( void_t ) noexcept
    {
        return light_type::spot;
    }

    virtual light_type get_light_type( void_t ) const noexcept
    {
        return this_t::get_light_type_static();
        ;
    }

    motor::math::vec3f_cref_t get_position( void_t ) const noexcept
    {
        return _position;
    }
};
motor_typedef( spot_light );

} // namespace gfx
} // namespace motor