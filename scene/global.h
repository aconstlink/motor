
#pragma once

#include "api.h"
#include "typedefs.h"
#include "double_dispatch.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API global
        {
            motor_this_typedefs( global ) ;

        private:

            static motor::scene::double_dispatcher_t _dd ;

        public:
            
            template< class caller_t, class callee_t >
            static void_t register_callbacks( motor::scene::double_dispatch_callbacks && fs ) noexcept
            {
                _dd.register_funk_set< caller_t, callee_t > ( std::move( fs ) ) noexcept ;
            }

            // by default, the visitor uses visit() and post_visit(), so
            // those functions will be called for caller_t 
            template< class caller_t, class callee_t >
            static void_t register_default_callbacks( void_t ) noexcept
            {
                _dd.register_funk_set< caller_t, callee_t > ( 
                    motor::scene::double_dispatch_callbacks::make_default< caller_t, callee_t >() ) ;
            }

            static motor::scene::double_dispatch_callbacks resolve( motor::scene::ivisitor_ptr_t, motor::scene::ivisitable_ptr_t ) noexcept ;

            static void_t init( void_t ) noexcept ;
            static void_t deinit( void_t ) noexcept ;

        private:


        };

    }
}