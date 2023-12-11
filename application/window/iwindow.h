

#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API iwindow
        {
        public:

            // check for user messages and pass further to window implementation.
            virtual void_t check_for_messages( void_t ) noexcept = 0 ;
        } ;
    }
}