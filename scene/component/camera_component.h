
#pragma once

#include "icomponent.h"

#include <motor/gfx/camera/generic_camera.h>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API camera_component : public icomponent
{
    motor_this_typedefs( camera_component );

  private:

    motor::gfx::generic_camera_mtr_t _cam = nullptr;

  public:

    camera_component( void_t ) noexcept;
    camera_component( motor::gfx::generic_camera_mtr_safe_t ) noexcept;
    camera_component( this_rref_t ) noexcept;
    camera_component( this_cref_t ) = delete;
    virtual ~camera_component( void_t ) noexcept;

  public: // camera

    motor::gfx::generic_camera_mtr_t borrow_camera( void_t ) const noexcept
    {
        return _cam;
    }

    motor::gfx::generic_camera_mtr_safe_t get_camera( void_t ) const noexcept
    {
        return motor::share( _cam );
    }

    void_t exchange_camera( motor::gfx::generic_camera_mtr_safe_t cam ) noexcept
    {
        motor::release( motor::move( _cam ) );
        _cam = motor::move( cam );
    }
};
motor_typedef( camera_component );

} // namespace scene
} // namespace motor