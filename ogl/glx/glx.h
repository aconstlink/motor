#pragma once 

#include "../result.h"
#include "../typedefs.h"
#include "../api.h"

#include <motor/std/vector>

//#include <GL/glcorearb.h>
#include <GL/glx.h>
#include <GL/glxext.h>

namespace motor 
{
    namespace ogl
    {
        struct MOTOR_OGL_API glx
        {
            motor_this_typedefs( glx ) ;

        private:

            typedef motor::vector< char_t const * > strings_t ;
            static strings_t _glx_extensions ;

        public: 

            /// will initialize all static wgl functions. a context must
            /// be bound in order to get the correct driver .dll where 
            /// all functions are loaded from.
            /// @precondition a opengl context must be current.
            static motor::ogl::result init( Display *, int ) ;


            static bool_t is_supported( char_cptr_t name ) ;

        private:

            static void_ptr_t load_glx_function( char_cptr_t name ) ;

        public: 
            
            static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribs ;
            static PFNGLXSWAPINTERVALEXTPROC glXSwapInterval ;
            static PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig ;
            
#if 0
            static PFNWGLCREATEBUFFERREGIONARBPROC wglCreateBufferRegion;
            static PFNWGLDELETEBUFFERREGIONARBPROC wglDeleteBufferRegion;
            static PFNWGLSAVEBUFFERREGIONARBPROC wglSaveBufferRegion;
            static PFNWGLRESTOREBUFFERREGIONARBPROC wglRestoreBufferRegion;

            static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribs;

            static PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsString;
            static PFNWGLMAKECONTEXTCURRENTARBPROC wglMakeContextCurrent;
            static PFNWGLGETCURRENTREADDCARBPROC wglGetCurrentReadDC;

            static PFNWGLCREATEPBUFFERARBPROC wglCreatePbuffer;
            static PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDC;
            static PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDC;
            static PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbuffer;
            static PFNWGLQUERYPBUFFERARBPROC wglQueryPbuffer;

            static PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribiv;
            static PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfv;
            static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat;
            static PFNWGLBINDTEXIMAGEARBPROC wglBindTexImage;
            static PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImage;
            static PFNWGLSETPBUFFERATTRIBARBPROC wglSetPbufferAttrib;
            static PFNWGLSETSTEREOEMITTERSTATE3DLPROC wglSetStereoEmitterState3D;


        public: // ext

            static PFNWGLSWAPINTERVALEXTPROC wglSwapInterval ;
            static PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapInterval ;
            
#endif
        
        };
    }
}
