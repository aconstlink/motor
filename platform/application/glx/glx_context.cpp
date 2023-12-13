#include "glx_context.h"
#include "glx_window.h"

#include <natus/graphics/backend/gl/gl4.h>

#include <natus/ogl/gl/gl.h>
#include <natus/ogl/glx/glx.h>
#include <natus/ntd/string/split.hpp>

using namespace motor::application ;
using namespace motor::application::glx ;

//****************************************************************
context::context( void_t ) noexcept
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::glx::gl_context( this ),
        "[context] : backend gl_context" ) ;
}

//****************************************************************
context::context( gl_info_in_t gli, Window wnd, Display * disp ) noexcept
{
    _bend_ctx = natus::memory::global_t::alloc( natus::application::glx::gl_context( this ),
        "[context] : backend gl_context" ) ;

    _display = disp ;
    _wnd = wnd ;
    this_t::create_the_context( gli ) ;
}

//****************************************************************
context::context( this_rref_t rhv ) noexcept
{
    _display = rhv._display ;
    rhv._display = NULL ;

    _wnd = rhv._wnd ;
    rhv._wnd = 0 ;

    _context = rhv._context ;
    rhv._context = 0 ;

    motor_move_member_ptr( _bend_ctx, rhv ) ;
    _bend_ctx->change_owner( this ) ;
}

//****************************************************************
context::this_ref_t context::operator = ( this_rref_t rhv ) noexcept
{
    _display = rhv._display ;
    rhv._display = NULL ;

    _wnd = rhv._wnd ;
    rhv._wnd = 0 ;

    _context = rhv._context ;
    rhv._context = 0 ;
    
    motor_move_member_ptr( _bend_ctx, rhv ) ;
    _bend_ctx->change_owner( this ) ;

    return *this ;
}

//****************************************************************
context::~context( void_t ) noexcept
{
    this_t::deactivate() ;

    natus::memory::global_t::dealloc( _bend_ctx ) ;
}

//***************************************************************
natus::application::result context::activate( void_t ) noexcept
{
    //glXMakeCurrent( _display, _wnd, NULL ) ;
    //XLockDisplay( _display ) ;
    auto const res = glXMakeCurrent( _display, _wnd, _context ) ;
    //XUnlockDisplay( _display ) ;
    natus::log::global_t::warning( natus::core::is_not(res), 
            motor_log_fn( "glXMakeCurrent" ) ) ;

    return natus::application::result::ok ;
}

//***************************************************************
natus::application::result context::deactivate( void_t ) noexcept
{
    auto const res = glXMakeCurrent( _display, 0, 0 ) ;
    natus::log::global_t::warning( natus::core::is_not(res), 
            motor_log_fn( "glXMakeCurrent" ) ) ;
    return natus::application::result::ok ;
}

//***************************************************************
natus::application::result context::vsync( bool_t const on_off ) noexcept
{
    natus::ogl::glx::glXSwapInterval( _display, _wnd, on_off ? 1 : 0 ) ;
    return natus::application::result::ok ;
}

//**************************************************************
natus::application::result context::swap( void_t ) noexcept
{
    glXSwapBuffers( _display, _wnd ) ;
    const GLenum glerr = natus::ogl::glGetError( ) ;
    natus::log::global_t::warning( glerr != GL_NO_ERROR, 
            motor_log_fn( "glXSwapBuffers" ) ) ;
    return natus::application::result::ok ;
}

natus::graphics::backend_res_t context::create_backend( void_t ) noexcept 
{
    natus::application::gl_version glv ;
    this->get_gl_version( glv ) ;
    if( glv.major >= 3 )
    {
        return natus::graphics::gl4_backend_res_t(
            natus::graphics::gl4_backend_t( _bend_ctx ) ) ;
    }

    return natus::graphics::null_backend_res_t(
        natus::graphics::null_backend_t() ) ;
}

//**************************************************************
natus::application::result context::create_context( 
     Display* display, Window wnd, GLXContext context ) noexcept
{
    _display = display ;
    _wnd = wnd ;

    // the context comes in already create
    _context = context ;

    return natus::application::result::ok ;
}

//***************************************************************
natus::application::result context::is_extension_supported( 
    natus::ntd::string_cref_t extension_name ) noexcept
{
    this_t::strings_t ext_list ;
    if( natus::application::no_success( get_glx_extension(ext_list) ) ) 
        return natus::application::result::failed_wgl ;

    this_t::strings_t::iterator iter = ext_list.begin() ;
    while( iter != ext_list.end() )
    {
        if( *iter == extension_name ) 
            return natus::application::result::ok ;
        ++iter ;
    }
    return natus::application::result::invalid_extension ;
}

//*****************************************************************
natus::application::result context::get_glx_extension( this_t::strings_out_t /*ext_list*/ ) noexcept
{
    return natus::application::result::ok ;
}

//****************************************************************
natus::application::result context::get_gl_extension( this_t::strings_out_t ext_list ) noexcept
{
    const GLubyte * ch = natus::ogl::glGetString( GL_EXTENSIONS ) ;
    if( !ch ) return natus::application::result::failed ;

    natus::ntd::string_t extension_string( (const char*)ch) ;
    natus::ntd::string_ops::split( extension_string, ' ', ext_list ) ;
    return natus::application::result::ok ;
}

//****************************************************************
natus::application::result context::get_gl_version( natus::application::gl_version & version ) const noexcept
{
    const GLubyte* ch = natus::ogl::glGetString(GL_VERSION) ;
    if( !ch ) return natus::application::result::failed ;

    natus::ntd::string_t version_string = natus::ntd::string((const char*)ch) ;

    GLint major = 0;//boost::lexical_cast<GLint, std::string>(*token) ;
    GLint minor = 0;//boost::lexical_cast<GLint, std::string>(*(++token));

    {
        natus::ogl::glGetIntegerv( GL_MAJOR_VERSION, &major ) ;
        GLenum err = natus::ogl::glGetError() ;
        if( err != GL_NO_ERROR )
        {
            natus::ntd::string_t const es = ::std::to_string(err) ;
            natus::log::global::error( 
                "[context::get_gl_version] : get gl major <"+es+">" ) ;
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

//****************************************************************
void_t context::clear_now( natus::math::vec4f_t const & vec ) noexcept
{
    natus::ogl::glClearColor( vec.x(), vec.y(), vec.z(), vec.w() ) ;
    natus::ogl::glClear( GL_COLOR_BUFFER_BIT ) ;
    
    GLenum const gler = natus::ogl::glGetError() ;
    natus::log::global_t::error( gler != GL_NO_ERROR, "[context::clear_now] : glClear" ) ;
}

//***************************************************************
natus::application::result context::create_the_context( gl_info_cref_t gli ) noexcept
{
    auto res = natus::ogl::glx::init( _display, DefaultScreen( _display ) ) ;

    if( natus::log::global_t::error( natus::ogl::no_success(res), 
               "[glx_window::create_glx_window] : init glx") )
    {
        return natus::application::result::failed ;
    }

    int glx_major, glx_minor ;

    if( !glXQueryVersion( _display, &glx_major, &glx_minor ) ) 
    {
        natus::log::global_t::error( 
              "[glx_window::create_glx_window] : glXQueryVersion") ;
        return natus::application::result::failed ;
    }

    if( glx_major < 1 ) return natus::application::result::failed_glx ;
    if( glx_minor < 3 ) return natus::application::result::failed_glx ;

    // determine the GL version by creating a simple 1.0 context.
    natus::application::gl_version glv ;
    if( !this_t::determine_gl_version( glv ) )
    {
        natus::log::global_t::error( motor_log_fn(
           "failed to determine gl version ") ) ;
        return natus::application::result::failed ;
    }

    int context_attribs[] =
    {
     GLX_CONTEXT_MAJOR_VERSION_ARB, glv.major,
     GLX_CONTEXT_MINOR_VERSION_ARB, glv.minor,
     None
    } ;

    GLXContext context = natus::ogl::glx::glXCreateContextAttribs( 
          _display, natus::application::glx::window::get_config(), 
          0, True, context_attribs );

    if( natus::log::global_t::error( !context, 
           motor_log_fn( "glXCreateContextAttribs" )) ) 
    {
        return natus::application::result::failed ;
    }

    natus::ogl::gl::init() ;
    
    //this_t::activate() ;
    glXMakeCurrent( _display, _wnd, context ) ;
    {
        gl_version version ;
        if( !success( this_t::get_gl_version( version ) ) )
        {
            natus::log::global_t::error( motor_log_fn( "" ) ) ;
            this_t::deactivate() ;
            return result::failed_gfx_context_creation ;
        }
        natus::log::global_t::status( "GL Version: " +
           ::std::to_string(version.major) + "." + 
           ::std::to_string(version.minor) ) ;
    }

    {
        auto const res = this_t::vsync( gli.vsync_enabled ) ;
        natus::log::global_t::warning( natus::application::no_success(res),
               motor_log_fn("vsync") ) ;
    }
    glXMakeCurrent( _display, 0, 0 ) ;


    _context = context ;

    return natus::application::result::ok ;
}

//****************************************************************
bool_t context::determine_gl_version( gl_version & gl_out ) const noexcept
{
    int context_attribs[] =
    {
     GLX_CONTEXT_MAJOR_VERSION_ARB, 1,//gli.version.major,
     GLX_CONTEXT_MINOR_VERSION_ARB, 0,//gli.version.minor,
     None
    } ;

    GLXContext context = natus::ogl::glx::glXCreateContextAttribs( 
          _display, natus::application::glx::window::get_config(), 
          0, True, context_attribs );

    if( natus::log::global_t::error( !context, 
           motor_log_fn( "glXCreateContextAttribs") ) ) 
    {
        return false ;
    }

    natus::ogl::gl::init() ;

    gl_version version ;
    glXMakeCurrent( _display, _wnd, context ) ;
    {
        if( !success( this_t::get_gl_version( version ) ) )
        {
            natus::log::global_t::error( motor_log_fn( "" ) ) ;
            glXMakeCurrent( _display, 0, 0 ) ;
            glXDestroyContext( _display, context ) ;
            return false ;
        }
        natus::log::global_t::status( "[determine_gl_version] : GL Version: " +
           ::std::to_string(version.major) + "." + 
           ::std::to_string(version.minor) ) ;
    }
    glXMakeCurrent( _display, 0, 0 ) ;
    glXDestroyContext( _display, context ) ;

    gl_out = version ;
    return true ;
}
