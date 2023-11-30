#pragma once 

#include "../result.h"
#include "../typedefs.h"
#include "../api.h"

#include <natus/ntd/vector.hpp>

//#include <GL/glcorearb.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace motor 
{
    namespace ogl
    {
        struct MOTOR_OGL_API egl
        {
            motor_this_typedefs( egl ) ;

        private:

            typedef motor::vector< motor::string > strings_t ;
            static strings_t _egl_extensions ;

        public: 

            /// will init extensions
            static motor::ogl::result init( EGLNativeDisplayType ) ;


            static bool_t is_supported( char_cptr_t name ) ;
        };
    }
}
