
#pragma once

#include "ivisitor.h"

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API default_visitor : public motor::scene::ivisitor
{
    motor_this_typedefs( default_visitor );

  public:

    virtual ~default_visitor( void_t ) noexcept ;

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept = 0 ;
    virtual motor::scene::result post_visit( motor::scene::node_ptr_t nptr, motor::scene::result const res ) noexcept = 0 ;

  private:

    virtual motor::scene::result visit( motor::scene::group_ptr_t nptr ) noexcept final;

    virtual motor::scene::result post_visit( motor::scene::group_ptr_t nptr,
                                             motor::scene::result const res ) noexcept final;

    virtual motor::scene::result visit( motor::scene::leaf_ptr_t nptr ) noexcept final;

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
} // namespace scene
} // namespace motor