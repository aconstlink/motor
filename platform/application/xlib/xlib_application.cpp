#include "xlib_application.h"
#include "xlib_window.h"

#include <natus/device/global.h>
#include <natus/log/global.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

using namespace natus::application ;
using namespace natus::application::xlib ;

Display * xlib_application::_display = NULL ;
size_t xlib_application::_display_use_count = 0 ;

//**********************************************************************
Display * xlib_application::connect_display( void_t ) 
{
    if( _display_use_count++ > 0 )
    {
        return _display ;
    }

    _display = XOpenDisplay( NULL ) ;

    if( natus::log::global_t::error( 
         _display==NULL, "[xlib_application] : XOpenDisplay" ) )
    {
        exit(0) ;
    }

    return _display ;
}

//**********************************************************************
void_t xlib_application::disconnect_display( void_t ) 
{
    if( _display_use_count-- == 1 )
    {
        XCloseDisplay( _display ) ;
        _display = NULL ;
    }
}

//**********************************************************************
Display * xlib_application::move_display( void_t ) 
{
    ++_display_use_count ;
    return _display ;
}

//**********************************************************************
xlib_application::xlib_application( void_t ) 
{
    connect_display() ;

    _device_module = natus::device::xlib::xlib_module_t() ;
    natus::device::global_t::system()->add_module( _device_module ) ;
}

//***********************************************************************
xlib_application::xlib_application( natus::application::app_res_t app ) : base_t( app )
{
    connect_display() ;

    _device_module = natus::device::xlib::xlib_module_t() ;
    natus::device::global_t::system()->add_module( _device_module ) ;
}
//**********************************************************************
Display * xlib_application::get_display( void_t ) 
{
    return this_t::connect_display()  ;
}

//**********************************************************************
xlib_application::xlib_application( this_rref_t rhv ) : base_t( ::std::move( rhv ) )
{
    this_t::move_display() ;
    _device_module = ::std::move( rhv._device_module ) ;
}

//**********************************************************************
xlib_application::~xlib_application( void_t )
{
    this_t::disconnect_display() ;
}

//***********************************************************************
xlib_application::this_ptr_t xlib_application::create( void_t ) 
{
    return this_t::create( this_t() ) ;
}

//***********************************************************************
xlib_application::this_ptr_t xlib_application::create( this_rref_t rhv )
{
    return natus::memory::global_t::alloc( ::std::move( rhv ), "[xlib_application::create]" ) ;
}

//********************************************************************
void_t xlib_application::destroy( this_ptr_t ptr ) 
{
    natus::memory::global_t::dealloc( ptr ) ;
}

//********************************************************************
natus::application::result xlib_application::on_exec( void_t )
{
    int_t ret = XInitThreads() ;
    if( natus::log::global_t::error( 
           ret == 0, "[xlib_application] : XInitThreads" ) )
        {
            exit(ret) ;
        }

    Display * display = this_t::get_display() ;

    XEvent event ;

    bool_t run = true ;
    while( run )
    {
        int_t num_events = 0 ;
        //bool_t pressed = false ;

        while( (num_events = XPending(display)) != 0 )
        {
            XNextEvent( display, &event ) ;

            {
                Atom a = XInternAtom( event.xany.display, "wnd_ptr_value", false ) ;
                Atom at = XInternAtom( event.xany.display, "wnd_ptr_type", false ) ;
                Atom at_ret ;
                int format_out ;
                unsigned long len_out, bytes_after ;

                uchar_ptr_t stored_data ;
                XGetWindowProperty( event.xany.display, 
                    event.xany.window, a, 0, sizeof(void_ptr_t), false,
                    at, &at_ret, &format_out, &len_out, 
                    &bytes_after, (uchar_ptr_t*)&stored_data ) ;

                auto * wnd_ptr = natus::application::xlib::window_ptr_t(
                                        *(this_ptr_t*)stored_data) ;
                wnd_ptr->xevent_callback( event ) ;

            }

            _device_module->handle_input_event( event ) ;

            switch( event.type )
            {
            case Expose:
                //natus::log::global_t::status("application expose") ;
                break ;

            case DestroyNotify:
                break ;

            case UnmapNotify:
                break ;

            case ClientMessage:
                {
                    run = false ;
                }
                break ;
            }
        }
    }

    return natus::application::result::ok ;
}



