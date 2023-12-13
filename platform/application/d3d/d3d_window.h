#pragma once

#include "d3d_context.h"
#include "../win32/win32_window.h"

#include "../platform_window.h"

namespace motor
{
    namespace application
    {
        namespace d3d
        {
            class NATUS_APPLICATION_API window : public platform_window
            {
                motor_this_typedefs( window ) ;

            private:

                context_res_t _context ;
                win32::window_res_t _window ;

                bool_t _vsync = true ;

            public:

                window( void_t ) ;
                window( d3d_info_cref_t gli, window_info_cref_t wi ) ;
                window( this_rref_t ) ;
                virtual ~window( void_t ) ;

            public:

                context_res_t get_context( void_t ) const { return _context ; }
                win32::window_res_t get_window( void_t ) { return _window ; }

                virtual void_t check_for_messages( void_t ) noexcept ;
            };
            motor_res_typedef( window ) ;
        }
    }
}