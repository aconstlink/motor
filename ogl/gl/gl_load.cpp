#include "gl_load.h"

#if defined( MOTOR_TARGET_OS_WIN )
#include <GL/wglext.h>
#elif defined( MOTOR_TARGET_OS_LIN )
//#include <X11/X.h>
//#include <X11/Xlib.h>
//#include <X11/Xutil.h>
//#include <X11/Xmd.h>
#include <GL/glx.h>
//#include <GL/glxext.h>
#endif

using namespace motor::ogl ;

//**************************************************************
void_ptr_t motor::ogl::gl_load::load_function( char_cptr_t name ) noexcept
{
#if defined( MOTOR_TARGET_OS_WIN )
    void *p = (void *)wglGetProcAddress(name);
    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);
    }
#elif defined( MOTOR_TARGET_OS_LIN )
    void_ptr_t p = (void_ptr_t) glXGetProcAddress( (GLubyte const *) name ) ;
#else
#error "Requires implementation"
#endif
    return p;
}