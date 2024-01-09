
#include "glx_context.h"

#include <motor/ogl/gl/gl.h>

using namespace motor::platform ;
using namespace motor::platform::glx ;

// this function needs to be in its own translation unit
// because corearb.h and glx.h can not be in the same becasue
// glx.h includes gl.h which interferes with corearb.h.
void_t context::init_gl_context( void_t ) noexcept 
{
    motor::ogl::gl::init() ;
}