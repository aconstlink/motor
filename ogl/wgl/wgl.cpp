#include "wgl.h"

#include <motor/log/global.h>
#include <motor/std/string_split.hpp>

using namespace motor::ogl ;

/////////////////////////////////////////////////////////////////////////
// some defines
/////////////////////////////////////////////////////////////////////////

#define CHECK_AND_LOAD_COND( fn, name ) \
    !motor::log::global::error( \
    (fn = (fn == NULL ? reinterpret_cast<decltype(fn)>(this_t::load_wgl_function( name )) : fn)) == NULL, \
    "[CHECK_AND_LOAD_COND] : Failed to load: "  name  )

#define CHECK_AND_LOAD( fn, name ) \
{ \
    if( fn == NULL ) \
    { \
        fn = reinterpret_cast<decltype(fn)>(this_t::load_wgl_function( name )) ; \
    } \
    \
    motor::log::global::error( fn == NULL, "[CHECK_AND_LOAD] : Failed to load: "  name ) ; \
}

#define NULL_STATIC_MEMBER( fn ) decltype(wgl::fn) wgl::fn = nullptr ;

/////////////////////////////////////////////////////////////////////////
// static member init
/////////////////////////////////////////////////////////////////////////

wgl::strings_t wgl::_wgl_extensions = wgl::strings_t() ;

NULL_STATIC_MEMBER( wglCreateBufferRegion )
NULL_STATIC_MEMBER( wglDeleteBufferRegion )
NULL_STATIC_MEMBER( wglSaveBufferRegion )
NULL_STATIC_MEMBER( wglRestoreBufferRegion )
 
NULL_STATIC_MEMBER( wglCreateContextAttribs )
NULL_STATIC_MEMBER( wglGetExtensionsString )
NULL_STATIC_MEMBER( wglMakeContextCurrent )
NULL_STATIC_MEMBER( wglGetCurrentReadDC )

NULL_STATIC_MEMBER( wglCreatePbuffer )
NULL_STATIC_MEMBER( wglGetPbufferDC )
NULL_STATIC_MEMBER( wglReleasePbufferDC )
NULL_STATIC_MEMBER( wglDestroyPbuffer )
NULL_STATIC_MEMBER( wglQueryPbuffer )

NULL_STATIC_MEMBER( wglGetPixelFormatAttribiv )
NULL_STATIC_MEMBER( wglGetPixelFormatAttribfv )
NULL_STATIC_MEMBER( wglChoosePixelFormat )
NULL_STATIC_MEMBER( wglBindTexImage )
NULL_STATIC_MEMBER( wglReleaseTexImage )
NULL_STATIC_MEMBER( wglSetPbufferAttrib )

NULL_STATIC_MEMBER( wglSetStereoEmitterState3D )

NULL_STATIC_MEMBER( wglSwapInterval )
NULL_STATIC_MEMBER( wglGetSwapInterval )

/////////////////////////////////////////////////////////////////////////
// Function definitions
/////////////////////////////////////////////////////////////////////////

//**************************************************************
void_ptr_t wgl::load_wgl_function( char_cptr_t name ) 
{
    void *p = (void *)wglGetProcAddress(name);
    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);
    }

    return p;
}

//**************************************************************
bool_t wgl::is_supported( char_cptr_t name ) 
{
    if( motor::log::global::warning( _wgl_extensions.size() == 0, 
        "[wgl::is_supported] : extension string is empty" ) )
        return false ;    

    auto const iter = std::find( _wgl_extensions.begin(), _wgl_extensions.end(), name ) ;
    return iter != _wgl_extensions.end() ;
}

//**************************************************************
motor::ogl::result wgl::init( HDC hdc ) 
{
    if( CHECK_AND_LOAD_COND( wglGetExtensionsString, "wglGetExtensionsStringARB" ) )
    {
        char const * wglext = wglGetExtensionsString( hdc );

        motor::vector< motor::string_t > extensions ;
        motor::mstd::string_ops::split( motor::string_t(char_cptr_t(wglext)), ' ', extensions ) ;

        for( auto const & e : extensions ) _wgl_extensions.push_back( e.c_str() ) ;
    }

    if( this_t::is_supported("WGL_ARB_buffer_region") )
    {
        CHECK_AND_LOAD( wglCreateBufferRegion, "wglCreateBufferRegionARB" ) ;
        CHECK_AND_LOAD( wglDeleteBufferRegion, "wglDeleteBufferRegionARB" ) ;
        CHECK_AND_LOAD( wglSaveBufferRegion, "wglSaveBufferRegionARB" ) ;
        CHECK_AND_LOAD( wglRestoreBufferRegion, "wglRestoreBufferRegionARB" ) ;
    }

    if( this_t::is_supported("WGL_ARB_context_flush_control"))
    {
    }

    if( this_t::is_supported("WGL_ARB_create_context") )
    {
        if( !CHECK_AND_LOAD_COND( wglCreateContextAttribs, "wglCreateContextAttribsARB" ) )
        {
            motor::log::global::error( "[wgl::init] : Function pointer not found. Although extension supported."  ) ;
            return motor::ogl::result::failed_load_function ;
        }
    }

    if( this_t::is_supported("WGL_ARB_create_context_profile") )
    {
    }

    if( this_t::is_supported("WGL_ARB_create_context_robustness") )
    {
    }

    if( this_t::is_supported("WGL_ARB_extensions_string") )
    {
    }

    if( this_t::is_supported("WGL_ARB_framebuffer_sRGB") )
    {
    }

    if( this_t::is_supported("WGL_ARB_make_current_read") )
    {
        CHECK_AND_LOAD( wglMakeContextCurrent, "wglMakeContextCurrentARB" ) ;
        CHECK_AND_LOAD( wglGetCurrentReadDC, "wglGetCurrentReadDCARB" ) ;
    }

    if( this_t::is_supported("WGL_ARB_multisample") )
    {
    }

    if( this_t::is_supported("WGL_ARB_pbuffer") )
    {
        CHECK_AND_LOAD( wglCreatePbuffer, "wglCreatePbufferARB" ) ;
        CHECK_AND_LOAD( wglGetPbufferDC, "wglGetPbufferDCARB" ) ;
        CHECK_AND_LOAD( wglReleasePbufferDC, "wglReleasePbufferDCARB" ) ;
        CHECK_AND_LOAD( wglDestroyPbuffer, "wglDestroyPbufferARB" ) ;
        CHECK_AND_LOAD( wglQueryPbuffer, "wglQueryPbufferARB" ) ;
    }

    if( this_t::is_supported("WGL_ARB_pixel_format") )
    {
        CHECK_AND_LOAD( wglGetPixelFormatAttribiv, "wglGetPixelFormatAttribivARB" ) ;
        CHECK_AND_LOAD( wglGetPixelFormatAttribfv, "wglGetPixelFormatAttribfvARB" ) ;
        CHECK_AND_LOAD( wglChoosePixelFormat, "wglChoosePixelFormatARB" ) ;
    }

    if( this_t::is_supported("WGL_ARB_pixel_format_float") )
    {
    }

    if( this_t::is_supported("WGL_ARB_render_texture") )
    {
        CHECK_AND_LOAD( wglBindTexImage, "wglBindTexImageARB" ) ;
        CHECK_AND_LOAD( wglReleaseTexImage, "wglReleaseTexImageARB" ) ;
        CHECK_AND_LOAD( wglSetPbufferAttrib, "wglSetPbufferAttribARB" ) ;
    }

    if( this_t::is_supported("WGL_ARB_robustness_application_isolation") )
    {
    }

    if( this_t::is_supported("WGL_ARB_robustness_share_group_isolation") )
    {
    }

    if( this_t::is_supported("WGL_EXT_swap_control") )
    {
        CHECK_AND_LOAD( wglSwapInterval, "wglSwapIntervalEXT" ) ;
        CHECK_AND_LOAD( wglGetSwapInterval, "wglGetSwapIntervalEXT" ) ;
    }

    return motor::ogl::result::ok ;
}

