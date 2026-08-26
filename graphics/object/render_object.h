
#pragma once

#include "../object.h"

#include "../object/shader_object.h"
#include "../buffer/vertex_attribute.h"
#include "../variable/variable_set.hpp"
#include "../state/state_set.h"

#include <motor/std/vector>

#include <algorithm>

namespace motor
{
namespace graphics
{
class MOTOR_GRAPHICS_API render_object : public object
{
    motor_this_typedefs( render_object );

  private:

    motor::string_t _name;

    motor::string_t _shader;

  public:

    struct geometry_link
    {
        // change happended
        size_t hash;
        // number of links
        size_t ref_count;
        // name of geometry
        motor::string_t name;
    };
    motor_typedef( geometry_link );

    struct variable_set
    {
        size_t hash;
        motor::graphics::variable_set_mtr_t vs;
    };
    motor_typedef( variable_set );

    struct safe_variable_set
    {
        size_t hash;
        motor::graphics::variable_set_mtr_safe_t vs;

        safe_variable_set( size_t const h, motor::graphics::variable_set_mtr_safe_t vs_ )
            : hash( h ), vs( motor::move( vs_ ) )
        {
        }

        safe_variable_set( safe_variable_set const & rhv ) noexcept
            : hash( rhv.hash ), vs( motor::share( rhv.vs ) )
        {
        }
        safe_variable_set( safe_variable_set && rhv ) noexcept
            : hash( rhv.hash ), vs( motor::move( rhv.vs ) )
        {
        }
        ~safe_variable_set( void_t ) noexcept
        {
            motor::release( motor::move( vs ) );
        }
    };
    motor_typedef( safe_variable_set );

  private:

    motor::vector< geometry_link_t > _geo;
    motor::vector< motor::string_t > _soo;

  private: // variable sets

    motor::vector< variable_set_t > _vars;

  private: // states

    motor::vector< motor::graphics::render_state_sets_t > _states;

  public:

    render_object( void_t ) noexcept;
    render_object( motor::string_cref_t name ) noexcept;

    ~render_object( void_t ) noexcept;

    render_object( this_cref_t rhv ) noexcept;

    render_object( this_rref_t rhv ) noexcept;

    this_ref_t operator=( this_cref_t rhv ) noexcept;

    this_ref_t operator=( this_rref_t rhv ) noexcept;

  public:

    using for_each_geo_link_funk_t =
        std::function< void_t( size_t const idx, this_t::geometry_link_cref_t ) >;
    void_t for_each_geometry_link( for_each_geo_link_funk_t funk ) const noexcept;

    size_t link_geometry( motor::string_cref_t name ) noexcept;
    bool_t unlink_geometry( motor::string_cref_t name ) noexcept;
    bool_t unlink_geometry( size_t const geo_idx ) noexcept;

    this_ref_t link_geometry( motor::vector< motor::string_t > const & names ) noexcept;

    // link to stream out object so geometry can be fed from there.
    // the geometry is then mainly used for geometry layout.
    this_ref_t link_geometry( motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept;

    size_t get_num_geometry( void_t ) const noexcept;
    size_t get_num_streamout( void_t ) const noexcept;

    this_t::geometry_link_cref_t get_geometry_link( size_t const i = 0 ) const noexcept;

    motor::vector< motor::string_t > const & get_streamouts( void_t ) const noexcept;

    motor::string_cref_t get_streamout( size_t const i = 0 ) const noexcept;

    bool_t has_streamout_link( void_t ) const noexcept;

    this_ref_t link_shader( motor::string_cref_t name ) noexcept;

    motor::string_cref_t get_shader( void_t ) const noexcept;

  public: // variable sets

    size_t add_variable_set( motor::graphics::variable_set_mtr_safe_t vs ) noexcept;
    void_t drop_variable_set( size_t const ) noexcept;

    this_ref_t add_variable_sets(
        motor::vector< motor::graphics::variable_set_mtr_safe_t > && vss ) noexcept;

    this_ref_t remove_variable_sets( void_t ) noexcept;

    typedef std::function< void_t( size_t const i, this_t::variable_set_cref_t ) >
        for_each_var_funk_t;

    void_t for_each( for_each_var_funk_t funk ) noexcept;

    size_t get_num_variable_sets( void_t ) const noexcept;

    motor::graphics::variable_set_mtr_safe_t get_variable_set( size_t const i ) noexcept;
    safe_variable_set_t get_safe_variable_set( size_t const i ) noexcept;

    motor::vector< motor::graphics::variable_set_mtr_safe_t > get_varibale_sets(
        void_t ) const noexcept;

    motor::vector< render_object::variable_set_t > & borrow_varibale_sets( void_t ) noexcept;

    motor::vector< render_object::variable_set_t > const & borrow_varibale_sets(
        void_t ) const noexcept;

    // fast version for quick access without ref counting
    render_object::variable_set_t borrow_variable_set( size_t const i ) const noexcept;

    this_ref_t fill_variable_sets( size_t const idx ) noexcept;

  public: // render state sets

    this_ref_t add_render_state_set( motor::graphics::render_state_sets_cref_t rs ) noexcept;

    typedef std::function< void_t( size_t const i, motor::graphics::render_state_sets_cref_t ) >
        for_each_render_state_funk_t;

    void_t for_each( for_each_render_state_funk_t funk ) noexcept;

  public:

    motor::string_cref_t name( void_t ) const noexcept;
};
motor_typedef( render_object );
} // namespace graphics
} // namespace motor
