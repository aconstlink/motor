
#include "glx_context.h"

#include <motor/ogl/gl/gl.h>

using namespace motor::platform ;
using namespace motor::platform::glx ;

void_t context::init_gl_context( void_t ) noexcept 
{
    motor::ogl::gl::init() ;
}