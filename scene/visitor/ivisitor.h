#pragma once

#include "../result.h"
#include "../protos.h"
#include "../typedefs.h"
#include "../api.h"

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API ivisitor
{
  public:

    virtual ~ivisitor( void_t ) noexcept {}    

    virtual motor::scene::result visit( motor::scene::group_ptr_t nptr ) noexcept 
    {
        return motor::scene::result::ok ;
    }

    virtual motor::scene::result post_visit( motor::scene::group_ptr_t nptr, motor::scene::result const res ) noexcept
    {
        return motor::scene::result::ok ;
    }

    virtual motor::scene::result visit( motor::scene::leaf_ptr_t nptr ) noexcept 
    {
        return motor::scene::result::ok ;
    }

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
} // namespace scene
} // namespace motor