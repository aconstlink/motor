
#pragma once

#include "../typedefs.h"

#include "../iobserver.hpp"
#include "midi_message.hpp"
#include <motor/log/global.h>

namespace motor
{
    namespace controls
    {
        class midi_observer : public iobserver
        {
            motor_this_typedefs( midi_observer ) ;

        public:

            virtual ~midi_observer( void_t ) noexcept{}
        
            virtual void_t on_message( motor::string_cref_t dname, motor::controls::midi_message_cref_t msg ) noexcept
            {
                motor::log::global::status( "[" + dname + "] : " +
                    motor::to_string( msg.status ) + "; " +
                    motor::to_string( msg.byte1 ) + "; " +
                    motor::to_string( msg.byte2 ) + "; " +
                    motor::to_string( msg.byte3 ) ) ;
            }
        };
        motor_typedef( midi_observer ) ;
    }

}
