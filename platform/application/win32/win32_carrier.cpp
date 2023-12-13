#include "win32_carrier.h"
#include "win32_window.h"

#include <motor/device/global.h>
#include <motor/log/global.h>

#include <windows.h>

using namespace motor::platform ;
using namespace motor::platform::win32 ;

//***********************************************************************
win32_carrier::win32_carrier( void_t ) noexcept
{
    this_t::create_and_register_device_modules() ;
}

//***********************************************************************
win32_carrier::win32_carrier( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) )
{
    _rawinput = motor::move( rhv._rawinput ) ;
    _xinput = motor::move( rhv._xinput ) ;
    _win32_windows = std::move( rhv._win32_windows ) ;
    _queue = std::move( rhv._queue ) ;
}

//***********************************************************************
win32_carrier::win32_carrier( motor::application::iapp_mtr_shared_t app ) noexcept : base_t( std::move( app ) )
{
    this_t::create_and_register_device_modules() ;
}

//***********************************************************************
win32_carrier::win32_carrier( motor::application::iapp_mtr_unique_t app ) noexcept : base_t( std::move( app ) )
{
    this_t::create_and_register_device_modules() ;
}

//***********************************************************************
win32_carrier::~win32_carrier( void_t ) noexcept
{
    motor::memory::release_ptr( _rawinput ) ;
    motor::memory::release_ptr( _xinput ) ;

    //for( motor::platform::win32::window_mtr_t mtr : _win32_windows ) 
      //  motor::memory::release_ptr( mtr ) ;
}

//***********************************************************************
motor::application::result win32_carrier::on_exec( void_t ) noexcept
{
    while( !_done )
    {
        {
            MSG msg ;
            for( auto & d : _win32_windows )
            {
                while( PeekMessage( &msg, d.hwnd, 0, 0, PM_REMOVE ) )
                {
                    TranslateMessage( &msg ) ;
                    DispatchMessage( &msg ) ;
                }
            }

            // test window destruction
            {
                for( HWND hwnd : _destroy_queue )
                {
                    auto iter = std::find_if( _win32_windows.begin(), _win32_windows.end(), [&]( win32_window_data_cref_t d )
                        {
                            return d.hwnd == hwnd ;
                        } ) ;
                
                    if( iter == _win32_windows.end() ) continue ;

                    this_t::send_destroy( *iter ) ;

                    motor::memory::release_ptr( iter->wnd ) ;

                    _win32_windows.erase( iter ) ;
                    
                }
                _destroy_queue.clear() ;
            }

            // test window queue for creation
            {
                std::lock_guard< std::mutex > lk ( _mtx_queue ) ;
                for( auto & d : _queue )
                {
                    //auto mtr = motor::memory::create_ptr( motor::platform::win32::window_t( d.wi ),
                        //  "[win32_carrier] : win32 window" ) ;

                    HWND hwnd = this_t::create_win32_window( d.wi ) ;

                    _win32_windows.emplace_back( win32_window_data{ hwnd, d.wnd } ) ;
                }
                _queue.clear() ;
            }
        }


        #if 0
        _rawinput->handle_input_event( msg.hwnd, msg.message,
            msg.wParam, msg.lParam ) ;
        #endif
        //std::this_thread::sleep_for( std::chrono::milliseconds(200) ) ;
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
    #if 0
    _rawinput = motor::memory::create_ptr< motor::platform::win32::rawinput_module_t >( 
        "[win32] : raw input module" ) ;

    _xinput = motor::memory::create_ptr< motor::platform::win32::xinput_module_t > (
        "[win32] : xinput module" ) ;

    motor::device::global_t::system()->add_module( motor::share( _rawinput ) ) ;
    motor::device::global_t::system()->add_module( motor::share( _xinput ) ) ;
    #endif
}

//***********************************************************************
motor::application::iwindow_mtr_shared_t win32_carrier::create_window( motor::application::window_info_cref_t info ) noexcept 
{
    motor::application::window_mtr_t wnd = motor::memory::create_ptr<motor::application::window_t>(
        "[win32_carrier] : window handle" ) ;

    {
        std::lock_guard< std::mutex > lk( _mtx_queue ) ;
        _queue.emplace_back( this_t::window_queue_msg_t{info, wnd} ) ;
    }

    return motor::share( wnd )  ;
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

    ShowWindow( hwnd, SW_SHOW ) ;

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

    case WM_SHOWWINDOW:
        //this_t::send_show( wParam ) ;
        // pass-through
    case WM_SIZE:
        //this_t::send_resize( hwnd ) ;
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
            l->on_close( motor::application::close_message_t{true} ) ;
    } ) ;
}