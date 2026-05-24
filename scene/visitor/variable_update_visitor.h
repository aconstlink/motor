
#pragma once

#include "default_visitor.h"

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API variable_update_visitor : public motor::scene::default_visitor
{
    motor_this_typedefs( variable_update_visitor );

  public:

    variable_update_visitor( void_t ) noexcept;
    virtual ~variable_update_visitor( void_t ) noexcept;

  private:

    virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept override;
    virtual motor::scene::result post_visit( motor::scene::node_ptr_t nptr, motor::scene::result const res ) noexcept override;

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
motor_typedef( variable_update_visitor );
} // namespace scene
} // namespace motor