#pragma once

#include "../imodule.h"
#include "../layouts/game_controller.hpp"

#include <motor/std/vector>

namespace motor
{
    namespace controls
    {
        // a system module that spawns virtual devices like game controllers.
        class MOTOR_CONTROLS_API vdev_module : public imodule
        {
            motor_this_typedefs( vdev_module ) ;
            
            struct data
            {
                bool_t xbox_added = false ;
                bool_t keyboard_added = false ;
                bool_t mouse_added = false ;

                motor::controls::game_device_mtr_t dev ;
            };
            motor::vector < data > _games ;

        public:

            vdev_module( void_t ) noexcept ;
            vdev_module( this_cref_t ) = delete ;
            vdev_module( this_rref_t ) noexcept ;
            virtual ~vdev_module( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            virtual void_t search( motor::controls::imodule::search_funk_t ) noexcept ;
            virtual void_t update( void_t ) noexcept ;
            virtual void_t release( void_t ) noexcept ;

        public:

            typedef std::function< void_t ( motor::controls::imapping_mtr_t ) > mapping_searach_ft ;
            void_t search( this_t::mapping_searach_ft ) noexcept ;

            void_t check_devices( motor::controls::imodule_mtr_t ) noexcept ;

        private:

            void_t init_controller_1( motor::controls::imodule_mtr_t ) noexcept ;
        } ;
        motor_typedef( vdev_module ) ;
    }
}