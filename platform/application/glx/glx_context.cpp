#include "glx_context.h"
#include "glx_config.h"

#include <motor/graphics/backend/gen4/null.h>

#include <motor/std/string_split.hpp>
#include <motor/log/global.h>

//#include <GL/glcorearb.h>
#include <motor/ogl/glx/glx.h>

using namespace motor::platform ;
using namespace motor::platform::glx ;

struct context::pimpl
{
    GLXContext context = nullptr ;
    GLXFBConfig config = nullptr ;
    GLXPbuffer pbuffer = 0 ;
    motor::application::gl_version version ;
} ;

//****************************************************************
context::context( void_t ) noexcept
{
    _pimpl = motor::memory::global_t::alloc( this_t::pimpl(), "glx context pimpl" ) ;
}

//****************************************************************
context::context( Window wnd, Display * disp ) noexcept
{
    _display = disp ;
    _wnd = wnd ;
    _pimpl = motor::memory::global_t::alloc( this_t::pimpl(), "glx context pimpl" ) ;
    this_t::create_the_context( motor::application::gl_info_t()  ) ;
}

//****************************************************************
context::context( this_rref_t rhv ) noexcept
{
    _display = rhv._display ;
    rhv._display = NULL ;

    _wnd = rhv._wnd ;
    rhv._wnd = 0 ;

    _pimpl = motor::move( rhv._pimpl ) ;
    _backend = motor::move( rhv._backend ) ;
}

//****************************************************************
context::this_ref_t context::operator = ( this_rref_t rhv ) noexcept
{
    if( this == &rhv ) return *this ;
    this_t::release_context() ;
    _display = rhv._display ;
    rhv._display = NULL ;

    _wnd = rhv._wnd ;
    rhv._wnd = 0 ;

    _pimpl = motor::move( rhv._pimpl ) ;
    
    _backend = motor::move( rhv._backend ) ;

    return *this ;
}

//****************************************************************
context::~context( void_t ) noexcept
{
    this_t::release_context() ;
}

//****************************************************************
void_t context::release_context( void_t ) noexcept
{
    if( _pimpl == nullptr ) return ;

    if( _backend != nullptr && motor::platform::no_success( this_t::activate() ) )
    {
        _backend->on_context_destruction() ;
    }

    _backend = motor::memory::release_ptr( _backend ) ;

    // the backend can not exist without the context.
    assert( _backend == nullptr ) ;

    if( _display != nullptr && _pimpl->context != nullptr )
    {
        // Destroying a shared context must not unbind the main context.
        this_t::deactivate() ;
        glXDestroyContext( _display, _pimpl->context ) ;
    }
    if( _display != nullptr && _pimpl->pbuffer != 0 )
        glXDestroyPbuffer( _display, _pimpl->pbuffer ) ;

    motor::memory::global_t::dealloc( _pimpl ) ;
    _pimpl = nullptr ;
    _display = nullptr ;
    _wnd = 0 ;
}

//***********************************************************************
motor::platform::result context::create_context( Window wnd, Display * disp ) noexcept
{   
    _display = disp ;
    _wnd = wnd ;
    
    if( motor::log::global::error( disp == NULL || _wnd == 0, 
        "[wgl_context::create_context] : Window handle is not valid." ) )
        return motor::platform::result::invalid_argument ;

    return this_t::create_the_context( motor::application::gl_info_t() ) ;
}

//***************************************************************
motor::platform::result context::activate( void_t ) noexcept
{
    if( _display == nullptr || _pimpl == nullptr || _pimpl->context == nullptr )
        return motor::platform::result::invalid_xlib_handle ;

    GLXDrawable const drawable = _pimpl->pbuffer != 0 ? _pimpl->pbuffer : _wnd ;
    auto const res = glXMakeContextCurrent( _display, drawable, drawable, _pimpl->context ) ;
    motor::log::global_t::warning( !res, 
            motor_log_fn( "glXMakeCurrent" ) ) ;

    return res ? motor::platform::result::ok : motor::platform::result::failed_glx ;
}

//***************************************************************
motor::platform::result context::deactivate( void_t ) noexcept
{
    if( _display == nullptr || _pimpl == nullptr || _pimpl->context == nullptr ||
        glXGetCurrentContext() != _pimpl->context ) return motor::platform::result::ok ;

    auto const res = glXMakeContextCurrent( _display, 0, 0, nullptr ) ;
    motor::log::global_t::warning( !res, 
            motor_log_fn( "glXMakeCurrent" ) ) ;
    return res ? motor::platform::result::ok : motor::platform::result::failed_glx ;
}

//***************************************************************
motor::platform::result context::vsync( bool_t const on_off ) noexcept
{
    if( _display == nullptr || _wnd == 0 ) return motor::platform::result::invalid_xlib_handle ;
    if( !motor::ogl::glx::is_supported( "GLX_EXT_swap_control" ) ||
        motor::ogl::glx::glXSwapInterval == nullptr ) return motor::platform::result::invalid_extension ;

    motor::ogl::glx::glXSwapInterval( _display, _wnd, on_off ? 1 : 0 ) ;
    return motor::platform::result::ok ;
}

//**************************************************************
motor::platform::result context::swap( void_t ) noexcept
{
    glXSwapBuffers( _display, _wnd ) ;
    const GLenum glerr = glGetError( ) ;
    motor::log::global_t::warning( glerr != GL_NO_ERROR, 
            motor_log_fn( "glXSwapBuffers" ) ) ;
    return motor::platform::result::ok ;
}

//**************************************************************
motor::graphics::gen4::backend_mtr_safe_t context::backend( void_t ) noexcept 
{
    if( _backend != nullptr ) return motor::share( _backend ) ;

    motor::application::gl_version glv ;
    if( motor::platform::no_success( this_t::activate() ) ||
        motor::platform::no_success( this_t::get_gl_version( glv ) ) ) return {} ;

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
motor::graphics::gen4::backend_borrow_t::mtr_t context::borrow_backend( void_t ) noexcept 
{
    if( _backend != nullptr ) return _backend ;
    return motor::memory::release_ptr( this_t::backend() ) ;
}

//***************************************************************
motor::platform::result context::is_extension_supported( 
    motor::string_cref_t extension_name ) const noexcept
{
    this_t::strings_t ext_list ;
    if( motor::platform::no_success( this_t::get_glx_extension(ext_list) ) ) 
        return motor::platform::result::failed_glx ;

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
motor::platform::result context::get_glx_extension( this_t::strings_out_t /*ext_list*/ ) const noexcept
{
    return motor::platform::result::ok ;
}

//****************************************************************
motor::platform::result context::get_gl_extension( this_t::strings_out_t ext_list ) const noexcept
{
    const GLubyte * ch = glGetString( GL_EXTENSIONS ) ;
    if( !ch ) return motor::platform::result::failed ;

    motor::string_t extension_string( (const char*)ch) ;
    motor::mstd::string_ops::split( extension_string, ' ', ext_list ) ;
    return motor::platform::result::ok ;
}

//****************************************************************
motor::platform::result context::get_gl_version( motor::application::gl_version & version ) const noexcept
{
    const GLubyte* ch = glGetString(GL_VERSION) ;
    if( !ch ) return motor::platform::result::failed ;

    motor::string_t version_string = motor::string_t((const char*)ch) ;

    GLint major = 0;
    GLint minor = 0;

    {
        glGetIntegerv( GL_MAJOR_VERSION, &major ) ;
        GLenum err = glGetError() ;
        if( err != GL_NO_ERROR )
        {
            motor::string_t const es = motor::to_string(err) ;
            motor::log::global::error( 
                "[context::get_gl_version] : get gl major <"+es+">" ) ;
        }
    }
    {
        glGetIntegerv( GL_MINOR_VERSION, &minor) ;
        GLenum err = glGetError() ;
        if( err != GL_NO_ERROR )
        {
            motor::string_t es = motor::to_string(err) ;
            motor::log::global::error( "[context::get_gl_version] : get gl minor <"+es+">" ) ;
        }
    }

    version.major = major ;
    version.minor = minor ;

    return motor::platform::result::ok ;
}

//****************************************************************
void_t context::clear_now( motor::math::vec4f_t const & vec ) noexcept
{
    glClearColor( vec.x(), vec.y(), vec.z(), vec.w() ) ;
    glClear( GL_COLOR_BUFFER_BIT ) ;
    
    GLenum const gler = glGetError() ;
    motor::log::global_t::error( gler != GL_NO_ERROR, "[context::clear_now] : glClear" ) ;
}

//***************************************************************
motor::platform::result context::create_the_context( motor::application::gl_info_cref_t gli ) noexcept
{
    XWindowAttributes attributes ;
    if( !XGetWindowAttributes( _display, _wnd, &attributes ) )
        return motor::platform::result::invalid_xlib_handle ;

    _pimpl->config = motor::platform::glx::choose_config( _display,
        XScreenNumberOfScreen( attributes.screen ), XVisualIDFromVisual( attributes.visual ) ) ;
    if( _pimpl->config == nullptr ) return motor::platform::result::failed_gfx_context_creation ;

    auto res = motor::ogl::glx::init( _display, DefaultScreen( _display ) ) ;

    if( motor::log::global_t::error( motor::ogl::no_success(res), 
               "[glx_window::create_glx_window] : init glx") )
    {
        return motor::platform::result::failed ;
    }

    int glx_major, glx_minor ;

    if( !glXQueryVersion( _display, &glx_major, &glx_minor ) ) 
    {
        motor::log::global_t::error( 
              "[glx_window::create_glx_window] : glXQueryVersion") ;
        return motor::platform::result::failed ;
    }

    if( glx_major < 1 || ( glx_major == 1 && glx_minor < 3 ) )
        return motor::platform::result::failed_glx ;

    // determine the GL version by creating a simple 1.0 context.
    motor::application::gl_version glv ;
    if( !this_t::determine_gl_version( glv ) )
    {
        motor::log::global_t::error( motor_log_fn(
           "failed to determine gl version ") ) ;
        return motor::platform::result::failed ;
    }

    int context_attribs[] =
    {
     GLX_CONTEXT_MAJOR_VERSION_ARB, glv.major,
     GLX_CONTEXT_MINOR_VERSION_ARB, glv.minor,
     None
    } ;

    GLXContext context = motor::ogl::glx::glXCreateContextAttribs( 
          _display, _pimpl->config,
          0, True, context_attribs );

    if( motor::log::global_t::error( !context, 
           motor_log_fn( "glXCreateContextAttribs" )) ) 
    {
        return motor::platform::result::failed ;
    }

    _pimpl->context = context ;
    _pimpl->version = glv ;
    if( motor::platform::no_success( this_t::activate() ) )
        return motor::platform::result::failed_gfx_context_creation ;

    this_t::init_gl_context() ;
    {
        motor::application::gl_version version ;
        if( !success( this_t::get_gl_version( version ) ) )
        {
            motor::log::global_t::error( motor_log_fn( "" ) ) ;
            this_t::deactivate() ;
            return result::failed_gfx_context_creation ;
        }
        motor::log::global_t::status( "GL Version: " +
           motor::to_string(version.major) + "." + 
           motor::to_string(version.minor) ) ;
    }

    {
        auto const res = this_t::vsync( gli.vsync_enabled ) ;
        motor::log::global_t::warning( motor::platform::no_success(res),
               motor_log_fn("vsync") ) ;
    }
    glXMakeCurrent( _display, 0, 0 ) ;


    _pimpl->context = context ;

    return motor::platform::result::ok ;
}

//****************************************************************
bool_t context::determine_gl_version( motor::application::gl_version & gl_out ) const noexcept
{
    int context_attribs[] =
    {
     GLX_CONTEXT_MAJOR_VERSION_ARB, 1,//gli.version.major,
     GLX_CONTEXT_MINOR_VERSION_ARB, 0,//gli.version.minor,
     None
    } ;

    GLXContext context = motor::ogl::glx::glXCreateContextAttribs( 
          _display, _pimpl->config,
          0, True, context_attribs );

    if( motor::log::global_t::error( !context, 
           motor_log_fn( "glXCreateContextAttribs") ) ) 
    {
        return false ;
    }

    motor::application::gl_version version ;
    if( !glXMakeCurrent( _display, _wnd, context ) )
    {
        glXDestroyContext( _display, context ) ;
        return false ;
    }

    this_t::init_gl_context() ;

    {
        if( !success( this_t::get_gl_version( version ) ) )
        {
            motor::log::global_t::error( motor_log_fn( "" ) ) ;
            glXMakeCurrent( _display, 0, 0 ) ;
            glXDestroyContext( _display, context ) ;
            return false ;
        }
        motor::log::global_t::status( "[determine_gl_version] : GL Version: " +
           motor::to_string(version.major) + "." + 
           motor::to_string(version.minor) ) ;
    }
    glXMakeCurrent( _display, 0, 0 ) ;
    glXDestroyContext( _display, context ) ;

    gl_out = version ;
    return true ;
}

motor::platform::opengl::rendering_context_mtr_safe_t context::create_shared( void_t ) noexcept 
{
    if( _display == nullptr || _pimpl == nullptr || _pimpl->context == nullptr ) return {} ;

    auto shared = motor::memory::create_ptr< this_t >( "glx shared context" ) ;
    shared->_display = _display ;
    shared->_pimpl->config = _pimpl->config ;
    shared->_pimpl->version = _pimpl->version ;

    int const context_attributes[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, _pimpl->version.major,
        GLX_CONTEXT_MINOR_VERSION_ARB, _pimpl->version.minor, None
    } ;
    shared->_pimpl->context = motor::ogl::glx::glXCreateContextAttribs(
        _display, _pimpl->config, _pimpl->context, True, context_attributes ) ;

    // The compiler thread gets its own drawable, not the visible window.
    int const pbuffer_attributes[] = { GLX_PBUFFER_WIDTH, 1, GLX_PBUFFER_HEIGHT, 1, None } ;
    if( shared->_pimpl->context != nullptr )
        shared->_pimpl->pbuffer = glXCreatePbuffer( _display, _pimpl->config, pbuffer_attributes ) ;

    if( shared->_pimpl->context == nullptr || shared->_pimpl->pbuffer == 0 )
    {
        motor::log::global_t::error( "[glx_context] : failed to create shared context" ) ;
        motor::release( motor::move( shared ) ) ;
        return {} ;
    }
    return motor::move( shared ) ;
}
