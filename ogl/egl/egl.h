#pragma once 

#include "../result.h"
#include "../typedefs.h"
#include "../api.h"

#include <motor/std/vector>
#include <motor/std/string>

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

            typedef motor::vector< motor::string_t > strings_t ;
            strings_t _egl_extensions ;

        public: 

            /// will init extensions
            motor::ogl::result init( EGLNativeDisplayType ) ;


            bool_t is_supported( char_cptr_t name ) ;
        };
    }
}
