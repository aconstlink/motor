#pragma once

#include "typedefs.h"

#include <motor/std/string>
#include <future>

namespace motor
{
    namespace format
    {
        struct item
        {
            virtual ~item( void_t ) {}
        };
        motor_typedef( item ) ;
        typedef std::future< item_mtr_t > future_item_t ;

        struct status_item : public item
        {
            motor::string_t msg ;

            status_item( motor::string_cref_t msg_ ) : msg( msg_ ) {}
        };
        motor_typedef( status_item ) ;
        typedef std::future< status_item_mtr_t > future_status_t ;
    }
}