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
        public: // group / decorator

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result post_visit( motor::scene::node_ptr_t ) noexcept = 0 ;

            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t ) noexcept  = 0 ;

            virtual motor::scene::result visit( motor::scene::transform_3d_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result post_visit( motor::scene::transform_3d_ptr_t ) noexcept = 0 ;

            virtual motor::scene::result visit( motor::scene::render_state_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result post_visit( motor::scene::render_state_ptr_t ) noexcept = 0 ;

            virtual motor::scene::result visit( motor::scene::decorator_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result post_visit( motor::scene::decorator_ptr_t ) noexcept = 0 ;

        public: // leaf

            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result visit( motor::scene::renderable_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result visit( motor::scene::camera_ptr_t ) noexcept  = 0 ;

        };
    }
}