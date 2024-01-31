#include "wgl_context.h"

#include <motor/graphics/backend/gen4/null.h>

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/wgl/wgl.h>
#include <motor/std/string_split.hpp>
#include <motor/log/global.h>

using namespace motor::platform ;
using namespace motor::platform::wgl ;

//***********************************************************************
wgl_context::wgl_context( void_t ) noexcept
{
}

//***********************************************************************
wgl_context::wgl_context( HWND hwnd ) noexcept
{
    this_t::create_context( hwnd ) ;
}

//***********************************************************************
wgl_context::wgl_context( HWND hwnd, HGLRC ctx ) noexcept
{
    _hwnd = hwnd ;
    _hrc = ctx ;    
}

//***********************************************************************
wgl_context::wgl_context( this_rref_t rhv ) noexcept
{
    *this = std::move( rhv ) ;
}

//***********************************************************************
wgl_context::~wgl_context( void_t ) noexcept
{
    if( motor::platform::no_success( this_t::activate() ) )
    {
        // clear out all backend object silently
        // this may happen if the window is already closed but the 
        // backend is still going on.
        if( _backend != nullptr ) _backend->clear_all_objects() ;
    }

    // the backend can not exist without the context.
    assert( motor::memory::release_ptr( _backend ) == nullptr ) ;

    this_t::deactivate() ;

    if( _hrc != NULL )
        wglDeleteContext( _hrc ) ;
}

//***********************************************************************
wgl_context::this_ref_t wgl_context::operator = ( this_rref_t rhv ) noexcept
{
    _hwnd = rhv._hwnd ;
    rhv._hwnd = NULL ;
    _hdc = rhv._hdc ;
    rhv._hdc = NULL ;
    _hrc = rhv._hrc ;
    rhv._hrc = NULL ;

    _backend = motor::move( rhv._backend ) ;

    return *this ;
}

//***********************************************************************
motor::platform::result wgl_context::activate( void_t ) noexcept
{
    if( _hwnd == NULL ) return motor::platform::result::invalid_win32_handle ;

    assert( _hdc == NULL ) ;

    _hdc = GetDC( _hwnd ) ;

    if( _hdc == NULL ) return motor::platform::result::invalid_win32_handle ;

    if( motor::log::global::error( wglMakeCurrent( _hdc, _hrc ) == FALSE, 
        motor_log_fn( "wglMakeCurrent" ) ) ) 
        return motor::platform::result::failed_wgl ;
        
    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::result wgl_context::deactivate( void_t ) noexcept
{
    if( _hdc == NULL ) return motor::platform::result::ok ;

    if( motor::log::global::error( wglMakeCurrent( 0,0 ) == FALSE, 
        motor_log_fn( "wglMakeCurrent" ) ) ) 
        return motor::platform::result::failed_wgl ;

    if( motor::log::global::error( ReleaseDC( _hwnd, _hdc ) == FALSE, 
        motor_log_fn( "ReleaseDC" ) ) ) 
        return motor::platform::result::failed_wgl ;
    
    _hdc = NULL ;

    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::result wgl_context::vsync( bool_t const on_off ) noexcept
{
    motor::platform::result const res = this_t::is_extension_supported("WGL_EXT_swap_control") ;

    if( motor::log::global::error( res != motor::platform::result::ok, 
        "[wgl_context::vsync] : vsync not supported." ) ) 
        return res ;
    
    if( motor::log::global::error( motor::ogl::wgl::wglSwapInterval(on_off ? 1 : 0) != TRUE, 
        "[wgl_context::vsync] : wglSwapIntervalEXT" ) )
        return motor::platform::result::failed_wgl ;

    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::result wgl_context::swap( void_t ) noexcept
{
    if( _hdc == NULL ) 
        return motor::platform::result::invalid_win32_handle ;

    if( motor::log::global::error( SwapBuffers( _hdc ) == FALSE, 
        "[wgl_context::swap] : SwapBuffers") ) 
        return motor::platform::result::failed_wgl ;
    
    return motor::platform::result::ok ;
}

//***********************************************************************
motor::graphics::gen4::backend_mtr_shared_t wgl_context::backend( void_t ) noexcept 
{
    if( _backend != nullptr ) return motor::share( _backend ) ;

    motor::application::gl_version glv ;
    this->get_gl_version( glv ) ;

    // create gen 4 renderer
    if( glv.major >= 4 || (glv.major >= 4 && glv.minor >= 0) )
    {
        _backend = motor::memory::create_ptr( motor::platform::gen4::gl4_backend_t( this ) ) ;
    }
    else
    {
        motor::log::global_t::error( "Can not create requested OpenGL 4 renderer. "
            "OpenGL not matching the version requirement.") ;
    }
    
    return motor::share( _backend ) ;
}

//***********************************************************************
motor::platform::result wgl_context::create_context( HWND hwnd ) noexcept
{   
    _hwnd = hwnd ;
    
    if( motor::log::global::error( _hwnd == NULL, 
        "[wgl_context::create_context] : Window handle is no win32 handle." ) )
        return motor::platform::result::invalid_argument ;

    return this_t::create_the_context( motor::application::gl_info_t() ) ;
}

//***********************************************************************
motor::platform::result wgl_context::is_extension_supported( motor::string_cref_t extension_name ) const noexcept
{
    motor::vector< motor::string_t > ext_list ;
    if( this_t::get_wgl_extension(ext_list) != motor::platform::result::ok ) 
        return motor::platform::result::failed_wgl ;

    auto const iter = std::find( ext_list.begin(), ext_list.end(), extension_name ) ;

    return iter != ext_list.end() ? 
        motor::platform::result::ok : motor::platform::result::invalid_extension ;
}

//***********************************************************************
motor::platform::result wgl_context::get_wgl_extension( motor::vector< motor::string_t > & ext_list ) const noexcept
{
    if( !motor::ogl::wgl::wglGetExtensionsString ) 
        return motor::platform::result::invalid_extension ;

    char_cptr_t ch = motor::ogl::wgl::wglGetExtensionsString( _hdc ) ;
    motor::mstd::string_ops::split( motor::string_t(ch), ' ', ext_list ) ;

    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::result wgl_context::get_gl_extension( motor::vector< motor::string_t > & ext_list ) noexcept
{
    const GLubyte * ch = motor::ogl::glGetString( GL_EXTENSIONS ) ;
    if( !ch ) return motor::platform::result::failed ;

    motor::mstd::string_ops::split( motor::string_t(char_cptr_t(ch)), ' ', ext_list ) ;

    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::result wgl_context::get_gl_version( motor::application::gl_version & version ) const noexcept
{
    const GLubyte* ch = motor::ogl::glGetString(GL_VERSION) ;
    if( !ch ) return motor::platform::result::failed ;

    std::string version_string = std::string((const char*)ch) ;

    GLint major = 0 ;
    GLint minor = 0 ;

    {
        motor::ogl::glGetIntegerv( GL_MAJOR_VERSION, &major ) ;
        GLenum err = motor::ogl::glGetError() ;
        if( err != GL_NO_ERROR )
        {
            motor::string_t const es = motor::to_string(err) ;
            motor::log::global::error( "[wgl_context::get_gl_version] : get gl major <"+es+">" ) ;
        }
    }
    {
        motor::ogl::glGetIntegerv( GL_MINOR_VERSION, &minor) ;
        GLenum err = motor::ogl::glGetError() ;
        if( err != GL_NO_ERROR )
        {
            motor::string_t es = motor::to_string(err) ;
            motor::log::global::error( "[wgl_context::get_gl_version] : get gl minor <"+es+">" ) ;
        }
    }
    version.major = major ;
    version.minor = minor ;

    return motor::platform::result::ok ;
}

//***********************************************************************
void_t wgl_context::clear_now( motor::math::vec4f_t const & vec ) noexcept
{
    motor::ogl::glClearColor( vec.x(), vec.y(), vec.z(), vec.w() ) ;
    motor::ogl::glClear( GL_COLOR_BUFFER_BIT ) ;
    
    GLenum const gler = motor::ogl::glGetError() ;
    motor::log::global::error( gler != GL_NO_ERROR, "[wgl_context::clear_now] : glClear" ) ;
}

//***********************************************************************
motor::platform::result wgl_context::create_the_context( motor::application::gl_info_cref_t gli ) noexcept
{
    typedef std::chrono::high_resolution_clock local_clock_t ;
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

    if( motor::log::global::error(_hrc == NULL, 
        "[wgl_context::create_the_context] : Failed to create the temporary context.") ) 
        return result::failed_gfx_context_creation ;

    if( motor::log::global::error( wglMakeCurrent( hdc, _hrc ) == FALSE, 
        "[wgl_context::create_the_context] : wglMakeCurrent" ) )
    {
        return motor::platform::result::failed ;
    }

    // init the current "old" gl context. We need it in
    // order to determine the "new" 3.x+ context availability.
    motor::ogl::wgl::init( hdc ) ;

    HGLRC new_context ;
    if( motor::ogl::wgl::is_supported("WGL_ARB_create_context") )
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
        new_context = motor::ogl::wgl::wglCreateContextAttribs(hdc, NULL, attribList) ;
        if( new_context != NULL )
        {
            wglMakeCurrent( 0, 0 ) ;
            wglDeleteContext(_hrc) ;
            wglMakeCurrent(hdc, new_context) ;
            _hrc = new_context ;
        }

        // now using the new 3.x+ context.
        motor::ogl::gl::init() ;
    }

    //
    // Check ogl version against passed gl context info
    {
        motor::application::gl_version version ;
        if( !success( this_t::get_gl_version( version ) ) )
        {
            motor::log::global_t::error( motor_log_fn( "" ) ) ;
            wglMakeCurrent( 0, 0 ) ;
            return result::failed_gfx_context_creation ;
        }
    }

    wglMakeCurrent( 0, 0 ) ;
    ReleaseDC( _hwnd, hdc ) ;
 
    {
        this_t::activate() ;
        
        motor::log::global::warning( motor::platform::no_success( this_t::vsync( gli.vsync_enabled ) ), 
            "[wgl_context::create_the_context] : vsync setting failed." ) ;
        
        this_t::deactivate() ;
    }

    // timing end
    {
        size_t const milli = size_t( std::chrono::duration_cast<std::chrono::milliseconds>(
            local_clock_t::now() - t1).count()) ;

        motor::log::global::status( motor_log_fn( "created [" + std::to_string(milli) +" ms]" ) ) ;
    }

    return motor::platform::result::ok ;
}
