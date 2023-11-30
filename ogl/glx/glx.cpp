#include "glx.h"

#include <motor/log/global.h>
#include <motor/std/string_split.hpp>

using namespace motor ;
using namespace motor::ogl ;

/////////////////////////////////////////////////////////////////////////
// some defines
/////////////////////////////////////////////////////////////////////////

#define CHECK_AND_LOAD_COND( fn, name ) \
    !motor::log::global_t::error( \
    (fn = (fn == NULL ? (decltype(fn))(this_t::load_glx_function( name )) : fn)) == NULL, \
    "[CHECK_AND_LOAD_COND] : Failed to load: "  name  )

#define CHECK_AND_LOAD( fn, name ) \
{ \
    if( fn == NULL ) \
    { \
        fn = (decltype(fn))(this_t::load_glx_function( name )) ; \
    } \
    \
    so_gli::log::error( fn == NULL, "[CHECK_AND_LOAD] : Failed to load: "  name ) ; \
}

#define NULL_STATIC_MEMBER( fn ) decltype(glx::fn) glx::fn = nullptr ;

/////////////////////////////////////////////////////////////////////////
// static member init
/////////////////////////////////////////////////////////////////////////

glx::strings_t glx::_glx_extensions = glx::strings_t() ;

NULL_STATIC_MEMBER( glXCreateContextAttribs )
NULL_STATIC_MEMBER( glXSwapInterval )
NULL_STATIC_MEMBER( glXChooseFBConfig )

#if 0
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
#endif

/////////////////////////////////////////////////////////////////////////
// Function definitions
/////////////////////////////////////////////////////////////////////////

//**************************************************************
void_ptr_t glx::load_glx_function( char_cptr_t name ) 
{
    return (void *)glXGetProcAddress( (GLubyte const *)name) ;
}

//**************************************************************
bool_t glx::is_supported( char_cptr_t name ) 
{
    if( motor::log::global_t::warning( _glx_extensions.size() == 0, 
        "[glx::is_supported] : extension string is empty" ) )
    {
        return false ;
    }

    auto iter = ::std::find( _glx_extensions.begin(), _glx_extensions.end(), name ) ;
    if( iter == _glx_extensions.end() )
    {
        return false ;
    }

    return true ;
}

//**************************************************************
motor::ogl::result glx::init( Display * display, int screen ) 
{
    {
        char_cptr_t extensions = glXQueryExtensionsString( display, screen ) ;

        motor::vector< motor::string_t > extension_strings ;
        motor::mstd::string_ops::split( motor::string_t(char_cptr_t(extensions)), ' ', extension_strings ) ;

        for( auto const & e : extension_strings ) _glx_extensions.push_back( e.c_str() ) ;
    }

    if( !CHECK_AND_LOAD_COND( glXCreateContextAttribs, "glXCreateContextAttribsARB" ) )
    {
        return motor::ogl::result::failed ;
    }

    
    if( !CHECK_AND_LOAD_COND( glXSwapInterval, "glXSwapIntervalEXT" ) )
    {
        return motor::ogl::result::failed ;
    }

    if( !CHECK_AND_LOAD_COND( glXChooseFBConfig, "glXChooseFBConfig" ) )
    {
        return motor::ogl::result::failed ;
    }

#if 0
    
    if( CHECK_AND_LOAD_COND( wglGetExtensionsString, "wglGetExtensionsStringARB" ) )
    {
        char const * wglext = wglGetExtensionsString( hdc );
        std::string extension_string((const char*)wglext);

        typedef boost::tokenizer< boost::char_separator<char> > tokenizer_t;
        tokenizer_t tokens(extension_string, boost::char_separator<char>(" "));
        for (tokenizer_t::const_iterator token = tokens.begin(); token != tokens.end(); ++token)
        {
            _wgl_extensions.push_back(*token);
        }
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
            so_gli::log::error( "[" SNAKEOIL_FUNC "] : Function pointer not found. Although extension supported."  ) ;
            return so_gli::failed_load_function ;
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

    int bp = 0;
    (void)bp;

#endif
#if 0
    PFNGLGETSTRINGPROC fnstring = (PFNGLGETSTRINGPROC)
        this_t::load_wgl_function("glGetString");

    {
        GLubyte const * extensions = fnstring(GL_EXTENSIONS);
        (void)extensions;

        std::list<std::string> ext_list;
        std::string extension_string((const char*)extensions);
        typedef boost::tokenizer< boost::char_separator<char> > tokenizer_t;
        tokenizer_t tokens(extension_string, boost::char_separator<char>(" "));
        for (tokenizer_t::const_iterator token = tokens.begin(); token != tokens.end(); ++token)
        {
            ext_list.push_back(*token);
        }
    }
    (void)fnstring ;

    {
        PFNGLGETINTEGERVPROC glgetintegerv =
            (PFNGLGETINTEGERVPROC)this_t::load_wgl_function("glGetIntegerv");

        PFNGLGETSTRINGIPROC glgetstringi = (PFNGLGETSTRINGIPROC)
            this_t::load_wgl_function("glGetStringi");

        GLint numext = 0 ;
        glgetintegerv( GL_NUM_EXTENSIONS, &numext ) ;
        std::list<std::string> ext_list;
        for( GLint i=0; i<numext; ++i )
        {
            GLubyte const * name = glgetstringi( GL_EXTENSIONS, i ) ;
            ext_list.push_back( std::string((char const *)name)) ;
        }
        int bp = 0;
        (void)bp;
        (void)ext_list;

    }
#endif
    //char_cptr_t ext = wglGetExtensionsString() ;

    
    return motor::ogl::result::ok ;
}

