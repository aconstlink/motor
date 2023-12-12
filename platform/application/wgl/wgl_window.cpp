
#include "wgl_window.h"
#include "wgl_context.h"

#include <natus/log/global.h>

using namespace natus::application ;
using namespace natus::application::wgl ;

//***********************************************************************
window::window( void_t ) 
{}

//***********************************************************************
window::window( gl_info_cref_t gli, window_info_cref_t wi ) 
{
    auto wii = wi ;
    wii.window_name = "[wgl] " + wi.window_name ;
    _window = win32::window( wii ) ;
    _context = wgl::context_t( _window->get_handle() ) ;

    _vsync = gli.vsync_enabled ;

    // give it a test
    {
        _context->activate() ;
        _context->clear_now( natus::math::vec4f_t(0.5f,0.5f,1.0f,1.0f) ) ;
        _context->swap() ;
        _context->clear_now( natus::math::vec4f_t(0.5f,0.5f,1.0f,1.0f) ) ;
        _context->swap() ;
        _context->deactivate() ;
    }
}

//***********************************************************************
window::window( this_rref_t rhv ) : platform_window( ::std::move( rhv ) )
{
    _window = ::std::move( rhv._window ) ;
    _context = ::std::move( rhv._context ) ;

    _vsync = rhv._vsync ;
}

//***********************************************************************
window::~window( void_t ) 
{
    _window = win32::window_res_t() ;
    _context = wgl::context_res_t() ;
}

//***********************************************************************
void_t window::check_for_messages( void_t ) noexcept 
{
    _window->check_for_messages() ;
}
