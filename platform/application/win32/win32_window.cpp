#include "win32_window.h"

#include <motor/log/global.h>

#include <algorithm>

using namespace motor::platform ;
using namespace motor::platform::win32 ;

//*******************************************************************************************
window::window( void_t ) noexcept
{
    #if 0
    _lsn_in = motor::memory::create_ptr( motor::application::window_message_listener_t(), 
        "[win32_window] : window message listener" ) ;

    this_t::register_in( motor::share( _lsn_in ) ) ;
    #endif
}

//*******************************************************************************************
window::window( motor::application::window_info_cref_t info ) noexcept : this_t()
{
    this_t::create_window( info ) ;
}

//*******************************************************************************************
window::window( this_rref_t rhv ) noexcept //: base_t( std::move( rhv ) )
{
    *this = std::move( rhv ) ;
}

//*******************************************************************************************
window::~window( void_t ) noexcept 
{
    this_t::destroy_window() ;

    motor::memory::release_ptr( _lsn_in ) ;
}

//*******************************************************************************************
window::this_ref_t window::operator = ( this_rref_t rhv ) noexcept 
{
    _handle = rhv._handle ;
    rhv._handle = NULL ;
    _cursor = rhv._cursor ;
    rhv._cursor = NULL ;

    if( _handle != NULL )
    {
        SetWindowLongPtr( _handle, GWLP_USERDATA, ( LONG_PTR ) this ) ;
    }

    _is_fullscreen = rhv._is_fullscreen ;
    _is_cursor = rhv._is_cursor ;

    _name = std::move( rhv._name ) ;

    _lsn_in = motor::move( rhv._lsn_in ) ;

    return *this ;
}

//*******************************************************************************************
HWND window::get_handle( void_t ) noexcept
{
    return _handle ;
}

//*******************************************************************************************
void_t window::send_toggle( motor::application::toggle_window_in_t di ) noexcept
{
    if( _is_fullscreen != di.toggle_fullscreen )
    {
        _is_fullscreen = di.toggle_fullscreen ;

        {
            DWORD ws_ex_style = WS_EX_APPWINDOW /*& ~(WS_EX_DLGMODALFRAME |
                          WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE )*/ ;
            SetWindowLongPtrA( _handle, GWL_EXSTYLE, ws_ex_style ) ;
        }

        {
            DWORD ws_style = 0 ;

            if( _is_fullscreen )
            {
                ws_style = WS_POPUP | SW_SHOWNORMAL ;
            }
            else
            {
                ws_style = WS_OVERLAPPEDWINDOW ;
            }

            SetWindowLongPtrA( _handle, GWL_STYLE, ws_style ) ;
        }

        {
            int_t start_x = 0, start_y = 0 ;
            int_t width = GetSystemMetrics( SM_CXSCREEN ) ;
            int_t height = GetSystemMetrics( SM_CYSCREEN ) ;

            if( !_is_fullscreen )
                height += GetSystemMetrics( SM_CYCAPTION ) ;

            if( !_is_fullscreen )
            {
                width /= 2 ;
                height /= 2 ;
            }

            SetWindowPos( _handle, HWND_TOP, start_x,
                start_y, width, height, SWP_SHOWWINDOW ) ;
        }
    }

    /// @todo make it work.
    if( _is_cursor != di.toggle_show_cursor )
    {
        _is_cursor = di.toggle_show_cursor ;
        if( !_is_cursor )
        {
            _cursor = GetCursor() ;
            SetCursor( NULL ) ;
            while( ShowCursor( FALSE ) > 0 ) ;
        }
        else
        {
            SetCursor( _cursor ) ;
            _cursor = NULL ;
            ShowCursor( TRUE ) ;
        }
    }
}

//*******************************************************************************************
void_t window::send_resize( motor::application::resize_message_in_t msg ) noexcept
{
    RECT rc, wr ;
    GetClientRect( _handle, &rc ) ;
    GetWindowRect( _handle, &wr ) ;

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

    if( msg.resize )
    {
        width = ( int_t ) msg.w ;
        height = ( int_t ) msg.h ;

        rc.right = x + width ;
        rc.bottom = y + height ;
    }

    SetWindowPos( _handle, NULL, x, y, width+difx, height+dify, 0 ) ;
    
}

//*******************************************************************************************
HWND window::create_window( motor::application::window_info const & wi ) 
{
    motor::application::window_info wil = wi ;

    HINSTANCE hinst = GetModuleHandle(0) ;
    
    static size_t window_number = 0 ;
    motor::string_t class_name = wi.window_name  + " " + motor::to_string( window_number++ ) ;

    HWND hwnd ;
    WNDCLASSA wndclass ;

    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
    wndclass.lpfnWndProc = this_t::StaticWndProc ;
    wndclass.cbClsExtra = 0 ;
    wndclass.cbWndExtra = 0 ;
    wndclass.hInstance = hinst ;
    wndclass.hIcon = LoadIcon(0, IDI_APPLICATION ) ;
    wndclass.hCursor = LoadCursor( 0, IDC_ARROW ) ;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH) ;
    wndclass.lpszMenuName = 0 ;
    wndclass.lpszClassName = class_name.c_str() ;
        
    _name = wi.window_name ;

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
        ShowCursor( wil.show_cursor ) ;
        _is_cursor = wil.show_cursor ;
    }

    // WS_POPUP is said to introduce tearing, so it is removed for now
    if( wil.fullscreen )
    {
        ws_ex_style = WS_EX_APPWINDOW /*& ~(WS_EX_DLGMODALFRAME |
                      WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE )*/ ;
        ws_style = /*WS_POPUP |*/ SW_SHOWNORMAL;
        start_x = start_y = 0 ;        
        width = GetSystemMetrics(SM_CXSCREEN) ;
        height = GetSystemMetrics(SM_CYSCREEN) ;
        _is_fullscreen = true ;
    }
    else
    {
        ws_ex_style = WS_EX_APPWINDOW ;// | WS_EX_WINDOWEDGE ;

        if( wil.borderless )
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
        class_name.c_str(), wil.window_name.c_str(),
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

    _handle = hwnd ;

    ShowWindow( _handle, SW_SHOW ) ;

    return hwnd ;
}

//*******************************************************************************************
void_t window::destroy_window( void_t ) 
{
    if( _handle == NULL ) return ;

    //this_t::unregister_in( _lsn_in ) ;

    SetWindowLongPtr( _handle, GWLP_USERDATA, (LONG_PTR)nullptr ) ;
    DestroyWindow( _handle ) ;
}

//*******************************************************************************************
LRESULT CALLBACK window::StaticWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    this_ptr_t wnd_ptr = (this_ptr_t)GetWindowLongPtr( hwnd, GWLP_USERDATA ) ;
    return wnd_ptr ? wnd_ptr->WndProc(hwnd, msg, wParam, lParam) : DefWindowProc(hwnd, msg, wParam, lParam) ;
}

//*******************************************************************************************
LRESULT CALLBACK window::WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
    case WM_SYSCOMMAND: break ;

    case WM_SHOWWINDOW:
        this_t::send_show( wParam ) ;
        // pass-through
    case WM_SIZE:
        this_t::send_resize( hwnd ) ;
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

    case WM_USER:
        
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
        break ;

    case WM_CLOSE: 
    case WM_DESTROY: 
        this_t::send_destroy() ;
        // post a WM_USER message to the stream so the 
        // win32_carrier knows when a window is closed.
        wParam = WPARAM( -1 ) ;
        PostMessage( hwnd, WM_USER, wParam, lParam ) ;

        return 0 ;
    }

    return DefWindowProc( hwnd, msg, wParam, lParam ) ;
}

//*******************************************************************************************
void_t window::check_for_messages( void_t ) noexcept 
{
    PostMessage( _handle, WM_USER, WPARAM( 1 ), 0 ) ;
}

//*******************************************************************************************
void_t window::show_window(  motor::application::window_info const & wi ) 
{
    if( wi.show ) 
    {
        ShowWindow( _handle, SW_SHOW ) ;
    }
    else 
    {
        ShowWindow( _handle, SW_HIDE ) ;
    }
}

//*******************************************************************************************
void_t window::send_show( WPARAM wparam ) 
{
    #if 0
    motor::application::show_message const amsg { bool_t( wparam == TRUE ) } ;
    this_t::foreach_out( [&] ( motor::application::iwindow_message_listener_mtr_t lst )
    {
        lst->on_visible( amsg ) ;
    } ) ;
    #endif
}

//*******************************************************************************************
void_t window::send_resize( HWND hwnd ) 
{
    RECT rect ;
    GetClientRect( hwnd, &rect ) ;

    motor::application::resize_message const rm {
        true,
        int_t(rect.left), int_t(rect.top), 
        true,
        size_t(rect.right-rect.left), size_t(rect.bottom-rect.top)
    } ;
    #if 0
    this_t::foreach_out( [&] ( motor::application::iwindow_message_listener_mtr_t lst ) 
    { 
        lst->on_resize( rm ) ;
    } ) ;
    #endif
}

//*******************************************************************************************
void_t window::send_destroy( void_t ) 
{
    #if 0
    motor::application::close_message const amsg { true } ;
    this_t::foreach_out( [&] ( motor::application::iwindow_message_listener_mtr_t lst )
    {
        lst->on_close( amsg ) ;
    } ) ;
    #endif
}

#if 0
//***************************************************************************************************************************************************************
void_t window::get_monitor_info( HWND hwnd, MONITORINFO & imon_out ) 
{
    RECT rect ;
    GetClientRect( hwnd, &rect ) ;

    HMONITOR hmon;

    hmon = MonitorFromRect( &rect, MONITOR_DEFAULTTONEAREST ) ;
    imon_out.cbSize = sizeof(imon_out) ;
    GetMonitorInfo( hmon, &imon_out ) ;
}

//***************************************************************************************************************************************************************
void_t window::send_screen_dpi( HWND hwnd ) 
{
    HDC hdc = GetDC( hwnd ) ;
    if( hdc )
    {
        uint_t dpix = GetDeviceCaps( hdc, LOGPIXELSX ) ;
        uint_t dpiy = GetDeviceCaps( hdc, LOGPIXELSY ) ;
        send_screen_dpi( hwnd, dpix, dpiy ) ;
    }
    
}

//***************************************************************************************************************************************************************
void_t window::send_screen_dpi( HWND, uint_t dpix, uint_t dpiy ) 
{
    motor::application::screen_dpi_message const dpim { dpix, dpiy } ;

    for( auto lptr : _msg_listeners )
        lptr->on_screen( dpim ) ;
}

//***************************************************************************************************************************************************************
void_t window::send_screen_size( HWND hwnd ) 
{
    uint_t const width = GetSystemMetrics( SM_CXSCREEN ) ;
    uint_t const height = GetSystemMetrics( SM_CYSCREEN ) ;
    send_screen_size( hwnd, width, height ) ;
}

//***************************************************************************************************************************************************************
void_t window::send_screen_size( HWND, uint_t width, uint_t height ) 
{
    motor::application::screen_size_message msg {
        width,height
    } ;

    for( auto lptr : _msg_listeners )
        lptr->on_screen( msg ) ;
}

#endif
