#pragma once

#include "../ivisitor.h"
#include "../../node/group.h"
#include "../../node/leaf.h"
#include "../../component/graphics/msl_set_component.hpp"

#include <motor/gfx/camera/generic_camera.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

namespace motor
{
namespace scene
{

// applies the set light for every geometry object in the scene.
class MOTOR_SCENE_API light_pass_render_visitor : public ivisitor
{
    motor_this_typedefs( light_pass_render_visitor );

  public:

    enum class light_type
    {
        point_light,
        directional_light
    };

    struct light
    {
        light_type lt ;
        
        // if point light, this is the lights' position
        // if directional light, this is the lights' direction
        motor::math::vec3f_t pos_dir ;

        // transforms in light projection space
        motor::math::mat4f_t proj ;
        // transforms into light view space
        motor::math::mat4f_t view ;

        // the shadow map pre-rendered.
        motor::string_t shadow_map ;
    };
    motor_typedef( light ) ;

  private:

    motor::scene::msl_set_component_t::id_t _msl_set_id =
        motor::scene::msl_set_component_t::invalid_id();

    motor::graphics::gen4::frontend_ptr_t _fe;

    motor::gfx::generic_camera_ptr_t _cam = nullptr;

    bool_t _light_dir_set = false;
    motor::math::vec3f_t _light_dir;

    light_t _light ;

  public:

    light_pass_render_visitor( motor::scene::msl_set_component_t::id_t const id,
        motor::graphics::gen4::frontend_ptr_t, motor::gfx::generic_camera_ptr_t cam,
        light_cref_t light ) noexcept;

    
    light_pass_render_visitor( this_rref_t ) noexcept;
    light_pass_render_visitor( this_cref_t ) = delete;
    virtual ~light_pass_render_visitor( void_t ) noexcept;

  public: // light interface

    bool_t is_light_dir_set( void_t ) const noexcept
    {
        return _light_dir_set;
    }

    motor::math::vec3f_cref_t get_light_dir( void_t ) const noexcept
    {
        return _light_dir;
    }

    void_t set_light_direction( motor::math::vec3f_in_t dir ) noexcept
    {
        _light_dir_set = true;
        _light_dir = dir;
    }

  public:

    virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept;
    virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept;
    virtual motor::scene::result post_visit(
        motor::scene::group_ptr_t, motor::scene::result const ) noexcept;

    virtual void_t on_start( void_t ) noexcept;
    virtual void_t on_finish( void_t ) noexcept;

  protected:

    motor::scene::msl_set_component_t::id_t msl_set_id( void_t ) const noexcept
    {
        return _msl_set_id;
    }

    motor::graphics::gen4::frontend_ptr_t borrow_frontend( void_t ) noexcept
    {
        return _fe;
    }

    motor::gfx::generic_camera_ptr_t borrow_camera( void_t ) noexcept
    {
        return _cam;
    }

  private:

    void_t handle_visit( motor::scene::node_ptr_t ) noexcept;
    void_t handle_post_visit( motor::scene::node_ptr_t ) noexcept;
};
motor_typedef( light_pass_render_visitor );
} // namespace scene
} // namespace motor