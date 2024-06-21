#pragma once

#include "ivisitor.h"

namespace motor
{
    namespace scene
    {
        /// implements the ivisitor interface with default
        /// behavior. The default behavior is to do nothing.
        class MOTOR_SCENE_API visitor : public ivisitor
        {
        public:

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result post_visit( motor::scene::node_ptr_t ) noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result visit( motor::scene::group_ptr_t )  noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result post_visit( motor::scene::group_ptr_t ) noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result visit( motor::scene::decorator_ptr_t ) noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result post_visit( motor::scene::decorator_ptr_t ) noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept
                { return motor::scene::ok ; }

            virtual motor::scene::result post_visit( motor::scene::leaf_ptr_t ) noexcept
                { return motor::scene::ok ; }

        };
    }
}