#include "xlib_carrier.h"
#include "xlib_window.h"

#include <motor/device/global.h>
#include <motor/log/global.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

using namespace motor::platform ;
using namespace motor::platform::xlib ;

Display * xlib_carrier::_display = NULL ;
size_t xlib_carrier::_display_use_count = 0 ;

//**********************************************************************
Display * xlib_carrier::connect_display( void_t ) noexcept
{
    if( _display_use_count++ > 0 )
    {
        return _display ;
    }

    _display = XOpenDisplay( NULL ) ;

    if( motor::log::global_t::error( 
         _display==NULL, "[xlib_carrier] : XOpenDisplay" ) )
    {
        exit(0) ;
    }

    return _display ;
}

//**********************************************************************
void_t xlib_carrier::disconnect_display( void_t ) noexcept
{
    if( _display_use_count-- == 1 )
    {
        XCloseDisplay( _display ) ;
        _display = NULL ;
    }
}

//**********************************************************************
Display * xlib_carrier::move_display( void_t ) noexcept
{
    ++_display_use_count ;
    return _display ;
}

//**********************************************************************
xlib_carrier::xlib_carrier( void_t ) noexcept
{
    connect_display() ;

    //_device_module = motor::device::xlib::xlib_module_t() ;
    //motor::device::global_t::system()->add_module( _device_module ) ;
}

//***********************************************************************
xlib_carrier::xlib_carrier( motor::application::iapp_mtr_shared_t /*app*/ ) noexcept
{
    connect_display() ;

    //_device_module = motor::device::xlib::xlib_module_t() ;
    //motor::device::global_t::system()->add_module( _device_module ) ;
}
//**********************************************************************
Display * xlib_carrier::get_display( void_t ) noexcept
{
    return this_t::connect_display()  ;
}

//**********************************************************************
xlib_carrier::xlib_carrier( this_rref_t /*rhv*/ ) noexcept
{
    this_t::move_display() ;
    //_device_module = std::move( rhv._device_module ) ;
}

//**********************************************************************
xlib_carrier::~xlib_carrier( void_t ) noexcept
{
    this_t::disconnect_display() ;
}

//********************************************************************
motor::application::result xlib_carrier::on_exec( void_t ) noexcept
{
    int_t ret = XInitThreads() ;
    if( motor::log::global_t::error( 
           ret == 0, "[xlib_carrier] : XInitThreads" ) )
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

#if 0
                auto * wnd_ptr = motor::application::xlib::window_ptr_t(
                                        *(this_ptr_t*)stored_data) ;
                wnd_ptr->xevent_callback( event ) ;
                #endif

            }
#if 0
            _device_module->handle_input_event( event ) ;
#endif
            switch( event.type )
            {
            case Expose:
                //motor::log::global_t::status("application expose") ;
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

    return motor::application::result::ok ;
}

//********************************************************************
motor::application::result xlib_carrier::close( void_t ) noexcept
{
    return motor::application::result::failed ;
}

//********************************************************************
motor::application::iwindow_mtr_shared_t xlib_carrier::create_window( motor::application::window_info_cref_t /*info*/ ) noexcept  
{
    return motor::application::iwindow_mtr_shared_t::make() ;
}

//********************************************************************
motor::application::iwindow_mtr_shared_t xlib_carrier::create_window( motor::application::graphics_window_info_in_t /*info*/ ) noexcept 
{
    return motor::application::iwindow_mtr_shared_t::make() ;
}
