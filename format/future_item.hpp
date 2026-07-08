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
            virtual void_t release( void_t ) noexcept = 0 ;
        };
        motor_typedef( item ) ;
        typedef std::future< item_mtr_t > future_item_t ;

        struct status_item : public item
        {
            motor::string_t msg ;

            status_item( motor::string_cref_t msg_ ) : msg( msg_ ) {}

            virtual void_t release( void_t ) noexcept {}
        };
        motor_typedef( status_item ) ;
        typedef std::future< status_item_mtr_t > future_status_t ;
    }
}