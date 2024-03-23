
#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/controls/imodule.h>
#include <motor/controls/types/xbox_controller.hpp>

#include <motor/std/vector>
#include <motor/math/vector/vector2.hpp>

#include <windows.h>

namespace motor
{
    namespace platform
    {
        namespace win32
        {
            class MOTOR_PLATFORM_API xinput_module : public motor::controls::imodule
            {
                motor_this_typedefs( xinput_module ) ;

                /// helper device for state tracking
                class xinput_device ;
                
                struct gamepad_data
                {
                    bool_t connected = false ;
                    this_t::xinput_device * xinput_ptr = nullptr ;
                    motor::controls::xbc_device_mtr_t dev ;
                };
                motor_typedef( gamepad_data ) ;
                motor_typedefs( motor::vector<gamepad_data_t>, devices ) ;

            private:

                devices_t _devices ;

            public:

                xinput_module( void_t ) noexcept ;
                xinput_module( this_cref_t ) = delete ;
                xinput_module( this_rref_t ) noexcept;
                virtual ~xinput_module( void_t ) noexcept ;

                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual void_t search( motor::controls::imodule::search_funk_t ) noexcept ;
                virtual void_t update( void_t ) noexcept ;
                virtual void_t release( void_t ) noexcept ;

            private:

                void_t check_gamepads( void_t ) noexcept ;
            };
            motor_typedef( xinput_module ) ;
        }
    }
}