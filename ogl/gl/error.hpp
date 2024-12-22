#pragma once

#include "gl.h"
#include "convert.hpp"

#include <motor/std/string>
#include <motor/log/global.h>

namespace motor
{
    namespace ogl
    {
        struct error
        {
            static bool_t check_and_log( motor::string_in_t msg ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if( __so__err == GL_NO_ERROR ) return false ;

                motor::string_t const __glstring = motor::ogl::to_string( __so__err ) ;
                motor::log::global_t::error( msg + " " + __glstring ) ;
                return true ;
            }

            static bool_t check_and_log( char const * msg ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if ( __so__err == GL_NO_ERROR ) return false ;

                motor::string_t const __glstring = motor::ogl::to_string( __so__err ) ;
                motor::log::global_t::error( motor::string_t( msg ) + " " + __glstring ) ;
                return true ;
            }
        };
    }
}