#include "egl_context.h"
#include "egl_window.h"

//#include <motor/graphics/backend/gl/es3.h>

#include <motor/std/string_split.hpp>
#include <GLES3/gl3.h>

using namespace motor::application ;
using namespace motor::application::egl ;

//****************************************************************
egl_context::egl_context( void_t ) noexcept
{
    _bend_ctx = motor::memory::global_t::alloc( motor::application::egl::es_context( this ),
        "[context] : backend es_context" ) ;
}

//****************************************************************
egl_context::egl_context( EGLNativeWindowType wnd, EGLNativeDisplayType disp ) noexcept
{
    _bend_ctx = motor::memory::global_t::alloc( motor::application::egl::es_context( this ),
        "[context] : backend es_context" ) ;

    _ndt = disp ;
    _wnd = wnd ;
    this_t::create_the_context( gli ) ;
}

//****************************************************************
egl_context::egl_context( this_rref_t rhv ) noexcept
{
    _display = rhv._display ;
    rhv._display = NULL ;

    _wnd = rhv._wnd ;
    rhv._wnd = 0 ;

    _context = rhv._context ;
    rhv._context = 0 ;

    _surface = rhv._surface ;
    rhv._surface = 0 ;

    _ndt = rhv._ndt ;
    rhv._ndt = 0 ;

    motor_move_member_ptr( _bend_ctx, rhv ) ;
    _bend_ctx->change_owner( this ) ;
}

//****************************************************************
egl_context::this_ref_t egl_context::operator = ( this_rref_t rhv )  noexcept
{
    _display = rhv._display ;
    rhv._display = NULL ;

    _wnd = rhv._wnd ;
    rhv._wnd = 0 ;

    _context = rhv._context ;
    rhv._context = 0 ;

    _surface = rhv._surface ;
    rhv._surface = 0 ;

    _ndt = rhv._ndt ;
    rhv._ndt = 0 ;

    motor_move_member_ptr( _bend_ctx, rhv ) ;
    _bend_ctx->change_owner( this ) ;
    return *this ;
}

//****************************************************************
egl_context::~context( void_t ) noexcept
{
    this_t::deactivate() ;
    motor::memory::global_t::dealloc( _bend_ctx ) ;
}

//***************************************************************
motor::platform::result egl_context::activate( void_t ) noexcept
{
    auto const res = eglMakeCurrent( _display, _surface, _surface, _context ) ;
    if( res == EGL_FALSE )
        return motor::platform::result::failed ;

    return motor::platform::result::ok ;
}

//***************************************************************
motor::platform::result egl_context::deactivate( void_t ) 
{
    auto const res = eglMakeCurrent( _display, EGL_NO_SURFACE, 
               EGL_NO_SURFACE, EGL_NO_CONTEXT ) ;
    if( res == EGL_FALSE )
        return motor::platform::result::failed ;
    return motor::platform::result::ok ;
}

//***************************************************************
motor::platform::result egl_context::vsync( bool_t const on_off ) 
{
    eglSwapInterval( _display, on_off ? 1 : 0 ) ;
    return motor::platform::result::ok ;
}

//**************************************************************
motor::platform::result egl_context::swap( void_t ) 
{
    eglSwapBuffers( _display, _surface ) ;
    return motor::platform::result::ok ;
}

//**************************************************************
motor::graphics::gen4::backend_mtr_safe_t egl_context::backend( void_t ) noexcept 
{
    #if 0
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
    #else
    return nullptr ;
    #endif
}

//***********************************************************************
motor::graphics::gen4::backend_borrow_t::mtr_t egl_context::borrow_backend( void_t ) noexcept 
{
    if( _backend != nullptr ) return _backend ;
    return motor::memory::release_ptr( this_t::backend() ) ;
}

#if 0
//***********************************************************************
motor::graphics::backend_res_t egl_context::create_backend( void_t ) noexcept
{
    motor::application::gl_version glv ;
    this->get_es_version( glv ) ;
    if( glv.major >= 3 )
    {
        return motor::graphics::es3_backend_res_t(
            motor::graphics::es3_backend_t( _bend_ctx ) ) ;
    }

    return motor::graphics::null_backend_res_t(
        motor::graphics::null_backend_t() ) ;
}
#endif
//***************************************************************
motor::platform::result egl_context::is_extension_supported( 
    motor::string_cref_t extension_name ) const noexcept
{
    this_t::strings_t ext_list ;
    if( motor::application::no_success( get_egl_extension(ext_list) ) ) 
        return motor::platform::result::failed_wgl ;

    this_t::strings_t::iterator iter = ext_list.begin() ;
    while( iter != ext_list.end() )
    {
        if( *iter == extension_name ) 
            return motor::platform::result::ok ;
        ++iter ;
    }
    return motor::platform::result::invalid_extension ;
}

//*****************************************************************
motor::platform::result egl_context::get_egl_extension( this_t::strings_out_t ext_list )
{
    char_cptr_t ch = eglQueryString( _display, EGL_EXTENSIONS ) ;
    if( !ch ) return motor::platform::result::failed ;
    
    motor::string_t extension_string( (const char*)ch) ;
    motor::string_ops::split( extension_string, ' ', ext_list ) ;

    return motor::platform::result::ok ;
}

//****************************************************************
motor::platform::result egl_context::get_es_extension( this_t::strings_out_t ext_list )
{
    const GLubyte * ch = glGetString( GL_EXTENSIONS ) ;
    if( !ch ) return motor::platform::result::failed ;

    motor::string_t extension_string( (const char*)ch) ;
    motor::string_ops::split( extension_string, ' ', ext_list ) ;
    return motor::platform::result::ok ;
}

//****************************************************************
motor::platform::result egl_context::get_es_version( motor::application::gl_version & version ) const 
{
    const GLubyte* ch = glGetString(GL_VERSION) ;
    if( !ch ) return motor::platform::result::failed ;

    motor::string_t version_string = motor::string((const char*)ch) ;

    GLint major = 0;//boost::lexical_cast<GLint, std::string>(*token) ;
    GLint minor = 0;//boost::lexical_cast<GLint, std::string>(*(++token));

    {
        glGetIntegerv( GL_MAJOR_VERSION, &major ) ;
        GLenum err = glGetError() ;
        if( err != GL_NO_ERROR )
        {
            motor::string_t const es = std::to_string(err) ;
            motor::log::global::error( 
                "[egl_context::get_gl_version] : get gl major <"+es+">" ) ;
        }
    }
    {
        glGetIntegerv( GL_MINOR_VERSION, &minor) ;
        GLenum err = glGetError() ;
        if( err != GL_NO_ERROR )
        {
            motor::string_t es = std::to_string(err) ;
            motor::log::global::error( "[egl_context::get_gl_version] : get gl minor <"+es+">" ) ;
        }
    }

    version.major = major ;
    version.minor = minor ;

    return motor::platform::result::ok ;
}


//****************************************************************
void_t egl_context::clear_now( motor::math::vec4f_t const & vec ) 
{
    {
        glClearColor( vec.x(), vec.y(), vec.z(), vec.w() ) ;
        glGetError() ;
    }
    {
        glClear( GL_COLOR_BUFFER_BIT ) ;
        glGetError() ;
    }
}

//***************************************************************
motor::platform::result egl_context::create_the_context( motor::application::gl_info_cref_t /*gli*/ ) 
{
    EGLConfig config ;
    EGLDisplay display = eglGetDisplay( _ndt ) ;

    {
        EGLint major = 0 ;
        EGLint minor = 0 ;

        auto const res = eglInitialize( display , &major, &minor ) ;
        if( res != EGL_TRUE )
        {
            motor::log::global_t::error( motor_log_fn("eglInitialize") ) ;
            return motor::platform::result::failed ;
        }

        motor::log::global_t::status( "[egl_context] : EGL Version " + 
               std::to_string( major ) + "." + std::to_string( minor ) ) ;
    }

    {
        auto const res = this_t::is_extension_supported("EGL_KHR_create_context") ;

        int const es3_supported = res == motor::platform::result::ok ?
            EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT ;

        EGLint numConfigs = 0 ;
        EGLint const  attribList[] = 
        {
            EGL_RED_SIZE, 5,
            EGL_GREEN_SIZE, 6,
            EGL_BLUE_SIZE, 5,
            EGL_ALPHA_SIZE, 1,
            EGL_DEPTH_SIZE, 1,
            EGL_STENCIL_SIZE, 1,
            EGL_SAMPLE_BUFFERS, 0,
            EGL_RENDERABLE_TYPE, es3_supported,
            EGL_NONE
        } ;

        if( !eglChooseConfig( display, attribList, &config, 1, &numConfigs ) )
        {
            motor::log::global_t::warning( motor_log_fn("eglChooseConfig") ) ;
            return motor::platform::result::failed ;
        }

        if( numConfigs < 1 ) 
        {
            motor::log::global_t::warning( motor_log_fn("numConfigs < 1") ) ;
            return motor::platform::result::failed ;
        }
    }

    {
        EGLSurface surface = eglCreateWindowSurface( display, config, _wnd, NULL ) ;
        if( surface == EGL_NO_SURFACE )
        {
            motor::log::global_t::warning( 
                motor_log_fn("eglCreateWindowSurface") ) ;
            return motor::platform::result::failed ;
        }
        _surface = surface ;
    }

    {
        EGLint const  attribList[] = 
        {
            EGL_CONTEXT_MAJOR_VERSION, 3, 
            EGL_CONTEXT_MINOR_VERSION, 2, 
            EGL_NONE
        } ;

        EGLContext context = eglCreateContext( display, config, 
                     EGL_NO_CONTEXT, attribList ) ;

        if( context == EGL_NO_CONTEXT )
        {
            motor::log::global_t::warning( motor_log_fn("eglCreateContext") ) ;
            return motor::platform::result::failed ;
        }
        _context = context ;
    }

    {
        eglMakeCurrent( display, _surface, _surface, _context ) ;
        {
            motor::application::gl_version v ;
            this_t::get_es_version( v ) ;
            motor::log::global_t::status( "[egl_context] : OpenGLES Version " 
                   + std::to_string( v.major ) + "." + std::to_string( v.minor ) ) ;
        }
        this_t::strings_t list ;
        this_t::get_es_extension( list ) ;
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear( GL_COLOR_BUFFER_BIT ) ;
        eglSwapBuffers( display, _surface ) ;
        glClearColor(1.0f,0.0f,1.0f,1.0f);
        glClear( GL_COLOR_BUFFER_BIT ) ;
        eglSwapBuffers( display, _surface ) ;

        eglMakeCurrent( display, EGL_NO_SURFACE, 
               EGL_NO_SURFACE, EGL_NO_CONTEXT ) ;
    }

    _display = display ;

    return motor::platform::result::ok ;
}

