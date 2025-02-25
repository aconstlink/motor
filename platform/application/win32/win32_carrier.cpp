#include "win32_carrier.h"
#include "../../cpu_id.h"

#include <motor/graphics/render_engine.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/log/global.h>

#if MOTOR_GRAPHICS_WGL
#include "../wgl/wgl_context.h"
#endif

#if MOTOR_GRAPHICS_DIRECT3D
#include "../d3d/dx11_context.h"
#endif

#include "../../audio/oal/oal.h"
#include "../../network/network_module_creator.hpp"

#include <motor/concurrent/global.h>

#include <windows.h>
//#include <windowsx.h>

#include <cstdio>

using namespace motor::platform ;
using namespace motor::platform::win32 ;

#if MOTOR_GRAPHICS_WGL
struct win32_carrier::wgl_pimpl
{
    motor::platform::wgl::wgl_context_t ctx ;
    motor::graphics::render_engine_t re ;
    motor::graphics::ifrontend_ptr_t fe ;
} ;
#endif

#if MOTOR_GRAPHICS_DIRECT3D
struct win32_carrier::d3d11_pimpl
{
    motor::platform::directx::dx11_context_t ctx ;
    motor::graphics::render_engine_t re ;
    motor::graphics::ifrontend_ptr_t fe ;
} ;
#endif
//***********************************************************************
win32_carrier::win32_carrier( void_t ) noexcept
{
    this_t::create_and_register_device_modules() ;
    this_t::create_and_register_audio_backend() ;
    this_t::create_and_register_network_modules() ;

    motor::log::global_t::status( "[win32::cpuv] : : " + motor::platform::cpu_id_t::vendor_string() ) ; 
    motor::log::global_t::status( "[win32::cpub]: " + motor::platform::cpu_id_t::brand_string() ) ;
    motor::log::global_t::status( "[win32::is]: " + motor::platform::cpu_id_t::instruction_sets_string() ) ;
}

//***********************************************************************
win32_carrier::win32_carrier( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
{
    _rawinput = motor::move( rhv._rawinput ) ;
    _xinput = motor::move( rhv._xinput ) ;
    _midi = motor::move( rhv._midi ) ;

    _win32_windows = std::move( rhv._win32_windows ) ;
    _queue = std::move( rhv._queue ) ;

    _wgl_windows = std::move( rhv._wgl_windows ) ;
    _d3d11_windows = std::move( rhv._d3d11_windows ) ;
}

//***********************************************************************
win32_carrier::win32_carrier( motor::application::app_mtr_safe_t app ) noexcept : base_t( std::move( app ) )
{
    this_t::create_and_register_device_modules() ;
    this_t::create_and_register_audio_backend() ;
    this_t::create_and_register_network_modules() ;

    motor::log::global_t::status( "[win32::cpuv] : : " + motor::platform::cpu_id_t::vendor_string() ) ;
    motor::log::global_t::status( "[win32::cpub]: " + motor::platform::cpu_id_t::brand_string() ) ;
    motor::log::global_t::status( "[win32::is]: " + motor::platform::cpu_id_t::instruction_sets_string() ) ;
}

//***********************************************************************
win32_carrier::~win32_carrier( void_t ) noexcept
{
    motor::memory::release_ptr( _rawinput ) ;
    motor::memory::release_ptr( _xinput ) ;
    motor::memory::release_ptr( _midi ) ;
}

//***********************************************************************
motor::application::result win32_carrier::on_exec( void_t ) noexcept
{
    _clock_t::time_point tp_begin = _clock_t::now() ;

    while( !_done )
    {
        // should be called in the main carrier!
        // but that requires a different handling of the while loop.
        motor::concurrent::global_t::update() ;

        {
            _clock_t::duration const dur = _clock_t::now() - tp_begin ;
            tp_begin = _clock_t::now() ;

            size_t const micro = std::chrono::duration_cast< std::chrono::microseconds >( dur ).count() ;
        
            // required for now, otherwise the application stalls.
            if( micro == 0 )
            {
                std::this_thread::sleep_for( std::chrono::microseconds(1) ) ;
            }
        }

        //motor::log::global_t::status( "main loop: " + motor::to_string(micro) ) ; 

        // this also determines the maximum frame 
        // rate of rendering windows
        //std::this_thread::sleep_for( std::chrono::milliseconds(2) )  ;

        {
            // The main message loop
            {
                MSG msg ;
                for( auto & d : _win32_windows )
                {
                    while( PeekMessage( &msg, d.hwnd, 0, 0, PM_REMOVE ) )
                    {
                        TranslateMessage( &msg ) ;
                        DispatchMessage( &msg ) ;

                        _rawinput->handle_input_event( msg.hwnd, msg.message,
                            msg.wParam, msg.lParam ) ;
                    }

                    // handle all incoming messages directed to the window
                    {
                        motor::application::window_message_listener_t::state_vector states ;
                        if( d.lsn->swap_and_reset( states ) )
                        {
                            this_t::handle_messages( d, states ) ;
                        }
                    }
                }
            }

            // test window destruction
            {
                this_t::handle_destroyed() ;
            }

            #if MOTOR_GRAPHICS_WGL

            // update wgl window context
            // must be done here. If the window is closed,
            // the window handle is not valid anymore and must be
            // destructed in this class first.
            {
                for( auto & d : _wgl_windows )
                {
                    bool_t need_vsync_change = false ;
                    bool_t do_vsync = false ;

                    this_t::find_window_info( d.hwnd, [&]( this_t::win32_window_data_ref_t wd )
                    {
                        // set frame time
                        {
                            char tmp_text[2048] ;
                            
                            size_t const milli = d.micro_rnd/1000 ;
                            size_t const fps = size_t( 1.0 / (double_t(d.micro_rnd)/1000000.0) ) ;

                            sprintf_s( tmp_text, "%s [ %zd ms; %zd fps ]", wd.window_text.c_str(), milli, fps  ) ;

                            #if 1
                            SetWindowText( wd.hwnd, tmp_text ) ;
                            #else
                            SetWindowText( wd.hwnd, ( wd.window_text + " [" + motor::to_string(milli) + " ms; " + 
                                motor::to_string(fps) + " fps]").c_str() ) ;
                            #endif
                        }

                        // set vsync
                        {
                            if( wd.sv.vsync_msg_changed ) 
                            {
                                need_vsync_change = true ;
                                do_vsync = wd.sv.vsync_msg.on_off ;
                                wd.sv.vsync_msg_changed = false ;
                            }
                        }
                    } )  ;

                    // could be threaded
                    {
                        // may fail if window is already closed! So need to ask.
                        if( d.ptr->ctx.activate() == motor::platform::result::ok )
                        {
                            if( need_vsync_change )
                            {
                                d.ptr->ctx.vsync( do_vsync ) ;
                            }

                            if( d.ptr->re.can_execute() )
                            {
                                {
                                    RECT rect ;
                                    GetClientRect( d.hwnd, &rect ) ;

                                    d.ptr->ctx.borrow_backend()->set_window_info( {
                                        size_t(rect.right-rect.left), size_t(rect.bottom-rect.top) } ) ;
                                }

                                d.ptr->ctx.borrow_backend()->render_begin() ;
                                d.ptr->re.execute_frame() ;
                                d.ptr->ctx.borrow_backend()->render_end() ;
                                d.ptr->ctx.swap() ;

                                d.micro_rnd = std::chrono::duration_cast< std::chrono::microseconds >( 
                                    _clock_t::now() - d.rnd_beg ).count() ;
                                d.rnd_beg = _clock_t::now() ;
                            }
                            else
                            {
                                //d.ptr->ctx.swap() ;
                            }
                            d.ptr->ctx.deactivate() ;
                        }
                        else if( d.ptr->re.can_execute() )
                        {
                            d.ptr->re.force_clear() ;
                        }

                    }
                }
            }

            #endif

            #if MOTOR_GRAPHICS_DIRECT3D

            // update d3d11 window context
            // must be done here. If the window is closed,
            // the window handle is not valid anymore and must be
            // destructed in this class first.
            {
                for( auto & d : _d3d11_windows )
                {
                    #if 0
                    // we can try to access the renderer a few times and
                    // wait for unlocking.
                    {
                        size_t trys = 5 ;
                        while( --trys > 0 && !d.ptr->re.can_execute() )
                        {
                            std::this_thread::sleep_for( std::chrono::microseconds(10) ) ;
                        }
                    }
                    #endif

                    // could be threaded
                    {
                        if( d.ptr->re.can_execute() )
                        {
                            d.ptr->ctx.activate() ;

                            {
                                RECT rect ;
                                GetClientRect( d.hwnd, &rect ) ;

                                d.ptr->ctx.borrow_backend()->set_window_info( {
                                    size_t(rect.right-rect.left), size_t(rect.bottom-rect.top) } ) ;
                            }

                            d.ptr->ctx.borrow_backend()->render_begin() ;
                            d.ptr->re.execute_frame() ;
                            d.ptr->ctx.borrow_backend()->render_end() ;

                            d.ptr->ctx.swap() ;

                            d.micro_rnd = std::chrono::duration_cast< std::chrono::microseconds >( 
                                _clock_t::now() - d.rnd_beg ).count() ;
                            d.rnd_beg = _clock_t::now() ;

                            d.ptr->ctx.deactivate() ;

                            d.frame_miss = 0 ;
                        }
                        // if this is not done, it will completely halt the whole computer!
                        // at some frame misses, e.g. user did not do any rendering, the loop 
                        // needs to go to sleep for a while.
                        else if( ++d.frame_miss > 5 )
                        {
                            std::this_thread::sleep_for( std::chrono::microseconds(10) ) ;
                        }
                    }

                    this_t::find_window_info( d.hwnd, [&]( this_t::win32_window_data_ref_t wd )
                    {
                        // set frame time
                        {
                            uint_t const milli = uint_t( d.micro_rnd/1000 );
                            uint_t const fps = d.micro_rnd == 0 ? 0 : uint_t( 1.0 / (double_t(d.micro_rnd)/1000000.0) ) ;

                            char buffer[100] ;

                            std::snprintf( buffer, 100, "%s [%3d ms; %3d fps ; %3d miss]", wd.window_text.c_str(), milli, fps, d.frame_miss ) ;
                            SetWindowText( wd.hwnd, buffer ) ;
                        }

                        // set vsync
                        {
                            if( wd.sv.vsync_msg_changed ) 
                            {
                                d.ptr->ctx.vsync( wd.sv.vsync_msg.on_off ) ;
                                wd.sv.vsync_msg_changed = false ;
                            }
                        }
                    } ) ;
                }
            }

            #endif
            
            // test window queue for creation
            {
                std::lock_guard< std::mutex > lk ( _mtx_queue ) ;
                for( auto & d : _queue )
                {
                    size_t const wnd_idx = _win32_windows.size() ;
                    HWND hwnd = this_t::create_win32_window( d.wi ) ;

                    {
                        win32_window_data wd ;
                        wd.hwnd = hwnd ;
                        wd.wnd = d.wnd ;
                        wd.lsn = d.lsn ;
                        wd.window_text = d.wi.window_name ;

                        _win32_windows.emplace_back(wd);
                    }

                    // deduce auto graphics api type
                    if( d.wi.gen == motor::application::graphics_generation::gen4_auto ) 
                    {
                        #if MOTOR_GRAPHICS_DIRECT3D
                        d.wi.gen = motor::application::graphics_generation::gen4_d3d11 ;
                        #elif MOTOR_GRAPHICS_WGL
                        d.wi.gen = motor::application::graphics_generation::gen4_gl4 ;
                        #else
                        d.wi.gen = motor::application::graphics_generation::none ;
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

                             pimpl->fe = motor::memory::global_t::alloc( motor::graphics::gen4::frontend_t( &pimpl->re, pimpl->ctx.borrow_backend() ),
                                 "[carrier32] : gen4 frontend") ;
                             
                             _win32_windows.back().wnd->set_renderable( &pimpl->re, pimpl->fe ) ;

                            _wgl_windows.emplace_back( wgl_window_data({ hwnd, pimpl, _clock_t::now() }) )  ;
                        }
                        else
                        {
                            motor::log::global_t::critical( "Wanted to create a WGL window but could not." ) ;
                        }
                    }
                    #endif

                    #if MOTOR_GRAPHICS_DIRECT3D
                    if( d.wi.gen == motor::application::graphics_generation::gen4_d3d11 )
                    {
                        _win32_windows.back().window_text = " [d3d11 #" + motor::to_string(wnd_idx) +"]";

                        motor::platform::directx::dx11_context_t ctx ;

                        auto const res = ctx.create_context( hwnd ) ;
                        if( motor::platform::success( res ) )
                        {
                            ctx.activate() ;
                            ctx.clear_now( motor::math::vec4f_t( 0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                            ctx.swap() ;
                            ctx.clear_now( motor::math::vec4f_t( 0.0f, 0.5f, 0.3f, 1.0f ) ) ;
                            ctx.swap() ;
                            ctx.deactivate() ;

                            this_t::d3d11_pimpl * pimpl = motor::memory::global_t::alloc(
                                this_t::d3d11_pimpl( {std::move(ctx) } ), "[win32_carrier] : d3d11 context") ;

                            pimpl->fe = motor::memory::global_t::alloc( motor::graphics::gen4::frontend_t( &pimpl->re, pimpl->ctx.borrow_backend() ),
                                 "[carrier32] : gen4 frontend") ;

                            _win32_windows.back().wnd->set_renderable( &pimpl->re, pimpl->fe ) ;

                            _d3d11_windows.emplace_back( d3d11_window_data({ hwnd, pimpl, this_t::_clock_t::now(), 0, 0 }) )  ;
                        }
                        else
                        {
                            motor::log::global_t::critical( "Wanted to create a D3D11 window but could not." ) ;
                        }
                    }
                    #endif

                    if( d.wi.gen == motor::application::graphics_generation::none )
                    {
                        motor::log::global_t::error( "Wanted to create a graphics window but no api chosen or available." ) ;
                    }

                    this_t::send_create( _win32_windows.back() ) ;
                }
                _queue.clear() ;
            }
        }
    }

    // in case while broke earlier.
    this_t::handle_destroyed() ;

    for( auto const & d : _win32_windows )
    {
        CloseWindow( d.hwnd ) ;
        while( !this_t::handle_destroyed_hwnd( d.hwnd ) ) ;
    }

    return motor::application::result::ok ;
}

//***********************************************************************
motor::application::result win32_carrier::close( void_t ) noexcept
{
    _done = true ;
    return motor::application::result::ok ;
}

//***********************************************************************
void_t win32_carrier::create_and_register_device_modules( void_t ) noexcept 
{
    _rawinput = motor::memory::create_ptr< motor::platform::win32::rawinput_module_t >( 
        "[win32] : raw input module" ) ;

    _xinput = motor::memory::create_ptr< motor::platform::win32::xinput_module_t > (
        "[win32] : xinput module" ) ;

    _midi = motor::memory::create_ptr< motor::platform::win32::midi_module_t > (
        "[win32] : midi module" ) ;

    this_t::get_dev_system()->add_module( motor::share( _rawinput ) ) ;
    this_t::get_dev_system()->add_module( motor::share( _xinput ) ) ;
    this_t::get_dev_system()->add_module( motor::share( _midi ) ) ;
}

//***********************************************************************
void_t win32_carrier::create_and_register_audio_backend( void_t ) noexcept 
{
    motor::platform::oal_backend_mtr_t bend = motor::memory::global_t::create( motor::platform::oal_backend_t(),
        "[win32_carrier] : created oal backend and passed to carrier audio system." ) ;

    motor::audio::system::controller( this->get_audio_system() ).start( motor::move( bend ) ) ;
}

//***********************************************************************
void_t win32_carrier::create_and_register_network_modules( void_t ) noexcept 
{
    this_t::network_system()->add_module( motor::platform::network_module_creator::create() ) ;
}

//***********************************************************************
motor::application::iwindow_mtr_safe_t win32_carrier::create_window( motor::application::window_info_cref_t info ) noexcept 
{
    motor::application::window_mtr_t wnd = motor::memory::create_ptr<motor::application::window_t>(
        "[win32_carrier] : window handle" ) ;

    motor::application::window_message_listener_mtr_t lsn = motor::memory::create_ptr<
        motor::application::window_message_listener_t>("[win32_carrier] : window message listener") ;

    wnd->register_in( motor::share( lsn ) ) ;

    {
        std::lock_guard< std::mutex > lk( _mtx_queue ) ;
        _queue.emplace_back( this_t::window_queue_msg_t{info, wnd, lsn} ) ;
    }

    return motor::unique( motor::memory::copy_ptr(wnd) )  ;
}

//***********************************************************************
HWND win32_carrier::create_win32_window( motor::application::window_info_cref_t wi ) noexcept 
{
    HINSTANCE hinst = GetModuleHandle(0) ;
    
    static size_t window_number = 0 ;
    motor::string_t class_name = wi.window_name  + " " + motor::to_string( window_number++ ) ;

    HWND hwnd ;
    WNDCLASSA wndclass ;

    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
    wndclass.lpfnWndProc = this_t::WndProc ;
    wndclass.cbClsExtra = 0 ;
    wndclass.cbWndExtra = 0 ;
    wndclass.hInstance = hinst ;
    wndclass.hIcon = LoadIcon(0, IDI_APPLICATION ) ;
    wndclass.hCursor = LoadCursor( 0, IDC_ARROW ) ;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH) ;
    wndclass.lpszMenuName = 0 ;
    wndclass.lpszClassName = class_name.c_str() ;
        
    //_name = wi.window_name ;

    if( motor::log::global::error( !RegisterClassA( &wndclass ), 
        "[window::create_window] : RegisterClassA" ) )
        exit(0) ;

    DWORD ws_style ;
    DWORD ws_ex_style ;

    int start_x = wi.x ; //GetSystemMetrics(SM_CXSCREEN) >> 2 ;
    int start_y = wi.y ; // GetSystemMetrics(SM_CYSCREEN) >> 2 ;
    int width = wi.w ; // GetSystemMetrics(SM_CXSCREEN) >> 1 ;
    int height = wi.h ; // GetSystemMetrics(SM_CYSCREEN) >> 1 ;

    {
        ShowCursor( wi.show_cursor ) ;
        //_is_cursor = wil.show_cursor ;
    }

    // WS_POPUP is said to introduce tearing, so it is removed for now
    if( wi.fullscreen )
    {
        ws_ex_style = WS_EX_APPWINDOW /*& ~(WS_EX_DLGMODALFRAME |
                      WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE )*/ ;
        ws_style = /*WS_POPUP |*/ SW_SHOWNORMAL;
        start_x = start_y = 0 ;        
        width = GetSystemMetrics(SM_CXSCREEN) ;
        height = GetSystemMetrics(SM_CYSCREEN) ;
        //_is_fullscreen = true ;
    }
    else
    {
        ws_ex_style = WS_EX_APPWINDOW ;// | WS_EX_WINDOWEDGE ;

        if( wi.borderless )
        {
            ws_style = /*WS_POPUP |*/ SW_SHOWNORMAL;
        }
        else
        {
            ws_style = WS_OVERLAPPEDWINDOW | SW_SHOWNORMAL ; // | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;
            height += GetSystemMetrics( SM_CYCAPTION ) ;
        }
    }

    hwnd = CreateWindowEx( ws_ex_style,
        class_name.c_str(), wi.window_name.c_str(),
        ws_style, // | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        start_x,
        start_y,
        width,
        height,
        0,
        0,
        hinst,
        0 ) ;

    if( motor::log::global::error( hwnd == NULL, 
        "[window::create_window] : CreateWindowA failed" ) )
        exit(0) ;

    // Important action here. The user data is used pass the object
    // that will perform the callback in the static wndproc
    SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)this ) ;

    return hwnd ;
}

//*******************************************************************************************
LRESULT CALLBACK win32_carrier::WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
    case WM_DWMNCRENDERINGCHANGED:
        break ;
    case WM_SYSCOMMAND: break ;

    case WM_STYLECHANGED:
        break ;

    case WM_SHOWWINDOW:
        //this_t::send_show( wParam ) ;
        // pass-through
        break ;
    
        // requires TrackMouseEvent
    case WM_MOUSELEAVE :
    {
        this_ptr_t p = (this_ptr_t) GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
        p->find_window_info( hwnd, [&] ( this_t::win32_window_data_ref_t wi )
        {
            p->send_mouse( wi, motor::application::mouse_message::state_type::leave ) ;
        } ) ;
    }
    break ;

    case WM_MOUSEMOVE:
    {
        // GET_X_LPARAM requires #include< windowsx.h > 
        //int_t x = GET_X_LPARAM( lParam ) ;
        //int_t y = GET_Y_LPARAM( lParam ) ;

        this_ptr_t p = (this_ptr_t) GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
        p->find_window_info( hwnd, [&] ( this_t::win32_window_data_ref_t wi )
        {
            p->send_mouse( wi, motor::application::mouse_message::state_type::move ) ;
        } ) ;
        
    }
    break ;

    case WM_MOVE:
    {
        this_ptr_t p = (this_ptr_t) GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
        p->find_window_info( hwnd, [&] ( this_t::win32_window_data_ref_t wi )
        {
            p->send_resize( wi ) ;
        } ) ;
    }
    break ;

    //case WM_SIZE: // too many messages
    //case WM_ENTERSIZEMOVE:
    case WM_EXITSIZEMOVE:
    {
        this_ptr_t p = (this_ptr_t)GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
        p->find_window_info( hwnd, [&]( this_t::win32_window_data_ref_t wi )
        {
            p->send_resize( wi ) ;
        } ) ;
    }
        
        break ;
        
    case WM_DPICHANGED:
        //this_t::send_screen_dpi( hwnd, 
          //  uint_t(LOWORD(wParam)), uint_t(HIWORD(wParam)) ) ;
        break ;

    case WM_DISPLAYCHANGE:
        //this_t::send_screen_size( hwnd, uint_t(LOWORD(lParam)), uint_t(HIWORD(lParam)) ) ;
        break ;
    
    case WM_ACTIVATE:
    {
        int bp=0; 
        (void)bp ;
    }
        break ;
    case WM_SETFOCUS:
    {
        int bp=0; 
        (void)bp ;
    }
        break ;
      
    case WM_SETCURSOR:
    {
        bool_t handle_it = false ;

        this_ptr_t p = (this_ptr_t)GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
        p->find_window_info( hwnd, [&]( this_t::win32_window_data_ref_t wi )
        {
            if( wi.sv.cursor_msg_changed && !wi.sv.cursor_msg.on_off )
            {
                handle_it = true ;
            }
        } ) ;

        if( handle_it && (LOWORD(lParam) == HTCLIENT) )
        {
            SetCursor(NULL);
            return TRUE;
        }
        
    }

    case WM_KILLFOCUS:
    {
        int bp=0; 
        (void)bp ;
    }
        break ;

    case WM_PAINT:
        break ;
    case WM_USER:
        #if 0
        // use wParam==1 for user-controlled window messaging
        if( wParam == WPARAM(1) )
        {
            // check window listener 
            {
                motor::application::window_message_listener_t::state_vector_t states ;
                if( _lsn_in->swap_and_reset(states) )
                {
                    if( states.fulls_msg_changed)
                    {
                        motor::application::toggle_window_t tw ;
                        tw.toggle_fullscreen = states.fulls_msg.on_off ;
                        this_t::send_toggle( tw ) ;
                    }
                    if( states.resize_changed )
                    {
                        motor::application::resize_message_t & msg_ = states.resize_msg ;
                        this_t::send_resize( msg_ ) ;
                    }
                }
            }
        }
        #endif
        
        

    case WM_QUIT: 
        break ;

    case WM_CLOSE: 
        break ;

    case WM_DESTROY: 
    {
        this_ptr_t p = (this_ptr_t)GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
        p->_destroy_queue.emplace_back( hwnd ) ;
        break ;
    }
        
    }

    return DefWindowProc( hwnd, msg, wParam, lParam ) ;
}

//*******************************************************************************************
void_t win32_carrier::send_destroy( win32_window_data_in_t d ) noexcept 
{
    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( motor::application::close_message_t{true} ) ;
    } ) ;
}

//*******************************************************************************************
void_t win32_carrier::send_create( win32_window_data_in_t d ) noexcept 
{
    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( motor::application::create_message_t() ) ;
    } ) ;
}

//*******************************************************************************************
void_t win32_carrier::send_resize( win32_window_data_inout_t d ) noexcept
{
    RECT client_rect ;
    GetClientRect( d.hwnd, &client_rect ) ;

    RECT window_rect ;
    GetWindowRect( d.hwnd, &window_rect ) ;

    int_t const x = int_t( window_rect.left ) ;
    int_t const y = int_t( window_rect.top ) ;

    int_t const w = int_t( client_rect.right - client_rect.left ) ;
    int_t const h = int_t( client_rect.bottom - client_rect.top ) ;

    motor::application::resize_message const rm {
        true,
        x, y,
        true,
        size_t(w), size_t(h)
    } ;

    d.wnd->foreach_out( [&]( motor::application::iwindow_message_listener_mtr_t l )
    {
        l->on_message( rm ) ;
    } ) ;

    if ( d.sv.fulls_msg.fullscreen == motor::application::three_state::off )
    {
        d.sv.resize_msg.x = x ;
        d.sv.resize_msg.y = y ;
        d.sv.resize_msg.w = w ;
        d.sv.resize_msg.h = h ;
    }
}

//*******************************************************************************************
void_t win32_carrier::send_mouse( win32_window_data_inout_t d, motor::application::mouse_message_t::state_type const st ) noexcept
{
    auto new_state = st ;
    auto const old_state = d.sv.mouse_msg.state ;

    bool_t const enter_state =
        ( new_state == motor::application::mouse_message_t::state_type::move &&
            old_state == motor::application::mouse_message_t::state_type::leave ) ||
        ( new_state == motor::application::mouse_message_t::state_type::move &&
            old_state == motor::application::mouse_message_t::state_type::none  ) ;

    bool_t const leave_state = 
        new_state == motor::application::mouse_message_t::state_type::leave ;

    bool_t const move_state = 
        new_state == motor::application::mouse_message_t::state_type::move ;

    // enter state
    if ( enter_state )
    {
        // reactivate mouse tracking for leave event
        {
            TRACKMOUSEEVENT tme ;
            tme.cbSize = sizeof( tme ) ;
            tme.hwndTrack = d.hwnd ;
            tme.dwFlags = TME_LEAVE ;

            if ( !TrackMouseEvent( &tme ) )
            {
                motor::log::global_t::status( "[win32] : TrackMouseEvent failed" ) ;
            }
        }

        new_state = motor::application::mouse_message_t::state_type::enter ;
    }
    // leave state
    else if ( leave_state )
    {
        new_state = motor::application::mouse_message_t::state_type::leave ;
    }
    // move state
    else if ( move_state )
    {
        new_state = motor::application::mouse_message_t::state_type::move ;
    }
    
    // send new state to the user
    // do not need move events. those come from the raw input module.
    if( new_state != motor::application::mouse_message_t::state_type::move )
    {
        motor::application::mouse_message_t mm ;
        mm.state = new_state ;
        d.wnd->foreach_out( [&] ( motor::application::iwindow_message_listener_mtr_t l )
        {
            l->on_message( mm ) ;
        } ) ;
    }

    d.sv.mouse_msg.state = new_state ;
}

//*******************************************************************************************
void_t win32_carrier::handle_messages( win32_window_data_inout_t d, 
    motor::application::window_message_listener_t::state_vector_in_t states ) noexcept 
{
    if( states.show_changed )
    {
        ShowWindow( d.hwnd, states.show_msg.show ? SW_SHOW : SW_HIDE ) ;

        if ( states.show_msg.borderless == motor::application::three_state::on )
        {
            DWORD const style = WS_POPUP | WS_VISIBLE ;
            SetWindowLongPtrA( d.hwnd, GWL_STYLE, style ) ;
        }
        else if ( states.show_msg.borderless == motor::application::three_state::off )
        {
            DWORD const style = WS_OVERLAPPEDWINDOW | WS_VISIBLE ;
            SetWindowLongPtrA( d.hwnd, GWL_STYLE, style ) ;
        }
        else if ( states.show_msg.borderless == motor::application::three_state::toggle )
        {
            DWORD style = (DWORD) GetWindowLongPtr( d.hwnd, GWL_STYLE ) ;
            if ( style & WS_POPUP ) // its borderless
            {
                style = WS_OVERLAPPEDWINDOW | WS_VISIBLE ;
                SetWindowLongPtrA( d.hwnd, GWL_STYLE, style ) ;
            }
            else
            {
                style =  WS_VISIBLE | WS_POPUP ;
                SetWindowLongPtrA( d.hwnd, GWL_STYLE, style ) ;
            }
        }
        else{ /*invalid state*/ }
    }

    if( states.fulls_msg_changed )
    {
        motor::application::fullscreen_message_t msg = states.fulls_msg ;

        {
            DWORD ws_ex_style = WS_EX_APPWINDOW /*& ~(WS_EX_DLGMODALFRAME |
                            WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE )*/ ;
            SetWindowLongPtrA( d.hwnd, GWL_EXSTYLE, ws_ex_style ) ;
        }

        {
            if ( msg.fullscreen == motor::application::three_state::toggle )
            {
                if ( d.sv.fulls_msg.fullscreen == motor::application::three_state::on )
                {
                    msg.fullscreen = motor::application::three_state::off ;
                }
                else if ( d.sv.fulls_msg.fullscreen == motor::application::three_state::off )
                {
                    msg.fullscreen = motor::application::three_state::on ;
                }
            }

            DWORD ws_style = 0 ;

            if( msg.fullscreen == motor::application::three_state::on )
            {
                ws_style = WS_POPUP | SW_SHOWNORMAL ;
                d.sv.fulls_msg.fullscreen = motor::application::three_state::on ;
            }
            else if( msg.fullscreen == motor::application::three_state::off )
            {
                ws_style = WS_OVERLAPPEDWINDOW ;
                d.sv.fulls_msg.fullscreen = motor::application::three_state::off ;
            }
            
            else{ /*invalid state*/ }

            SetWindowLongPtrA( d.hwnd, GWL_STYLE, ws_style ) ;
        }

        {
            int_t start_x = 0, start_y = 0 ;
            int_t width = GetSystemMetrics( SM_CXSCREEN ) ;
            int_t height = GetSystemMetrics( SM_CYSCREEN ) ;

            if( msg.fullscreen == motor::application::three_state::off )
            {
                height += GetSystemMetrics( SM_CYCAPTION ) ;

                start_x = d.sv.resize_msg.x ;
                start_y = d.sv.resize_msg.y ;
                width = int_t( d.sv.resize_msg.w ) ;
                height = int_t( d.sv.resize_msg.h ) ;
            }

            SetWindowPos( d.hwnd, HWND_TOP, start_x, start_y, width, height, SWP_SHOWWINDOW ) ;
        }
    }

    if( states.resize_changed )
    {
        UINT flags = 0 ;

        motor::application::resize_message_cref_t msg = states.resize_msg ;

        RECT rc, wr ;
        GetClientRect( d.hwnd, &rc ) ;
        GetWindowRect( d.hwnd, &wr ) ;

        int_t x = rc.left ;
        int_t y = rc.top ;
        int_t width = int_t( rc.right - rc.left ) ;
        int_t height = int_t( rc.bottom - rc.top ) ;

        int_t const difx = ( wr.right - wr.left ) - rc.right;
        int_t const dify = ( wr.bottom - wr.top ) - rc.bottom;

        if( msg.position )
        {
            x = msg.x ;
            y = msg.y ;
            rc.left = x ;
            rc.top = y ;
        }
        else
        {
            flags |= SWP_NOMOVE ;
        }

        if( msg.resize )
        {
            width = ( int_t ) msg.w ;
            height = ( int_t ) msg.h ;

            rc.right = x + width ;
            rc.bottom = y + height ;
        }
        else
        {
            flags |= SWP_NOSIZE ;
        }

        int_t const w = width + difx ;
        int_t const h = height + dify ;

        // SWP_NOACTIVATE : the window should not activate if  
        //  another window controls the size/position
        // SWP_NOZORDER : Do not z over any other window.
        // SWP_NOREDRAW : Optimization
        // SWP_NOREPOSITION : Parent z order not changing
        SetWindowPos( d.hwnd, NULL, x, y, width, height, 
            SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOZORDER | flags ) ;
    }

    if( states.cursor_msg_changed )
    {
        auto const & msg = states.cursor_msg ;
        if( !msg.on_off ) 
        {
            ShowCursor( FALSE ) ;
            SetCursor( NULL ) ;
        }
        else 
        {
            ShowCursor( TRUE  ) ;
            SetCursor( LoadCursor( 0, IDC_ARROW ) ) ;
        }

        d.sv.cursor_msg_changed = true ;
        d.sv.cursor_msg = msg ;
    }

    if( states.vsync_msg_changed )
    {
        d.sv.vsync_msg_changed = true ;
        d.sv.vsync_msg = states.vsync_msg ;
    }

    if ( states.close_changed )
    {
        CloseWindow( d.hwnd ) ;
        DestroyWindow( d.hwnd ) ;
    }
}

//*******************************************************************************************
void_t win32_carrier::handle_destroyed( void_t ) noexcept 
{
    for ( auto iter = _destroy_queue.begin(); iter != _destroy_queue.end(); )
    {
        // have to wait until all users of the window are ready. 
        if ( this_t::handle_destroyed_hwnd( *iter ) )
        {
            iter = _destroy_queue.erase( iter ) ; continue ;
        }
        ++iter ;
    }
}

//*******************************************************************************************
bool_t win32_carrier::handle_destroyed_hwnd( HWND hwnd ) noexcept 
{
    auto iter = std::find_if( _win32_windows.begin(), _win32_windows.end(), [&]( win32_window_data_cref_t d )
    {
        return d.hwnd == hwnd ;
    } ) ;

    assert( iter != _win32_windows.end() ) ;

    this_t::send_destroy( *iter ) ;

    size_t const borrowed = iter->wnd->set_renderable( nullptr, nullptr ) ;
    if( borrowed != 0 ) return false ;

    #if MOTOR_GRAPHICS_WGL
    // look for wgl windows/context connection
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

    #if MOTOR_GRAPHICS_DIRECT3D
    // look for d3d11 windows/context connection
    // and remove those along with the window
    {
        auto iter2 = std::find_if( _d3d11_windows.begin(), _d3d11_windows.end(), [&]( d3d11_window_data_cref_t d )
        {
            return d.hwnd == hwnd ;
        } ) ;

        if( iter2 != _d3d11_windows.end() )
        {
            motor::memory::global_t::dealloc( iter2->ptr->fe ) ;
            motor::memory::global_t::dealloc( iter2->ptr ) ;
            _d3d11_windows.erase( iter2 ) ;
        }
    }
    #endif
    
    motor::memory::release_ptr( iter->wnd ) ;
    motor::memory::release_ptr( iter->lsn ) ;
    _win32_windows.erase( iter ) ;

    return true ;
}

//*******************************************************************************************
bool_t win32_carrier::find_window_info( HWND hwnd, win32_carrier::find_window_info_funk_t funk ) noexcept 
{
    auto iter = std::find_if( _win32_windows.begin(), _win32_windows.end(), [&]( this_t::win32_window_data_cref_t d )
    {
        return d.hwnd == hwnd ;
    } ) ;

    if( iter == _win32_windows.end() ) return false ;

    funk( *iter ) ;

    return true ;
}