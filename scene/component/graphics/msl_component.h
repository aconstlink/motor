
#pragma once

#include "../icomponent.h"
#include "render_data_set.hpp"

#include <motor/graphics/variable/wire_variable_output_bridge.hpp>
#include <motor/graphics/object/msl_object.h>

#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/gfx/camera/generic_camera.h>

#include <motor/wire/slot/sheet.hpp>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API msl_component : public icomponent
{
    motor_this_typedefs( msl_component );

  public:

    using geo_idx_t = size_t;
    using vs_idx_t = size_t;

  private:

    motor::graphics::compilation_listener_mtr_t _comp_lst =
        motor::shared( motor::graphics::compilation_listener(), "render_node comp listener" );

    vs_idx_t _vs = 0;
    geo_idx_t _geo_id = 0;

    motor::graphics::msl_object_mtr_t _msl = nullptr;

#if 0
    motor::graphics::variable_set_mtr_t _var_set = nullptr;
#endif

    motor::scene::render_data_set_t _base_data_set;

    // allows temporary variable assignment. for example, a light render visitor
    // requires a variable set for each light rendered, or each camera view rendered.
    // so the shader has a puropse of being a pass of a material. but that material
    // could be hit by mutliple lights for eample. So for each light hit, we need to
    // store the variable data for a frame in order to render it properly. Otherwise
    // we would override the variable set data.
    motor::vector< motor::scene::render_data_set_t > _sub_sets;

  private: // trafo variables

    struct trafo_variables
    {
        motor::wire::output_slot< motor::math::mat4f_t > * world =
            motor::shared( motor::wire::output_slot< motor::math::mat4f_t >() );

        trafo_variables( void_t ) noexcept {}
        trafo_variables( trafo_variables && rhv ) noexcept
        {
            motor::release( motor::move( world ) );
            world = motor::move( rhv.world );
        }
        ~trafo_variables( void_t ) noexcept
        {
            trafo_variables::clear();
        }
        void_t clear( void_t ) noexcept
        {
            motor::release( motor::move( world ) );
        }
    };

    trafo_variables _trafo_vars;

  private:

    // this output bridge is designed to connect only to 
    // subset render data input bridges.
    // this is a current workaround because we do not have 
    // variable set views. So we have to use full variable sets
    // which required a full copy of the data.
    motor::graphics::wire_variable_output_bridge_t _out_bridge;

  public:

    msl_component( this_rref_t ) noexcept;
    msl_component( this_cref_t ) = delete;
    msl_component( motor::graphics::msl_object_mtr_safe_t ) noexcept;

    // create a non-managed msl_component. This means, the msl object is
    // managed by this component.
    msl_component( motor::graphics::msl_object_mtr_safe_t, vs_idx_t const,
        geo_idx_t const = geo_idx_t( -1 ) ) noexcept;

    virtual ~msl_component( void_t ) noexcept;

  public:

    bool_t is_managed( void_t ) const noexcept
    {
        return _msl->is_managed();
    }

    // size_t set_msl( motor::graphics::msl_object_mtr_safe_t ) noexcept;
    size_t set_msl( motor::graphics::msl_object_mtr_safe_t ) noexcept;

    motor::graphics::msl_object_mtr_t borrow_msl( void_t ) noexcept
    {
        return _msl;
    }
    motor::graphics::msl_object_mtr_t get_msl( void_t ) noexcept
    {
        return motor::share( _msl );
    }
    vs_idx_t get_variable_set_idx( void_t ) const noexcept
    {
        return _vs;
    }

    geo_idx_t get_geo_idx( void_t ) const noexcept
    {
        return _geo_id;
    }

  public: // render interface

    bool_t render_init( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    bool_t render_release( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    void_t render_update( motor::gfx::generic_camera_ptr_t ) noexcept;
    size_t render_update( size_t const render_id, motor::gfx::generic_camera_ptr_t ) noexcept;

  public: // transformation interface

    void_t set_world( motor::math::m3d::trafof_cref_t ) noexcept;

  public: // light interface

    // set a light direction on the shader variable
    // if there is a bindings.
    void_t set_light_direction( motor::math::vec3f_cref_t ) noexcept;
    void_t set_light_direction( size_t const render_id, motor::math::vec3f_cref_t dir ) noexcept ;

    void_t set_light_projection( motor::math::mat4f_cref_t ) noexcept;
    void_t set_light_view( motor::math::mat4f_cref_t ) noexcept;
    void_t set_light_shadow_map( motor::string_cref_t ) noexcept;

  public: // inputs

    motor::wire::inputs_cref_t borrow_shader_inputs( void_t ) const noexcept;
    motor::wire::inputs_ref_t borrow_shader_inputs( void_t ) noexcept;

  private:

    void_t update_bindings( void_t ) noexcept;
    void_t update_bindings( size_t const render_id ) noexcept;
    void_t update_camera( motor::gfx::generic_camera_ptr_t ) noexcept;
    void_t update_camera( size_t const render_id, motor::gfx::generic_camera_ptr_t ) noexcept;

    bool_t ensure_render_data( size_t const id ) noexcept;
    void_t ensure_render_data( size_t const id, motor::graphics::shader_bindings_cref_t ) noexcept;

    // connect the output bridge to the input bridge slots of
    // render_data_set with id.
    void_t connect_output_to_input( size_t const id );
};
motor_typedef( msl_component );
} // namespace scene
} // namespace motor