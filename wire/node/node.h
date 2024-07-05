
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "../slot/input_slot.h"
#include "../slot/output_slot.h"

#include <motor/std/string>

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API inode
        {
            motor_this_typedefs( inode ) ;

        public:

            virtual ~inode( void_t ) noexcept ;

        public:

            virtual void_t update( void_t ) noexcept = 0 ;
        };

        class node
        {

        private:

            void_t add( motor::string_in_t name, motor::wire::ioutput_slot_mtr_safe_t ) ;
            void_t add( motor::string_in_t name, motor::wire::iinput_slot_mtr_safe_t ) ;

            bool_t connect_to_slot( motor::string_in_t name, motor::wire::ioutput_slot_mtr_safe_t ) ;
            bool_t connect_to_signal( motor::string_in_t name, motor::wire::iinput_slot_mtr_safe_t ) ;
        };
    }
}