#include "wgl_context.h"

#include <natus/graphics/backend/null/null.h>

#include <natus/core/assert.h>

#include <natus/ogl/gl/gl.h>
#include <natus/ogl/wgl/wgl.h>

#include <natus/ntd/string/split.hpp>

#include <natus/log/global.h>

using namespace natus::application ;
using namespace natus::application::wgl ;

//***********************************************************************
context::context( void_t )
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::wgl::gl_context( this ),
        "[context] : backend gl_context" ) ;
}

//***********************************************************************
context::context( HWND hwnd ) 
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::wgl::gl_context( this ),
        "[context] : backend gl_context" ) ;

    this_t::create_context( hwnd ) ;
}

//***********************************************************************
context::context( HWND hwnd, HGLRC ctx ) 
{
    _hwnd = hwnd ;
    _hrc = ctx ;

    _bend_ctx = natus::memory::global_t::alloc( natus::application::wgl::gl_context( this ),
        "[context] : backend gl_context" ) ;
}

//***********************************************************************
context::context( this_rref_t rhv )
{
    *this = ::std::move( rhv ) ;
    natus_move_member_ptr( _bend_ctx, rhv ) ;
    _bend_ctx->change_owner( this ) ;
}

//***********************************************************************
context::~context( void_t )
{
    this_t::deactivate() ;

    if( _hrc != NULL )
        wglDeleteContext( _hrc ) ;

    natus::memory::global_t::dealloc( _bend_ctx ) ;
}

//***********************************************************************
context::this_ref_t context::operator = ( this_rref_t rhv )
{
    _hwnd = rhv._hwnd ;
    rhv._hwnd = NULL ;
    _hdc = rhv._hdc ;
    rhv._hdc = NULL ;
    _hrc = rhv._hrc ;
    rhv._hrc = NULL ;

    return *this ;
}

//***********************************************************************
natus::application::result context::activate( void_t ) 
{
    natus_assert( _hdc == NULL ) ;

    _hdc = GetDC( _hwnd ) ;

    if( natus::log::global::error( wglMakeCurrent( _hdc, _hrc ) == FALSE, 
        natus_log_fn( "wglMakeCurrent" ) ) ) 
        return natus::application::result::failed_wgl ;
        
    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::deactivate( void_t ) 
{
    if( _hdc == NULL ) return natus::application::result::ok ;

    if( natus::log::global::error( wglMakeCurrent( 0,0 ) == FALSE, 
        natus_log_fn( "wglMakeCurrent" ) ) ) 
        return natus::application::result::failed_wgl ;

    if( natus::log::global::error( ReleaseDC( _hwnd, _hdc ) == FALSE, 
        natus_log_fn( "ReleaseDC" ) ) ) 
        return natus::application::result::failed_wgl ;
    
    _hdc = NULL ;

    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::vsync( bool_t const on_off ) 
{
    natus::application::result const res = this_t::is_extension_supported("WGL_EXT_swap_control") ;

    if( natus::log::global::error( natus::application::no_success(res), 
        "[context::vsync] : vsync not supported." ) ) 
        return res ;
    
    if( natus::log::global::error( natus::ogl::wgl::wglSwapInterval(on_off ? 1 : 0) != TRUE, 
        "[context::vsync] : wglSwapIntervalEXT" ) )
        return natus::application::result::failed_wgl ;

    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::swap( void_t ) 
{
    if( _hdc == NULL ) 
        return natus::application::result::invalid_win32_handle ;

    if( natus::log::global::error( SwapBuffers( _hdc ) == FALSE, 
        "[context::swap] : SwapBuffers") ) 
        return natus::application::result::failed_wgl ;
    
    return natus::application::result::ok ;
}

//***********************************************************************
natus::graphics::backend_res_t context::create_backend( void_t ) noexcept 
{
    natus::application::gl_version glv ;
    this->get_gl_version( glv ) ;
    if( glv.major >= 4 || (glv.major >= 4 && glv.minor >= 0) )
    {
        return natus::graphics::gl4_backend_res_t(
            natus::graphics::gl4_backend_t( _bend_ctx ) ) ;
    }
    
    return natus::graphics::null_backend_res_t(
        natus::graphics::null_backend_t() ) ;
}

//***********************************************************************
natus::application::result context::create_context( HWND hwnd ) 
{   
    _hwnd = hwnd ;
    
    if( natus::log::global::error( _hwnd == NULL, 
        "[context::create_context] : Window handle is no win32 handle." ) )
        return natus::application::result::invalid_argument ;

    return this_t::create_the_context( natus::application::gl_info_t() ) ;
}

//***********************************************************************
natus::application::result context::is_extension_supported( natus::ntd::string_cref_t extension_name ) 
{
    natus::ntd::vector< natus::ntd::string_t > ext_list ;
    if( natus::application::no_success( get_wgl_extension(ext_list) ) ) return natus::application::result::failed_wgl ;

    auto const iter = ::std::find( ext_list.begin(), ext_list.end(), extension_name ) ;

    return iter != ext_list.end() ? 
        natus::application::result::ok : natus::application::result::invalid_extension ;
}

//***********************************************************************
natus::application::result context::get_wgl_extension( natus::ntd::vector< natus::ntd::string_t > & ext_list )
{
    if( !natus::ogl::wgl::wglGetExtensionsString ) 
        return natus::application::result::invalid_extension ;

    char_cptr_t ch = natus::ogl::wgl::wglGetExtensionsString( _hdc ) ;
    natus::ntd::string_ops::split( natus::ntd::string_t(ch), ' ', ext_list ) ;

    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::get_gl_extension( natus::ntd::vector< natus::ntd::string_t > & ext_list )
{
    const GLubyte * ch = natus::ogl::glGetString( GL_EXTENSIONS ) ;
    if( !ch ) return natus::application::result::failed ;

    natus::ntd::string_ops::split( natus::ntd::string_t(char_cptr_t(ch)), ' ', ext_list ) ;

    return natus::application::result::ok ;
}

//***********************************************************************
natus::application::result context::get_gl_version( natus::application::gl_version & version ) const 
{
    const GLubyte* ch = natus::ogl::glGetString(GL_VERSION) ;
    if( !ch ) return natus::application::result::failed ;

    std::string version_string = std::string((const char*)ch) ;

    GLint major = 0 ;
    GLint minor = 0 ;

    {
        natus::ogl::glGetIntegerv( GL_MAJOR_VERSION, &major ) ;
        GLenum err = natus::ogl::glGetError() ;
        if( err != GL_NO_ERROR )
        {
            natus::ntd::string_t const es = ::std::to_string(err) ;
            natus::log::global::error( "[context::get_gl_version] : get gl major <"+es+">" ) ;
        }
    }
    {
        natus::ogl::glGetIntegerv( GL_MINOR_VERSION, &minor) ;
        GLenum err = natus::ogl::glGetError() ;
        if( err != GL_NO_ERROR )
        {
            natus::ntd::string_t es = ::std::to_string(err) ;
            natus::log::global::error( "[context::get_gl_version] : get gl minor <"+es+">" ) ;
        }
    }
    version.major = major ;
    version.minor = minor ;

    return natus::application::result::ok ;
}

//***********************************************************************
void_t context::clear_now( natus::math::vec4f_t const & vec ) 
{
    natus::ogl::glClearColor( vec.x(), vec.y(), vec.z(), vec.w() ) ;
    natus::ogl::glClear( GL_COLOR_BUFFER_BIT ) ;
    
    GLenum const gler = natus::ogl::glGetError() ;
    natus::log::global::error( gler != GL_NO_ERROR, "[context::clear_now] : glClear" ) ;
}

//***********************************************************************
natus::application::result context::create_the_context( gl_info_cref_t gli ) 
{
    typedef ::std::chrono::high_resolution_clock local_clock_t ;
    auto t1 = local_clock_t::now() ;

    HDC hdc = GetDC( _hwnd ) ;

    PIXELFORMATDESCRIPTOR pfd ;
    ZeroMemory( &pfd, sizeof(pfd) ) ;
    pfd.nSize = sizeof( pfd ) ;
    pfd.nVersion = 1 ;
    {
        DWORD double_buffer = gli.double_buffer == true ? PFD_DOUBLEBUFFER : DWORD(0) ;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | double_buffer ;
    }
    pfd.iPixelType = PFD_TYPE_RGBA ;
    pfd.cColorBits = (BYTE)gli.color_bits ;
    pfd.cDepthBits = (BYTE)gli.depth_bits ;
    pfd.iLayerType = PFD_MAIN_PLANE ;
    int format = ChoosePixelFormat( hdc, &pfd ) ;

    SetPixelFormat( hdc, format, &pfd ) ;

    // create the temporary context.
    // we still need to determine if OpenGL 3.x is supported.
    _hrc = wglCreateContext( hdc ) ;

    if( natus::log::global::error(_hrc == NULL, 
        "[context::create_the_context] : Failed to create the temporary context.") ) 
        return result::failed_gfx_context_creation ;

    if( natus::log::global::error( wglMakeCurrent( hdc, _hrc ) == FALSE, 
        "[context::create_the_context] : wglMakeCurrent" ) )
    {
        return natus::application::result::failed ;
    }

    // init the current "old" gl context. We need it in
    // order to determine the "new" 3.x+ context availability.
    natus::ogl::wgl::init( hdc ) ;

    HGLRC new_context ;
    if( natus::ogl::wgl::is_supported("WGL_ARB_create_context") )
    {
        const int attribList[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, gli.version.major,
            WGL_CONTEXT_MINOR_VERSION_ARB, gli.version.minor,
#if defined( NATUS_DEBUG )
            context_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#else
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,        //End
        };

        // We create the new OpenGL 3.x+ context.
        //new_context = wglCreateContextAttribsARB(hdc, NULL, attribList) ;
        new_context = natus::ogl::wgl::wglCreateContextAttribs(hdc, NULL, attribList) ;
        if( new_context != NULL )
        {
            wglMakeCurrent( 0, 0 ) ;
            wglDeleteContext(_hrc) ;
            wglMakeCurrent(hdc, new_context) ;
            _hrc = new_context ;
        }

        // now using the new 3.x+ context.
        natus::ogl::gl::init() ;
    }

    //
    // Check ogl version against passed gl context info
    {
        gl_version version ;
        if( !success( this_t::get_gl_version( version ) ) )
        {
            natus::log::global_t::error( natus_log_fn( "" ) ) ;
            wglMakeCurrent( 0, 0 ) ;
            return result::failed_gfx_context_creation ;
        }
    }

    wglMakeCurrent( 0, 0 ) ;
    ReleaseDC( _hwnd, hdc ) ;
 
    {
        this_t::activate() ;
        
        natus::log::global::warning( natus::application::no_success( this_t::vsync( gli.vsync_enabled ) ), 
            "[context::create_the_context] : vsync setting failed." ) ;
        
        this_t::deactivate() ;
    }

    // timing end
    {
        size_t const milli = size_t( ::std::chrono::duration_cast<::std::chrono::milliseconds>(
            local_clock_t::now() - t1).count()) ;

        natus::log::global::status( natus_log_fn( "created [" + ::std::to_string(milli) +" ms]" ) ) ;
    }

    return natus::application::result::ok ;
}
