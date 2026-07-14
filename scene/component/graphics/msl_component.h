
#pragma once

#include "../icomponent.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/variable/wire_variable_bridge.h>
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

    motor::graphics::command_status_mtr_t _status = nullptr ;
    motor::graphics::msl_object_mtr_t _msl = nullptr;
    motor::graphics::variable_set_mtr_t _var_set = nullptr;

  private: // camera variables

    struct camera_variables
    {
        motor::graphics::data_variable< motor::math::mat4f_t > * proj;
        motor::graphics::data_variable< motor::math::mat4f_t > * cam;
        motor::graphics::data_variable< motor::math::mat4f_t > * view;
        motor::graphics::data_variable< motor::math::vec3f_t > * cam_pos;

        void_t clear( void_t ) noexcept
        {
            proj = nullptr;
            cam = nullptr;
            view = nullptr;
            cam_pos = nullptr;
        }
    };

    camera_variables _cam_vars;

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

    motor::graphics::wire_variable_bridge_t _brigde;

  public:

    msl_component( this_rref_t ) noexcept;
    msl_component( this_cref_t ) = delete;
    msl_component( motor::graphics::msl_object_mtr_safe_t ) noexcept;
    msl_component( motor::graphics::msl_object_mtr_safe_t, vs_idx_t const,
        geo_idx_t const = geo_idx_t( -1 ) ) noexcept;

    virtual ~msl_component( void_t ) noexcept;

  public:

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

    // allows to clone without shader variables. Those are determined by
    // the render_update function AFTER the msl is compiled.
    // Mainly used to populate a msl_set_component if only a msl_component
    // is present in the node.
    this_t light_clone( motor::string_in_t name ) const noexcept;

  public: // render interface

    bool_t render_init( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    bool_t render_release( motor::graphics::gen4::frontend_ptr_t ) noexcept;
    void_t render_update( motor::gfx::generic_camera_ptr_t ) noexcept;

  public: // transformation interface

    void_t set_world( motor::math::m3d::trafof_cref_t ) noexcept;

  public: // inputs

    motor::wire::inputs_cptr_t borrow_shader_inputs( void_t ) const noexcept;
    motor::wire::inputs_ptr_t borrow_shader_inputs( void_t ) noexcept;

  private:

    void_t update_bindings( void_t ) noexcept;
    void_t update_camera( motor::gfx::generic_camera_ptr_t ) noexcept;
    void_t prefill_bridge( void_t ) noexcept;
};
motor_typedef( msl_component );
} // namespace scene
} // namespace motor