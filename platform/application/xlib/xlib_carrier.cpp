#include "xlib_carrier.h"

#include <motor/device/global.h>
#include <motor/log/global.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

using namespace motor::platform ;
using namespace motor::platform::xlib ;

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
xlib_carrier::xlib_carrier( motor::application::iapp_mtr_shared_t /*app*/ ) noexcept : xlib_carrier()
{
}

//***********************************************************************
xlib_carrier::xlib_carrier( motor::application::iapp_mtr_unique_t /*app*/ ) noexcept : xlib_carrier()
{
}

//**********************************************************************
xlib_carrier::xlib_carrier( this_rref_t rhv ) noexcept
{
    this_t::move_display() ;
    //_device_module = std::move( rhv._device_module ) ;
    _xlib_windows = std::move( rhv._xlib_windows ) ;
    _destroy_queue = std::move( rhv._destroy_queue ) ;
    _display = motor::move( rhv._display ) ;
    _display_use_count = rhv._display_use_count ;
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
    if( motor::log::global_t::error( ret == 0, 
        "[xlib_carrier] : XInitThreads" ) )
    {
        exit(ret) ;
    }

    while( !_done )
    {
        int_t num_events = 0 ;

        while( (num_events = XPending(_display)) != 0 )
        {
            for( auto & wd : _xlib_windows )
            {
                XEvent event ;
                if( XCheckWindowEvent( _display, wd.hwnd, ExposureMask | VisibilityChangeMask | StructureNotifyMask | 
                    SubstructureRedirectMask | ResizeRedirectMask , &event ) )
                {
                    motor::log::global_t::status( "[message]" ) ;
                    switch( event.type )
                    {
                    case Expose:
                        motor::log::global_t::status("application expose") ;
                        break ;

                    case DestroyNotify:
                    {
                        int bp = 0 ;
                    }
                        break ;

                    case UnmapNotify:
                        break ;

                    
                    case ResizeRequest:
                    {
                        int bp = 0 ;
                    }
                        break ;
                    
                    }
                }
            }

            // handling close message
            {
                XEvent event ;
                XNextEvent( _display, &event ) ;
                switch( event.type )
                {
                case ClientMessage:
                {
                    auto const & e = (XClientMessageEvent const &)event ;
                    if( e.data.l[0] == XInternAtom( _display, "WM_DELETE_WINDOW", True ) )
                    {
                        _destroy_queue.emplace_back( e.window ) ;
                    }
                }
                break ;
                }
            }
            //XSync( _display, True ) ;
        }

        // test window destruction
        // also do 
        // -> wgl context destruction
        {
            for( Window hwnd : _destroy_queue )
            {
                this_t::handle_destroyed_hwnd( hwnd ) ;
            }
            _destroy_queue.clear() ;
        }

        motor::log::global_t::status( "[after]" ) ;
        
        // test window queue for creation
        {
            std::lock_guard< std::mutex > lk ( _mtx_queue ) ;
            for( auto & d : _queue )
            {
                size_t const wnd_idx = _xlib_windows.size() ;
                Window hwnd = this_t::create_xlib_window( d.wi ) ;

                {
                    xlib_window_data wd ;
                    wd.hwnd = hwnd ;
                    wd.wnd = d.wnd ;
                    wd.lsn = d.lsn ;
                    wd.window_text = d.wi.window_name ;

                    _xlib_windows.emplace_back(wd);
                }

                // deduce auto graphics api type
                if( d.wi.gen == motor::application::graphics_generation::gen4_auto ) 
                {
                    #if MOTOR_GRAPHICS_DIRECT3D
                    d.wi.gen = motor::application::graphics_generation::gen4_d3d11 ;
                    #elif MOTOR_GRAPHICS_WGL
                    d.wi.gen = motor::application::graphics_generation::gen4_gl4 ;
                    #else
                    d.wi.gen = motor::application::graphics_generation:: none ;
                    #endif
                }

                #if MOTOR_GRAPHICS_WGL
                if( d.wi.gen == motor::application::graphics_generation::gen4_gl4 )
                {
                    _win32_windows.back().window_text = " [gl4 #" + motor::to_string(wnd_idx) +"]";

                    motor::platform::wgl::wgl_context_t ctx ;

                    auto const res = ctx.create_context( hwnd ) ;
                    if( motor::platform::success( res ) )
                    {
                        ctx.activate() ;
                        ctx.clear_now( motor::math::vec4f_t(0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                        ctx.swap() ;
                        ctx.clear_now( motor::math::vec4f_t(0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                        ctx.swap() ;
                        ctx.deactivate() ;

                        this_t::wgl_pimpl * pimpl = motor::memory::global_t::alloc(
                            this_t::wgl_pimpl( { std::move(ctx) } ), "[win32_carrier] : wgl context") ;

                            pimpl->fe = motor::memory::global_t::alloc( motor::graphics::gen4::frontend_t( &pimpl->re, pimpl->ctx.backend() ),
                                "[carrier32] : gen4 frontend") ;
                            
                            _win32_windows.back().wnd->set_renderable( &pimpl->re, pimpl->fe ) ;

                        _wgl_windows.emplace_back( wgl_window_data({ hwnd, pimpl }) )  ;
                    }
                    else
                    {
                        motor::log::global_t::critical( "Wanted to create a WGL window but could not." ) ;
                    }
                }
                #endif

                if( d.wi.gen == motor::application::graphics_generation::none )
                {
                    motor::log::global_t::error( "Wanted to create a graphics window but no api chosen or available." ) ;
                }

                //this_t::send_create( _xlib_windows.back() ) ;
            }
            _queue.clear() ;
        }
    }

    for( auto const & d : _xlib_windows )
    {
        this_t::handle_destroyed_hwnd( d.hwnd ) ;
    }

    return motor::application::result::ok ;
}

//********************************************************************
motor::application::result xlib_carrier::close( void_t ) noexcept
{
    _done = true ;
    return motor::application::result::ok ;
}

//********************************************************************
motor::application::iwindow_mtr_shared_t xlib_carrier::create_window( motor::application::window_info_cref_t info ) noexcept  
{
    motor::application::window_mtr_t wnd = motor::memory::create_ptr<motor::application::window_t>(
        "[xlib_carrier] : window handle" ) ;

    motor::application::window_message_listener_mtr_t lsn = motor::memory::create_ptr<
        motor::application::window_message_listener_t>("[xlib_carrier] : window message listener") ;

    wnd->register_in( motor::share( lsn ) ) ;

    {
        std::lock_guard< std::mutex > lk( _mtx_queue ) ;
        _queue.emplace_back( this_t::window_queue_msg_t{info, wnd, lsn} ) ;
    }

    return motor::share( wnd )  ;
}

//********************************************************************
Window xlib_carrier::create_xlib_window( motor::application::window_info_cref_t wi ) noexcept
{
    motor::application::window_info wil = wi ;

    Window root = DefaultRootWindow( _display ) ;

    int start_x = wi.x ; 
    int start_y = wi.y ; 
    int width = wi.w ; 
    int height = wi.h ; 

    //_dims = motor::math::vec4i_t( start_x, start_y, width, height ) ;

    //ShowCursor( wil.show_cursor ) ;


    if( wil.fullscreen )
    {
        //motor::application::toggle_window_t tw ;
        //tw.toggle_fullscreen = wil.fullscreen ;
        //this_t::send_toggle(tw) ;
    }
    else
    {
    }

    Window wnd = XCreateSimpleWindow( 
            _display, root, 
            start_x, start_y, width, height, 1, 
            XBlackPixel(_display,0), 
            XWhitePixel(_display,0) ) ;
    
    
    if( motor::log::global_t::error( wnd == BadAlloc, 
            "[xlib_carrier::create_window] : XCreateSimpleWindow - BadAlloc" ) )
    {
        return wnd ;
    }
    else if( motor::log::global_t::error( wnd == BadValue, 
            "[xlib_carrier::create_window] : XCreateSimpleWindow - BadValue" ) )
    {
        return wnd ;
    }

    XSelectInput( _display, wnd, 
        ExposureMask | StructureNotifyMask | ResizeRedirectMask | SubstructureRedirectMask /*| KeyPressMask | KeyReleaseMask | 
        PointerMotionMask | ButtonPressMask | ButtonReleaseMask | 
        | VisibilityChangeMask*/
        //| ResizeRedirectMask 
        ) ;

    // prepare per window data
    //this_t::store_this_ptr_in_atom( display, wnd ) ;

    /// setup client message for closing a window
    {
        Atom del = XInternAtom( _display, "WM_DELETE_WINDOW", False ) ;
        XSetWMProtocols( _display, wnd, &del, 1 ) ;
    }

    //_handle = wnd ;
    //_display = display ;
    
    if( wi.show )
    {
        XMapWindow( _display, wnd );
        XFlush( _display ) ;
    }

    return wnd ;
}

//*******************************************************************************************
void_t xlib_carrier::handle_destroyed_hwnd( Window hwnd ) noexcept 
{
    auto iter = std::find_if( _xlib_windows.begin(), _xlib_windows.end(), [&]( xlib_window_data_cref_t d )
    {
        return d.hwnd == hwnd ;
    } ) ;

    if( iter == _xlib_windows.end() ) return ;

    #if MOTOR_GRAPHICS_WGL
    // look for glx windows/context connection
    // and remove those along with the window
    {
        auto iter2 = std::find_if( _wgl_windows.begin(), _wgl_windows.end(), [&]( wgl_window_data_cref_t d )
        {
            return d.hwnd == hwnd ;
        } ) ;

        if( iter2 != _wgl_windows.end() )
        {
            motor::memory::global_t::dealloc( iter2->ptr->fe ) ;
            motor::memory::global_t::dealloc( iter2->ptr ) ;
            _wgl_windows.erase( iter2 ) ;
        }
    }
    #endif 

    this_t::send_destroy( *iter ) ;

    XDestroyWindow( _display, iter->hwnd ) ;

    motor::memory::release_ptr( iter->wnd ) ;
    motor::memory::release_ptr( iter->lsn ) ;
    _xlib_windows.erase( iter ) ;
}

//*******************************************************************************************
void_t xlib_carrier::send_destroy( xlib_window_data_in_t d ) noexcept 
{
    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( motor::application::close_message_t{true} ) ;
    } ) ;
}