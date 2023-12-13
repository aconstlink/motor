
#include "xlib_application.h"
#include "xlib_window.h"

#include <motor/log/global.h>

#include <algorithm>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

using namespace motor::application ;
using namespace motor::application::xlib ;

//*****************************************************************
window::window( void_t ) 
{}

//*****************************************************************
window::window( window_info const & info ) 
{
    this_t::create_window( info ) ;
}

//*****************************************************************
window::window( Display * display, Window wnd ) 
{
    this_t::create_window( display, wnd ) ;
}

//****************************************************************
window::window( this_rref_t rhv ) : platform_window( ::std::move( rhv ) )
{
    _display = rhv._display ;
    rhv._display = nullptr ;

    _handle = rhv._handle ;
    rhv._handle = 0 ;

    this_t::store_this_ptr_in_atom( 
        _display, _handle ) ;

    _dims = rhv._dims ;
}

//***************************************************************
window::~window( void_t ) 
{
    this_t::destroy_window() ;
}

//***************************************************************
Window window::get_handle( void_t ) 
{
    return _handle ;
}

//***************************************************************
Display * window::get_display( void_t ) 
{
    return _display ;
}

//***************************************************************
void_t window::create_window( window_info const & wi ) 
{
    auto const status = XInitThreads() ;
    natus::log::global_t::warning( status == 0, 
           motor_log_fn("XInitThreads") ) ;

    window_info wil = wi ;

    Display * display = natus::application::xlib::xlib_application::get_display() ;

    Window root = DefaultRootWindow( display ) ;

    int start_x = wi.x ; 
    int start_y = wi.y ; 
    int width = wi.w ; 
    int height = wi.h ; 

    _dims = natus::math::vec4i_t( start_x, start_y, width, height ) ;

    //ShowCursor( wil.show_cursor ) ;


    if( wil.fullscreen )
    {
        natus::application::toggle_window_t tw ;
        tw.toggle_fullscreen = wil.fullscreen ;
        this_t::send_toggle(tw) ;
    }
    else
    {
    }

    Window wnd = XCreateSimpleWindow( 
            display, root, 
            start_x, start_y, width, height, 1, 
            XBlackPixel(display,0), 
            XWhitePixel(display,0) ) ;
    
    if( natus::log::global_t::error( wnd == BadAlloc, 
            "[window::create_window] : XCreateSimpleWindow - BadAlloc" ) ){
        return ;
    }
    else if( natus::log::global_t::error( wnd == BadValue, 
            "[window::create_window] : XCreateSimpleWindow - BadValue" ) ){
        return ;
    }

    this_t::create_window( display, wnd ) ;
    
    if( wi.show )
    {
        XMapWindow( display, wnd );
        XFlush( display ) ;
    }
}

//***************************************************************
void_t window::create_window( Display * display, Window wnd ) 
{
    XSelectInput( display, wnd, 
                  ExposureMask | KeyPressMask | KeyReleaseMask | 
                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask | 
                  StructureNotifyMask | VisibilityChangeMask
                  //| ResizeRedirectMask 
                  ) ;

    // prepare per window data
    this_t::store_this_ptr_in_atom( display, wnd ) ;

    /// setup client message for closing a window
    {
        Atom del = XInternAtom( display, "WM_DELETE_WINDOW", false ) ;
        XSetWMProtocols( display, wnd, &del, 1 ) ;
    }

    _handle = wnd ;
    _display = display ;
}

//**************************************************************
void_t window::store_this_ptr_in_atom( Display * display, Window wnd ) 
{
    Atom a = XInternAtom( display, "wnd_ptr_value", false ) ;
    Atom at = XInternAtom( display, "wnd_ptr_type", false ) ; 
    int format_in = 8 ; // 8bit
    int const len_in = sizeof(this_ptr_t) ;

    // store the data in the per window memory
    {
        this_ptr_t data = this ;
        XChangeProperty( display, wnd, a, at, 
          format_in, PropModeReplace, (uchar_ptr_t)&data, len_in ) ;
    }

    // test the stored data
    {
        Atom at_ret ;
        int format_out ;
        unsigned long len_out, bytes_after ;

        uchar_ptr_t stored_data ;
        XGetWindowProperty( display, wnd, a, 0, 
           sizeof(void_ptr_t), false, at, &at_ret, 
           &format_out, &len_out, &bytes_after, 
           (uchar_ptr_t*)&stored_data ) ;

        this_ptr_t test_ptr = this_ptr_t(*(this_ptr_t*)stored_data) ;

        motor_assert( len_out == len_in ) ;
        motor_assert( format_out == format_in ) ;
        motor_assert( test_ptr == this ) ;
    }
}

//*****************************************************************
void_t window::destroy_window( void_t ) 
{
    //if( _display != NULL && _handle != NULL )
        //XDestroyWindow( _display, _handle ) ;
}

//****************************************************************
void_t window::xevent_callback( XEvent const & event ) 
{
    //natus::log::global_t::status( "[window::xevent_callback]" ) ;

    switch( event.type )
    {
    case Expose:
        //XClearWindow( event.xany.display, event.xany.window ) ;
        //natus::log::global_t::status("expose") ;
        // check window listener 
        this_t::foreach_out( [&] ( natus::application::window_message_receiver_res_t lst )
        { 
            natus::application::window_message_receiver_t::state_vector_t states ;
            if( lst->swap_and_reset(states) )
            {
                if( states.fulls_msg_changed)
                {
                    natus::application::toggle_window_t tw ;
                    tw.toggle_fullscreen = states.fulls_msg.on_off ;
                    this_t::send_toggle( tw ) ;
                }
            }
        } ) ;
        break ;

    case VisibilityNotify:
        natus::log::global_t::status("visibility") ;
        break ;

    case ConfigureNotify:
    {
        XConfigureEvent xce = event.xconfigure ;
        natus::application::resize_message const rm 
        {
         true,
         0,0,
         true,
         (size_t)xce.width, (size_t)xce.height
        } ;

        this_t::foreach_in( [&]( natus::application::iwindow_message_listener_res_t lst )
        { 
            lst->on_resize( rm ) ; 
        } ) ;

        if( natus::core::is_not( _is_fullscreen ) )
        {
            _dims = natus::math::vec4i_t( rm.x, rm.y, rm.w, rm.h ) ;
        }
        break ;
    }

    case ResizeRequest:
        {
            natus::log::global_t::status("window resize") ;
            XWindowAttributes attr ;
            XGetWindowAttributes( event.xany.display, 
                event.xany.window, &attr ) ;

            XResizeRequestEvent const & rse = (XResizeRequestEvent const &) event ;
            natus::application::resize_message const rm {
                true, attr.x, attr.y, true,
                (size_t)rse.width, (size_t)rse.height
            } ;

            this_t::foreach_in( [&]( natus::application::iwindow_message_listener_res_t lst )
            { 
                lst->on_resize( rm ) ; 
            } ) ;
            XClearArea( event.xany.display, event.xany.window, 0,0,0,0, true ) ;

            if( natus::core::is_not( _is_fullscreen ) )
            {
                _dims = natus::math::vec4i_t( rm.x, rm.y, rm.w, rm.h ) ;
            }
        }
        break ;
    }
}

//***
void_t window::show_window(  window_info const & wi ) 
{
    if( wi.show ) 
    {
        XMapWindow( _display, _handle ) ;
    }
    else 
    {
        XUnmapWindow( _display, _handle ) ;
    }
}

//***************************************************************
void_t window::send_toggle( natus::application::toggle_window_in_t di ) 
{
    if( _is_fullscreen != di.toggle_fullscreen )
    {
        _is_fullscreen = di.toggle_fullscreen ;

        natus::math::vec4i_t dims = _dims ;

        if( _is_fullscreen ) 
        {
            dims = natus::math::vec4i_t( 0, 0, 
                 XDisplayWidth( _display, 0 ),
                 XDisplayHeight( _display, 0 ) ) ;

            XSetWindowBorderWidth( _display, _handle, 0 ) ;
        }

        uint_t const mask = CWX | CWY | CWWidth | CWHeight | CWBorderWidth | CWStackMode ;

        XWindowChanges wc ;
        wc.x = dims.x() ;
        wc.y = dims.y() ;
        wc.width = dims.z() ;
        wc.height = dims.w() ;
        wc.border_width = 0 ;
        wc.stack_mode = Above ;
        XConfigureWindow( _display, _handle, mask, &wc ) ;

        

        
#if 0 // does not work
        XEvent xev ;
        ::std::memset( &xev, 0, sizeof(xev) ) ;
        xev.type = ClientMessage ;
        xev.xclient.window = _handle ;
        xev.xclient.message_type = XInternAtom( _display, "_NET_WM_STATE", False ) ;
        xev.xclient.format = 32 ;
        xev.xclient.data.l[0] = 1 ;
        xev.xclient.data.l[1] = XInternAtom( _display, "_NET_WM_STATE_FULLSCREEN", False ) ;
        xev.xclient.data.l[2] = 0 ;
        XSendEvent( _display, _handle, False, SubstructureNotifyMask | SubstructureRedirectMask, &xev ) ;
#endif
#if 0 // does not work
        Atom wm_state = XInternAtom( _display, "_NET_WM_STATE", False ) ;
        Atom wm_fulls = XInternAtom( _display, "_NET_WM_STATE_FULLSCREEN", False ) ;
        XChangeProperty( _display, _handle, wm_state, XA_ATOM, 32, PropModeReplace, 
                         (unsigned char *)&wm_fulls, 1 ) ;
#endif
    }

}

//***
void_t window::check_for_messages( void_t ) noexcept 
{
    XEvent ev ;
    ::std::memset( &ev, 0, sizeof(ev) ) ;

    ev.type = Expose ;
    ev.xexpose.window = _handle ;
    XSendEvent( _display, _handle, False, ExposureMask, &ev ) ;
    XFlush(_display) ;
}
