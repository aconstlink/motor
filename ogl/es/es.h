#pragma once

#include "../result.h"
#include "../api.h"
#include "../typedefs.h"

#if defined( NATUS_TARGET_OS_WIN )
#elif defined( NATUS_TARGET_OS_LIN )
#else
#endif

#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <natus/ntd/list.hpp>
#include "convert.hpp"
namespace natus
{
    namespace ogl
    {
        struct NATUS_OGL_API es
        {
            natus_this_typedefs( es ) ;

        public:

            typedef natus::ntd::list< natus::ntd::string > string_list_t ;
            static string_list_t _extensions ;

            static bool_t _init ;

        public:

            static natus::ogl::result init( void_t ) ;

            static bool_t is_supported( char const* name ) ;

        public:

            static bool_t check_and_log( natus::ntd::string_in_t msg ) noexcept
            {
                GLenum __so__err = glGetError() ;
                if( __so__err == GL_NO_ERROR ) return false ;

                std::string const __glstring = natus::ogl::to_string( __so__err ) ;
                natus::log::global_t::error( msg + " " + __glstring ) ;
                return true ;
            }
        };
    }
}
