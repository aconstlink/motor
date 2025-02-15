#include "wgl_context.h"

#include <motor/graphics/backend/gen4/null.h>

#include <motor/ogl/gl/gl.h>
#include <motor/ogl/wgl/wgl.h>
#include <motor/std/string_split.hpp>
#include <motor/log/global.h>

#define wgl_context_log( text ) "[WGL Context] : " text

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
wgl_context::wgl_context( HWND hwnd, HGLRC ctx, motor::platform::gen4::gl4_backend_mtr_safe_t be ) noexcept :
    _hwnd( hwnd ), _hrc( ctx ), _backend( motor::move( be) )
{
}

//***********************************************************************
wgl_context::wgl_context( this_rref_t rhv ) noexcept :
    _hwnd( motor::move( rhv._hwnd) ), _hrc( motor::move( rhv._hrc) ), _backend( motor::move( rhv._backend) ),
    _attrib_list( std::move( rhv._attrib_list ) ), _shared_contexts( std::move( rhv._shared_contexts ) )
{
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
    
    _backend = motor::memory::release_ptr( _backend ) ;

    // the backend can not exist without the context.
    assert( _backend == nullptr ) ;

    this_t::deactivate() ;

    if( _hrc != NULL )
        wglDeleteContext( _hrc ) ;
}

//***********************************************************************
motor::platform::result wgl_context::activate( void_t ) noexcept
{
    if( _hwnd == NULL ) return motor::platform::result::invalid_win32_handle ;
    if( _active ) return motor::platform::result::ok ;

    HDC hdc = GetDC( _hwnd ) ;
    if( hdc == NULL ) return motor::platform::result::win32_hdc_failed ;

    auto const res = wglMakeCurrent( hdc, _hrc ) ;
    ReleaseDC( _hwnd, hdc ) ;
    
    if( motor::log::global::error( res == FALSE, wgl_context_log( "wglMakeCurrent" ) ) ) 
    {
        return motor::platform::result::failed_wgl ;
    }
    
    _active = true ;
    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::result wgl_context::deactivate( void_t ) noexcept
{
    if( !_active ) return motor::platform::result::context_not_active ;

    if( motor::log::global::error( wglMakeCurrent( 0,0 ) == FALSE, 
        wgl_context_log( "wglMakeCurrent(00)" ) ) ) 
        return motor::platform::result::failed_wgl ;

    _active = false ;
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
    HDC hdc = GetDC( _hwnd ) ;
    auto const res = SwapBuffers( hdc ) ;
    ReleaseDC( _hwnd, hdc ) ;

    if( motor::log::global::error( res == FALSE, 
        "[wgl_context::swap] : SwapBuffers") ) 
        return motor::platform::result::failed_wgl ;
    
    return motor::platform::result::ok ;
}

//***********************************************************************
motor::graphics::gen4::backend_mtr_safe_t wgl_context::backend( void_t ) noexcept 
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
motor::graphics::gen4::backend_borrow_t::mtr_t wgl_context::borrow_backend( void_t ) noexcept 
{
    if( _backend != nullptr ) return _backend ;
    return motor::memory::release_ptr( this_t::backend() ) ;
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

    HDC hdc = GetDC( _hwnd ) ;
    char_cptr_t ch = motor::ogl::wgl::wglGetExtensionsString( hdc ) ;
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
        _attrib_list = 
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, gli.version.major,
            WGL_CONTEXT_MINOR_VERSION_ARB, gli.version.minor,
#if defined( MOTOR_DEBUG )
            context_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#else
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0,        //End
        };

        // We create the new OpenGL 3.x+ context.
        //new_context = wglCreateContextAttribsARB(hdc, NULL, attribList) ;
        new_context = motor::ogl::wgl::wglCreateContextAttribs( hdc, NULL, _attrib_list.data() ) ;
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
            motor::log::global_t::error( wgl_context_log( "" ) ) ;
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

        //motor::log::global::status<2048>( "[wgl_context] : created [%zu ms]", milli ) ;
    }

    return motor::platform::result::ok ;
}

//***********************************************************************
motor::platform::opengl::rendering_context_mtr_safe_t 
                    wgl_context::create_shared( void_t ) noexcept 
{
    if( _attrib_list[0] == 0 ) 
    {
        motor::log::global::error( "[wgl_context] : can not create shared context from shared context." ) ;

        return motor::platform::opengl::rendering_context_mtr_safe_t() ;
    }

    typedef std::chrono::high_resolution_clock local_clock_t ;
    auto t1 = local_clock_t::now() ;

    HDC hdc = GetDC( _hwnd ) ;
    HGLRC shared_ctx = motor::ogl::wgl::wglCreateContextAttribs( hdc, _hrc, _attrib_list.data() ) ;
    ReleaseDC( _hwnd, hdc ) ;

    if( shared_ctx == NULL )
    {
        motor::log::global::error( "[wgl_context] : unable to create shared context" ) ;
        return motor::platform::opengl::rendering_context_mtr_safe_t() ;
    }

    _shared_contexts.emplace_back( shared_ctx ) ;

    // timing end
    {
        size_t const milli = size_t( std::chrono::duration_cast<std::chrono::milliseconds>(
            local_clock_t::now() - t1).count()) ;

        motor::log::global::status<2048>( "[wgl_context] : shared created  [%zu ms]", milli ) ;
    }

    return motor::shared( std::move( this_t( _hwnd, shared_ctx, motor::share( _backend ) ) ), 
        "[wgl_context] : created_shared" ) ;
}