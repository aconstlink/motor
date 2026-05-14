#pragma once

#include "ivisitor.h"
#include "../node/group.h"
#include "../node/leaf.h"

#include <motor/gfx/camera/generic_camera.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API render_visitor : public ivisitor
{
    motor_this_typedefs( render_visitor );

    motor::graphics::gen4::frontend_ptr_t _fe;

    motor::gfx::generic_camera_ptr_t _cam = nullptr;

  public:

    render_visitor( motor::graphics::gen4::frontend_ptr_t, motor::gfx::generic_camera_ptr_t cam ) noexcept;
    render_visitor( this_rref_t ) noexcept;
    render_visitor( this_cref_t ) = delete;
    virtual ~render_visitor( void_t ) noexcept;

  public:
    
    virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept; 
    virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept;
    virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept;

    virtual void_t on_start( void_t ) noexcept;
    virtual void_t on_finish( void_t ) noexcept;

  protected:

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
motor_typedef( render_visitor );
} // namespace scene
} // namespace motor