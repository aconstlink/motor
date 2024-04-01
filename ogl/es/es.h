#pragma once

#include "../result.h"
#include "../api.h"
#include "../typedefs.h"

#if defined( MOTOR_TARGET_OS_WIN )
#elif defined( MOTOR_TARGET_OS_LIN )
#else
#endif

#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <motor/std/list>
#include "convert.hpp"
namespace motor
{
    namespace ogl
    {
        struct MOTOR_OGL_API es
        {
            motor_this_typedefs( es ) ;

        public:

            typedef motor::list< motor::string_t > string_list_t ;
            static string_list_t _extensions ;

            static bool_t _init ;

        public:

            static motor::ogl::result init( void_t ) ;

            static bool_t is_supported( char const* name ) ;

        public:

            static bool_t check_and_log( motor::string_in_t msg ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if( __so__err == GL_NO_ERROR ) return false ;

                motor::string_t const __glstring = motor::ogl::to_string( __so__err ) ;
                motor::log::global_t::error( msg + " " + __glstring ) ;
                return true ;
            }
        };
    }
}
