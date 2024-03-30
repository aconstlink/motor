#include "xlib_carrier.h"

#include <motor/graphics/render_engine.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>



#include <motor/log/global.h>
#include <motor/concurrent/global.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#if MOTOR_GRAPHICS_GLX
#include "../glx/glx_context.h"
#include <motor/ogl/glx/glx.h>
#endif

#if MOTOR_GRAPHICS_EGL
#include "../egl/egl_context.h"
#include <motor/ogl/egl/egl.h>
#endif

using namespace motor::platform ;
using namespace motor::platform::xlib ;

#if MOTOR_GRAPHICS_GLX
struct xlib_carrier::glx_pimpl
{
    motor::platform::glx::glx_context_t ctx ;
    motor::graphics::render_engine_t re ;
    motor::graphics::ifrontend_ptr_t fe ;

    static GLXFBConfig get_config( Display  * display ) noexcept
    {
        static GLXFBConfig *fbc = nullptr ;
        if( fbc != nullptr )
        {
            return fbc[0] ;
        }

        int_ptr_t visual_attribs = motor::memory::global_t::alloc_raw<int_t>( 24, 
                "[glx_window::create_glx_window] : visual_attribs" ) ;

        {
            struct va_pair{
                int_t flag ;
                int_t value ;
            };

            va_pair * va_pairs = (va_pair*)visual_attribs ;
            va_pairs[0] = {GLX_X_RENDERABLE, True} ;
            va_pairs[1] = {GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT} ;
            va_pairs[2] = {GLX_RENDER_TYPE, GLX_RGBA_BIT} ;
            va_pairs[3] = {GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR} ;
            va_pairs[4] = {GLX_RED_SIZE, 8} ;
            va_pairs[5] = {GLX_GREEN_SIZE, 8} ;
            va_pairs[6] = {GLX_BLUE_SIZE, 8} ;
            va_pairs[7] = {GLX_ALPHA_SIZE, 8} ;
            va_pairs[8] = {GLX_DEPTH_SIZE, 24} ;
            va_pairs[9] = {GLX_STENCIL_SIZE, 8} ;
            va_pairs[10] = {GLX_DOUBLEBUFFER, True} ;
            va_pairs[11] = {None, None} ;
        }

        int fbcount ;
        fbc = glXChooseFBConfig( 
                display, DefaultScreen( display ),
                visual_attribs, &fbcount ) ;

        if( fbc == nullptr || fbcount == 0 ) 
        {
            motor::log::global_t::error( 
                "[glx_window::create_glx_window] : glXChooseFBConfig" ) ;

            return 0 ;
        }

        GLXFBConfig fbconfig = fbc[0] ;

        //XFree( fbc ) ;
        //natus::memory::global_t::dealloc( visual_attribs ) ;

        return fbconfig ;
    }
} ;
#endif

#if MOTOR_GRAPHICS_EGL
struct xlib_carrier::egl_pimpl
{
    motor::platform::egl::egl_context_t ctx ;
    motor::graphics::render_engine_t re ;
    motor::graphics::ifrontend_ptr_t fe ;

} ;
#endif

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

    this_t::create_and_register_device_modules() ;    
}

//***********************************************************************
xlib_carrier::xlib_carrier( motor::application::app_mtr_safe_t app ) noexcept : base_t( std::move( app ) )
{
    connect_display() ;
    this_t::create_and_register_device_modules() ;
}

//**********************************************************************
xlib_carrier::xlib_carrier( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
{
    this_t::move_display() ;
    _device_module = motor::move( rhv._device_module ) ;
    _xlib_windows = std::move( rhv._xlib_windows ) ;
    _destroy_queue = std::move( rhv._destroy_queue ) ;
    _display = motor::move( rhv._display ) ;
    _display_use_count = rhv._display_use_count ;
    rhv._display_use_count = 0 ;
}

//**********************************************************************
xlib_carrier::~xlib_carrier( void_t ) noexcept
{
    this_t::disconnect_display() ;
    motor::memory::release_ptr( _device_module ) ;
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

    using _clock_t = std::chrono::high_resolution_clock ;
    _clock_t::time_point tp_begin = _clock_t::now() ;

    while( !_done )
    {
        // should be called in the main carrier!
        // but that requires a different handling of the while loop.
        motor::concurrent::global_t::update() ;

        _clock_t::duration const dur = _clock_t::now() - tp_begin ;
        tp_begin = _clock_t::now() ;

        size_t const milli = std::chrono::duration_cast< std::chrono::milliseconds >( dur ).count() ;

        int_t num_events = 0 ;

        while( (num_events = XPending(_display)) != 0 )
        {
            for( auto & wd : _xlib_windows )
            {
                XEvent event ;
                if( XCheckWindowEvent( _display, wd.hwnd, KeyPressMask | KeyReleaseMask | 
                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | VisibilityChangeMask | StructureNotifyMask | 
                    SubstructureRedirectMask | ResizeRedirectMask , &event ) )
                {
                    motor::log::global_t::status( "[message]" ) ;
                    switch( event.type )
                    {
                    case Expose:
                        motor::log::global_t::status("application expose") ;
                        break ;

                    case DestroyNotify:
                        break ;

                    case UnmapNotify:
                        break ;

                    
                    case ResizeRequest:
                    {
                        XResizeRequestEvent evt = event.xresizerequest ;
                        wd.width = evt.width ;
                        wd.height = evt.height ;
                        this_t::send_resize( wd ) ;
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
                _device_module->handle_input_event( event ) ;
            }
            //XSync( _display, True ) ;
        }

        // test window destruction
        // also do 
        // -> glx context destruction
        {
            for( auto iter=_destroy_queue.begin(); iter != _destroy_queue.end(); )
            {
                // have to wait until all users of the window are ready. 
                if( this_t::handle_destroyed_hwnd( *iter ) )
                {
                    iter = _destroy_queue.erase( iter ) ; continue ;
                }
                ++iter ;
            }
        }

        #if MOTOR_GRAPHICS_GLX

        // update glx window context
        // must be done here. If the window is closed,
        // the window handle is not valid anymore and must be
        // destructed in this class first.
        {
            for( auto & d : _glx_windows )
            {
                // could be threaded
                {
                    if( d.ptr->re.can_execute() )
                    {
                        {
                            auto iter = std::find_if( _xlib_windows.begin(), _xlib_windows.end(), [&]( xlib_window_data const & wd )
                            {
                                return wd.hwnd == d.hwnd ;
                            } ) ;

                            d.ptr->ctx.borrow_backend()->set_window_info( {
                                size_t(iter->width), size_t(iter->height) } ) ;
                        }

                        d.ptr->ctx.activate() ;
                        d.ptr->ctx.borrow_backend()->render_begin() ;
                        d.ptr->re.execute_frame() ;
                        d.ptr->ctx.borrow_backend()->render_end() ;
                        d.ptr->ctx.swap() ;
                        d.ptr->ctx.deactivate() ;
                    }
                    // required for now, otherwise the application stalls.
                    else if( milli == 0 )
                    {
                        std::this_thread::sleep_for( std::chrono::milliseconds(1) ) ;
                    }
                }

                this_t::find_window_info( d.hwnd, [&]( this_t::xlib_window_data_ref_t wd )
                {
                    // set frame time
                    {
                        XStoreName( _display, wd.hwnd, ( wd.window_text + " [" + motor::to_string(milli) + " ms]").c_str() ) ;
                    }

                    // set vsync
                    {
                        if( wd.sv.vsync_msg_changed ) 
                        {
                            d.ptr->ctx.vsync( wd.sv.vsync_msg.on_off ) ;
                            wd.sv.vsync_msg_changed = false ;
                        }
                    }
                } )  ;
            }
        }

        #endif

        #if MOTOR_GRAPHICS_EGL

        // update egl window context
        // must be done here. If the window is closed,
        // the window handle is not valid anymore and must be
        // destructed in this class first.
        {
            for( auto & d : _egl_windows )
            {
                // could be threaded
                {
                    if( d.ptr->re.can_execute() )
                    {
                        {
                            auto iter = std::find_if( _xlib_windows.begin(), _xlib_windows.end(), [&]( xlib_window_data const & wd )
                            {
                                return wd.hwnd == d.hwnd ;
                            } ) ;

                            d.ptr->ctx.borrow_backend()->set_window_info( {
                                size_t(iter->width), size_t(iter->height) } ) ;
                        }

                        d.ptr->ctx.activate() ;
                        d.ptr->ctx.borrow_backend()->render_begin() ;
                        d.ptr->re.execute_frame() ;
                        d.ptr->ctx.borrow_backend()->render_end() ;
                        d.ptr->ctx.swap() ;
                        d.ptr->ctx.deactivate() ;
                    }
                    // required for now, otherwise the application stalls.
                    else if( milli == 0 )
                    {
                        std::this_thread::sleep_for( std::chrono::milliseconds(1) ) ;
                    }
                }

                this_t::find_window_info( d.hwnd, [&]( this_t::xlib_window_data_ref_t wd )
                {
                    // set frame time
                    {
                        XStoreName( _display, wd.hwnd, ( wd.window_text + " [" + motor::to_string(milli) + " ms]").c_str() ) ;
                    }

                    // set vsync
                    {
                        if( wd.sv.vsync_msg_changed ) 
                        {
                            d.ptr->ctx.vsync( wd.sv.vsync_msg.on_off ) ;
                            wd.sv.vsync_msg_changed = false ;
                        }
                    }
                } )  ;
            }
        }

        #endif
        
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
                    wd.x = 0 ;
                    wd.y = 0 ;
                    wd.width = d.wi.w ;
                    wd.height = d.wi.h ;
                    wd.window_text = d.wi.window_name ;

                    _xlib_windows.emplace_back(wd);
                }

                // deduce auto graphics api type
                if( d.wi.gen == motor::application::graphics_generation::gen4_auto ) 
                {
                    #if MOTOR_GRAPHICS_DIRECT3D
                    d.wi.gen = motor::application::graphics_generation::gen4_d3d11 ;
                    #elif MOTOR_GRAPHICS_EGL
                    d.wi.gen = motor::application::graphics_generation::gen4_es3 ;
                    #elif MOTOR_GRAPHICS_GLX
                    d.wi.gen = motor::application::graphics_generation::gen4_gl4 ;
                    #else
                    d.wi.gen = motor::application::graphics_generation:: none ;
                    #endif
                }

                #if MOTOR_GRAPHICS_GLX
                if( d.wi.gen == motor::application::graphics_generation::gen4_gl4 )
                {
                    _xlib_windows.back().window_text = " [gl4 #" + motor::to_string(wnd_idx) +"]";

                    motor::platform::glx::glx_context_t ctx ;

                    auto const res = ctx.create_context( hwnd, _display ) ;
                    if( motor::platform::success( res ) )
                    {
                        ctx.activate() ;
                        ctx.clear_now( motor::math::vec4f_t(0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                        ctx.swap() ;
                        ctx.clear_now( motor::math::vec4f_t(0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                        ctx.swap() ;
                        ctx.deactivate() ;

                        this_t::glx_pimpl * pimpl = motor::memory::global_t::alloc(
                            this_t::glx_pimpl( { std::move(ctx), motor::graphics::render_engine_t(), nullptr } ), "[xlib_carrier] : glx context") ;

                            pimpl->fe = motor::memory::global_t::alloc( motor::graphics::gen4::frontend_t( &pimpl->re, pimpl->ctx.backend() ),
                                "[xlib_carrier] : gen4 frontend") ;
                            
                            _xlib_windows.back().wnd->set_renderable( &pimpl->re, pimpl->fe ) ;

                        _glx_windows.emplace_back( glx_window_data({ hwnd, pimpl }) )  ;
                    }
                    else
                    {
                        motor::log::global_t::critical( "Wanted to create a GLX window but could not." ) ;
                    }
                }
                #endif

                #if MOTOR_GRAPHICS_EGL 
                if( d.wi.gen == motor::application::graphics_generation::gen4_es3 )
                {
                    _xlib_windows.back().window_text = " [es3 #" + motor::to_string(wnd_idx) +"]";

                    motor::platform::egl::egl_context_t ctx ;

                    auto const res = ctx.create_context( (EGLNativeWindowType)hwnd, (EGLNativeDisplayType)_display ) ;
                    if( motor::platform::success( res ) )
                    {
                        ctx.activate() ;
                        ctx.clear_now( motor::math::vec4f_t(0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                        ctx.swap() ;
                        ctx.clear_now( motor::math::vec4f_t(0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                        ctx.swap() ;
                        ctx.deactivate() ;

                        this_t::egl_pimpl * pimpl = motor::memory::global_t::alloc(
                            this_t::egl_pimpl( { std::move(ctx), motor::graphics::render_engine_t(), nullptr } ), "[xlib_carrier] : glx context") ;

                            pimpl->fe = motor::memory::global_t::alloc( motor::graphics::gen4::frontend_t( &pimpl->re, pimpl->ctx.backend() ),
                                "[xlib_carrier] : gen4 frontend") ;
                            
                            _xlib_windows.back().wnd->set_renderable( &pimpl->re, pimpl->fe ) ;

                        _egl_windows.emplace_back( egl_window_data({ hwnd, pimpl }) )  ;
                    }
                    else
                    {
                        motor::log::global_t::critical( "Wanted to create a EGL window but could not." ) ;
                    }
                }
                #endif

                if( d.wi.gen == motor::application::graphics_generation::none )
                {
                    motor::log::global_t::error( "Wanted to create a graphics window but no api chosen or available." ) ;
                }

                this_t::send_create( _xlib_windows.back() ) ;
                this_t::send_resize( _xlib_windows.back() ) ;
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
motor::application::iwindow_mtr_safe_t xlib_carrier::create_window( motor::application::window_info_cref_t info ) noexcept  
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
        ExposureMask | StructureNotifyMask | ResizeRedirectMask | SubstructureRedirectMask | KeyPressMask | KeyReleaseMask | 
        PointerMotionMask | ButtonPressMask | ButtonReleaseMask | VisibilityChangeMask ) ;

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
bool_t xlib_carrier::handle_destroyed_hwnd( Window hwnd ) noexcept 
{
    auto iter = std::find_if( _xlib_windows.begin(), _xlib_windows.end(), [&]( xlib_window_data_cref_t d )
    {
        return d.hwnd == hwnd ;
    } ) ;

    assert( iter != _xlib_windows.end() ) ;

    this_t::send_destroy( *iter ) ;

    size_t const borrowed = iter->wnd->set_renderable( nullptr, nullptr ) ;
    if( borrowed != 0 ) return false ;
    
    #if MOTOR_GRAPHICS_GLX
    // look for glx windows/context connection
    // and remove those along with the window
    {
        auto iter2 = std::find_if( _glx_windows.begin(), _glx_windows.end(), [&]( glx_window_data_cref_t d )
        {
            return d.hwnd == hwnd ;
        } ) ;

        if( iter2 != _glx_windows.end() )
        {
            motor::memory::global_t::dealloc( iter2->ptr->fe ) ;
            motor::memory::global_t::dealloc( iter2->ptr ) ;
            _glx_windows.erase( iter2 ) ;
        }
    }
    #endif 

    #if MOTOR_GRAPHICS_EGL
    // look for egl windows/context connection
    // and remove those along with the window
    {
        auto iter2 = std::find_if( _egl_windows.begin(), _egl_windows.end(), [&]( egl_window_data_cref_t d )
        {
            return d.hwnd == hwnd ;
        } ) ;

        if( iter2 != _egl_windows.end() )
        {
            motor::memory::global_t::dealloc( iter2->ptr->fe ) ;
            motor::memory::global_t::dealloc( iter2->ptr ) ;
            _egl_windows.erase( iter2 ) ;
        }
    }
    #endif 

    XDestroyWindow( _display, iter->hwnd ) ;

    motor::memory::release_ptr( iter->wnd ) ;
    motor::memory::release_ptr( iter->lsn ) ;
    _xlib_windows.erase( iter ) ;

    return true ;
}

//*******************************************************************************************
void_t xlib_carrier::send_destroy( xlib_window_data_in_t d ) noexcept 
{
    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( motor::application::close_message_t{true} ) ;
    } ) ;
}

//*******************************************************************************************
void_t xlib_carrier::send_create( xlib_window_data_in_t d ) noexcept 
{
    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( motor::application::create_message_t() ) ;
    } ) ;
}

//*******************************************************************************************
void_t xlib_carrier::send_resize( xlib_window_data_in_t d ) noexcept
{    
    XWindowAttributes attrs ;
    XGetWindowAttributes( _display, d.hwnd, &attrs ) ;

    motor::application::resize_message const rm {
        true,
        int_t(attrs.x), int_t(attrs.y), 
        true,
        size_t(attrs.width), size_t(attrs.height)
    } ;

    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( rm ) ;
    } ) ;
}

//*******************************************************************************************
bool_t xlib_carrier::find_window_info( Window hwnd, xlib_carrier::find_window_info_funk_t funk ) noexcept 
{
    auto iter = std::find_if( _xlib_windows.begin(), _xlib_windows.end(), [&]( this_t::xlib_window_data_cref_t d )
    {
        return d.hwnd == hwnd ;
    } ) ;

    if( iter == _xlib_windows.end() ) return false ;

    funk( *iter ) ;

    return true ;
}

//***********************************************************************
void_t xlib_carrier::create_and_register_device_modules( void_t ) noexcept 
{
    _device_module = motor::shared( motor::platform::xlib::xlib_module_t(), "[xlib] : device module" ) ;
    this_t::get_dev_system()->add_module( motor::share( _device_module ) ) ;
}