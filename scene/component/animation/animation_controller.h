#pragma once

#include "../icontroller.h"
#include <motor/wire/node/node.h>

namespace motor
{
namespace scene
{

// can do any animation
class MOTOR_SCENE_API animation_controller : public single_controller
{
    motor_this_typedefs( animation_controller );

  public:

    animation_controller( void_t ) noexcept {}
    animation_controller( this_cref_t ) = delete;
    animation_controller( this_rref_t rhv ) noexcept : single_controller( std::move( rhv ) ) {}
    virtual ~animation_controller( void_t ) noexcept {}
};
motor_typedef( animation_controller );

} // namespace scene
} // namespace motor