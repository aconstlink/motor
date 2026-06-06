
#pragma once

#include "../icomponent.h"
#include "animation_controller.h"

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API animation_component : public icomponent
{
    motor_this_typedefs( animation_component );

  private:

    motor::vector< motor::scene::animation_controller_mtr_t > _controllers ;

  public:

    animation_component( void_t ) noexcept;
    animation_component( this_rref_t ) noexcept;
    animation_component( this_cref_t ) = delete;
    virtual ~animation_component( void_t ) noexcept;

  public:

    bool_t attach_controller( animation_controller_mtr_safe_t ) noexcept;
};
motor_typedef( animation_component );

} // namespace scene
} // namespace motor