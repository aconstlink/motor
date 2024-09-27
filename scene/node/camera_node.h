#pragma once

#include "leaf.h"
#include <motor/gfx/camera/generic_camera.h>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API camera_node : public leaf
        {
            typedef leaf base_t ;
            motor_this_typedefs( camera_node ) ;

            motor_core_dd_id_fn() ;

        private:

            motor::gfx::generic_camera_mtr_t _cam = nullptr ;

        public:

            camera_node( void_t ) noexcept ;
            camera_node( motor::gfx::generic_camera_mtr_safe_t ) noexcept ;
            camera_node( this_rref_t ) noexcept ;
            camera_node( this_cref_t ) = delete ;
            virtual ~camera_node( void_t ) noexcept ;

        public: // camera

            motor::gfx::generic_camera_mtr_t borrow_camera( void_t ) const noexcept 
            {
                return _cam ;
            }

            motor::gfx::generic_camera_mtr_safe_t get_camera( void_t ) const noexcept
            {
                return motor::share( _cam ) ;
            }

            void_t exchange_camera( motor::gfx::generic_camera_mtr_safe_t cam ) noexcept
            {
                motor::release( motor::move( _cam ) ) ;
                _cam = motor::move( cam ) ;
            }

        public: // interface

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;
        };
        motor_typedef( camera_node ) ;
    }
}