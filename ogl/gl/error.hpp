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
            // returns true on error!
            static bool_t check_and_log( char const * msg ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if ( __so__err == GL_NO_ERROR ) return false ;

                char const * __glstring = motor::ogl::to_string( __so__err ) ;
                motor::log::global::error<2048>( "%s [%s]", msg, __glstring ) ;
                return true ;
            }

            // returns true on error!
            static bool_t check_and_log( char const * msg, motor::string_in_t name ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if ( __so__err == GL_NO_ERROR ) return false ;

                char const * __glstring = motor::ogl::to_string( __so__err ) ;
                motor::log::global::error<2048>( "%s : %s : [%s]", msg, name.c_str(), __glstring ) ;
                return true ;
            }

            // returns true on error!
            template< typename T >
            static bool_t check_and_log( char const * msg, typename std::remove_reference<T>::type const & obj ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if ( __so__err == GL_NO_ERROR ) return false ;

                char const * __glstring = motor::ogl::to_string( __so__err ) ;
                motor::log::global::error<2048>( "%s : %s : [%s]", msg, obj.name().c_str(), __glstring ) ;
                return true ;
            }
        };
    }
}