#pragma once

#include "../result.h"
#include "../protos.h"
#include "../typedefs.h"
#include "../api.h"

#include <motor/core/double_dispatch.hpp>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API ivisitor
        {
            motor_core_dd_id_vfn() ;

        public: 

            virtual ~ivisitor( void_t ) noexcept {}

            virtual motor::scene::result visit( motor::scene::ivisitable_ptr_t ) noexcept 
                { return motor::scene::result::not_implemented ; }

            virtual motor::scene::result post_visit( motor::scene::ivisitable_ptr_t, motor::scene::result const ) noexcept  
                { return motor::scene::result::not_implemented ; }

            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept
            {
                return motor::scene::result::not_implemented ;
            }

            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept
            {
                return motor::scene::result::not_implemented ;
            }

            virtual void_t on_start( void_t ) noexcept {}
            virtual void_t on_finish( void_t ) noexcept {}
        };
    }
}