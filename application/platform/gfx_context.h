#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../result.h"

#include <motor/graphics/backend/backend.h>

namespace motor
{
    namespace application
    {
        class MOTOR_APPLICATION_API gfx_context
        {
        public:

            virtual ~gfx_context( void_t ) {}

        public:

            virtual motor::application::result activate( void_t ) = 0 ;
            virtual motor::application::result deactivate( void_t ) = 0 ;
            virtual motor::application::result vsync( bool_t const on_off ) = 0 ;
            virtual motor::application::result swap( void_t ) = 0 ;

        public:

            virtual motor::graphics::backend_mtr_unique_t create_backend( void_t ) noexcept = 0 ;
        };
        motor_typedef( gfx_context ) ;
    }
}