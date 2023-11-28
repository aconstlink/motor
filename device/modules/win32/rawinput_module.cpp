
#include "rawinput_module.h"

#include <motor/math/vector/vector2.hpp>

#include <windows.h>

using namespace motor::device ;
using namespace motor::device::win32 ;

namespace this_file
{
    typedef motor::device::layouts::three_mouse_t::button three_button_t ;
    typedef motor::device::components::button_state button_state_t ;

    static button_state_t map_raw_button_state_left_to_button_state( USHORT const usButtonFlags )
    {
        if( usButtonFlags & RI_MOUSE_BUTTON_1_UP ) return button_state_t::released ;
        if( usButtonFlags & RI_MOUSE_BUTTON_1_DOWN ) return button_state_t::pressed ;
        return button_state_t::none ;
    }

    static button_state_t map_raw_button_state_right_to_button_state( USHORT const usButtonFlags )
    {
        if( usButtonFlags & RI_MOUSE_BUTTON_2_UP ) return button_state_t::released ;
        if( usButtonFlags & RI_MOUSE_BUTTON_2_DOWN ) return button_state_t::pressed ;
        return button_state_t::none ;
    }

    static button_state_t map_raw_button_state_middle_to_button_state( USHORT const usButtonFlags )
    {
        if( usButtonFlags & RI_MOUSE_BUTTON_3_UP ) return button_state_t::released ;
        if( usButtonFlags & RI_MOUSE_BUTTON_3_DOWN ) return button_state_t::pressed ;
        return button_state_t::none ;
    }

    static three_button_t map_raw_button_left_to_three_button( USHORT const usButtonFlags )
    {
        if( usButtonFlags & RI_MOUSE_BUTTON_1_DOWN ) return three_button_t::left ;
        if( usButtonFlags & RI_MOUSE_BUTTON_1_UP ) return three_button_t::left ;
        return three_button_t::none ;
    }

    static three_button_t map_raw_button_middle_to_three_button( USHORT const usButtonFlags )
    {
        if( usButtonFlags & RI_MOUSE_BUTTON_3_DOWN ) return three_button_t::middle ;
        if( usButtonFlags & RI_MOUSE_BUTTON_3_UP ) return three_button_t::middle ;
        return three_button_t::none ;
    }

    static three_button_t map_raw_button_right_to_three_button( USHORT const usButtonFlags )
    {
        if( usButtonFlags & RI_MOUSE_BUTTON_2_DOWN ) return three_button_t::right ;
        if( usButtonFlags & RI_MOUSE_BUTTON_2_UP ) return three_button_t::right ;
        return three_button_t::none ;
    }
}

namespace this_file
{
    typedef motor::device::layouts::ascii_keyboard::ascii_key ascii_key_t ;
    typedef motor::device::components::key_state key_state_t ;
    typedef motor::device::layouts::ascii_keyboard_t layout_t ;

    static ascii_key_t map_virtual_keycode_to_ascii_key( WPARAM const wparam )
    {
        if( wparam >= 0x30 && wparam <= 0x39 )
        {
            size_t dif = size_t( wparam - 0x30 ) ;
            return layout_t::convert_ascii_number_keys( dif ) ;
        }

        if( wparam >= 0x41 && wparam <= 0x5a )
        {
            size_t dif = size_t( wparam - 0x41 ) ;
            return layout_t::convert_ascii_letter_keys( dif ) ;
        }

        // 0 - 9 on numpad
        if( wparam >= 0x60 && wparam <= 0x69 )
        {
            size_t dif = size_t( wparam - 0x60 ) ;
            return layout_t::convert_ascii_numpad_number_keys( dif ) ;
        }

        // VK_F1 - VK_F12
        if( wparam >= 0x70 && wparam <= 0x7B )
        {
            size_t dif = size_t( wparam - 0x70 ) ;
            return layout_t::convert_ascii_function_keys( dif ) ;
        }

        switch( wparam )
        {
        case VK_BACK: return ascii_key_t::back_space ;
        case VK_TAB: return ascii_key_t::tab ;
        case VK_RETURN: return ascii_key_t::k_return ;
        case VK_ESCAPE: return ascii_key_t::escape ;
        case VK_SPACE: return ascii_key_t::space ;
        case VK_LEFT: return ascii_key_t::arrow_left ;
        case VK_UP: return ascii_key_t::arrow_up ;
        case VK_RIGHT: return ascii_key_t::arrow_right ;
        case VK_DOWN: return ascii_key_t::arrow_down ;
        case VK_SHIFT: return ascii_key_t::shift_left ;
        case VK_LSHIFT: return ascii_key_t::shift_left ;
        case VK_RSHIFT: return ascii_key_t::shift_right ;
        case VK_CONTROL: return ascii_key_t::ctrl_left ;
        case VK_LCONTROL: return ascii_key_t::ctrl_left ;
        case VK_RCONTROL: return ascii_key_t::ctrl_right ;
        case VK_ADD: return ascii_key_t::num_add ;
        case VK_SUBTRACT: return ascii_key_t::num_sub ;
        case VK_MENU: return ascii_key_t::context ;
        case 0xBD: return ascii_key_t::minus ;
        case 0xBB: return ascii_key_t::plus ;

        default: return ascii_key_t::none ;
        }
    }

    /// map with window message
    static key_state_t map_key_state_raw_wm_for_vzt_device( UINT const msg )
    {
        switch( msg )
        {
        case WM_KEYUP: return key_state_t::released;
        case WM_KEYDOWN: return key_state_t::pressed ;
        }
        return key_state_t::none ;
    }
}


//***
rawinput_module::rawinput_module( void_t ) 
{
    RAWINPUTDEVICE rid[ 3 ] ;

    // mouse
    rid[ 0 ].usUsagePage = 0x01 ;
    rid[ 0 ].usUsage = 0x02 ;
    rid[ 0 ].hwndTarget = NULL ;
    rid[ 0 ].dwFlags = 0;//RIDEV_CAPTUREMOUSE | RIDEV_NOLEGACY ;

    // keyboard
    rid[ 1 ].usUsagePage = 0x01 ;
    rid[ 1 ].usUsage = 0x06 ;
    rid[ 1 ].hwndTarget = NULL ;
    rid[ 1 ].dwFlags = 0;//RIDEV_NOLEGACY ;

    // joystick
    rid[ 2 ].usUsagePage = 0x01 ;
    rid[ 2 ].usUsage = 0x04 ;
    rid[ 2 ].hwndTarget = NULL ;
    rid[ 2 ].dwFlags = 0;//RIDEV_NOLEGACY ;

    motor::log::global_t::error(
        RegisterRawInputDevices( rid, 3, sizeof( RAWINPUTDEVICE ) ) == FALSE,
        "[rawinput::initialize] : need raw input support."
    ) ;

    {
        // @todo keyboard
    }

    {
        _three_device = motor::memory::create_ptr( 
            motor::device::three_device_t( "Win32 Rawinput Three Button Mouse" ),
            "RawInput Mouse") ;

        _ascii_device = motor::memory::create_ptr( 
            motor::device::ascii_device_t( "Win32 Rawinput Ascii Keyboard" ),
            "RawInput Keyboard") ;
    }
}

//***
rawinput_module::rawinput_module( this_rref_t rhv ) noexcept
{
    _three_device = motor::move( rhv._three_device ).mtr() ;
    _ascii_device = motor::move( rhv._ascii_device ).mtr() ;
    _three_button_items = std::move( rhv._three_button_items ) ;
    _pointer_coords_global = std::move( rhv._pointer_coords_global ) ;
    _pointer_coords_local = std::move( rhv._pointer_coords_local ) ;
    _scroll_items = std::move( rhv._scroll_items ) ;
}

//***
rawinput_module::this_ref_t rawinput_module::operator = ( this_rref_t rhv ) noexcept
{
    _three_device = motor::move( rhv._three_device ).mtr() ;
    _ascii_device = motor::move( rhv._ascii_device ).mtr() ;
    _three_button_items = std::move( rhv._three_button_items ) ;
    _pointer_coords_global = std::move( rhv._pointer_coords_global ) ;
    _pointer_coords_local = std::move( rhv._pointer_coords_local ) ;
    _scroll_items = std::move( rhv._scroll_items ) ;
    return *this ;
}

//***
rawinput_module::~rawinput_module( void_t )
{
    this_t::release() ;
}

//***
void_t rawinput_module::search( motor::device::imodule::search_funk_t funk )
{
    funk( _three_device ) ;
    funk( _ascii_device ) ;
}

//***
void_t rawinput_module::update( void_t )
{
    // check for plug and play
    // update all devices

    // 1. update components
    {
        _three_device->update() ;
        _ascii_device->update() ;
    }

    // mouse
    {
        motor::device::layouts::three_mouse_t mouse( _three_device ) ;

        std::lock_guard<std::mutex> lk( _buffer_mtx ) ;

        // insert new events
        {
            for( auto const& item : _three_button_items )
            {
                auto* btn = mouse.get_component( item.first ) ;

                *btn = item.second ;
            }

            if( _pointer_coords_global.size() > 0 )
            {
                auto* coord = mouse.get_global_component() ;

                *coord = _pointer_coords_global.back() ;
            }
            else 
            {
                auto* coord = mouse.get_global_component() ;
                *coord = motor::device::components::move_state::unmoved ;
            }

            if( _pointer_coords_local.size() > 0 )
            {
                auto* coord = mouse.get_local_component() ;
                *coord = _pointer_coords_local.back() ;
            }
            else
            {
                auto* coord = mouse.get_local_component() ;
                *coord = motor::device::components::move_state::unmoved ;
            }

            if( _scroll_items.size() > 0 )
            {
                auto* scroll = mouse.get_scroll_component() ;
                
                if( _scroll_items.back() == 65416 ) *scroll = -1.0f ;
                else if( _scroll_items.back() == 120 ) *scroll = 1.0f ;
                else *scroll = 0.0f ;
            }
        }

        _three_button_items.clear() ;
        _pointer_coords_global.clear() ;
        _pointer_coords_local.clear() ;
        _scroll_items.clear() ;
    }

    // keybaord
    {
        motor::device::layouts::ascii_keyboard_t keyboard( _ascii_device ) ;

        std::lock_guard<std::mutex> lk( _buffer_mtx ) ;
        
        {
            for( auto const& item : _ascii_keyboard_keys )
            {
                float_t const value = item.second != motor::device::components::key_state::released ? 1.0f : 0.0f ;
                keyboard.set_value( item.first, value ) ;
                keyboard.set_state( item.first, item.second ) ;
            }
            _ascii_keyboard_keys.clear() ;
        }
    }
}

//*********************************************************************************************************
void_t rawinput_module::release( void_t ) noexcept 
{
    motor::memory::release_ptr( motor::move( _three_device ) ) ;
    motor::memory::release_ptr( motor::move( _ascii_device ) ) ;

    motor::memory::global_t::dealloc( _raw_input_buffer ) ;
    _raw_input_data_size = 0 ;
}

//*********************************************************************************************************
bool_t rawinput_module::handle_input_event( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if( msg != WM_INPUT )
        return false ;

    //
    // 1. get the input data from the api
    //
    UINT sib ;
    GetRawInputData( ( HRAWINPUT ) lParam, RID_INPUT, NULL, &sib, sizeof( RAWINPUTHEADER ) ) ;
    if( sib > _raw_input_data_size )
    {
        motor::memory::global_t::dealloc( _raw_input_buffer ) ;
        _raw_input_buffer = motor::memory::global_t::alloc<BYTE>( sib, "[rawinput_module] : raw_input_buffer" ) ;
        _raw_input_data_size = sib ;
    }

    if( GetRawInputData( ( HRAWINPUT ) lParam, RID_INPUT,
        _raw_input_buffer, &sib, sizeof( RAWINPUTHEADER ) ) != sib )
        return true ;
    
    RAWINPUT * raw = ( RAWINPUT* ) _raw_input_buffer ;

    if( raw->header.dwType == RIM_TYPEMOUSE )
    {
        bool_t outside = false ;

        POINT pt, client_pt ;
        GetCursorPos( &pt ) ;

        client_pt = pt ;
        ScreenToClient( hwnd, &client_pt ) ;

        // check if the event occurred within
        // the client area.
        // there is a problem with rawinput mice.
        // if a window is focused and the user presses 
        // another window, the original focused window does
        // NOT receive a release event! But it will receive the 
        // press event of the newly focused window!
        {
            RECT r ;
            GetClientRect( hwnd, &r ) ;
            motor::math::vec2f_t window_dim(
                float_t( r.right - r.left ),
                float_t( r.bottom - r.top )
            ) ;

            if( client_pt.x < r.left || client_pt.x > r.right ||
                client_pt.y < r.top || client_pt.y > r.bottom )
            {
                outside = true ;
            }
        }

        auto bs = motor::device::components::button_state::none ;
        POINT last_pt = { raw->data.mouse.lLastX, raw->data.mouse.lLastY } ;

        if( raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE )
        {
            bs = ( last_pt.x == 0 && last_pt.y == 0 ) ?
                motor::device::components::button_state::pressed :
                motor::device::components::button_state::none ;
        }
        else if( raw->data.mouse.usFlags == MOUSE_MOVE_ABSOLUTE )
        {
            bs = ( pt.x == last_pt.x && pt.y == last_pt.y ) ?
                motor::device::components::button_state::pressed :
                motor::device::components::button_state::none ;
        }

        std::lock_guard< std::mutex > lk( _buffer_mtx ) ;

        // do buttons and state
        {
            //using so_device::button_state ;
            //using so_device::three_button ;

            // left
            {
                const USHORT bflags = raw->data.mouse.usButtonFlags ;
                const this_t::button_state_t lbs = this_file::map_raw_button_state_left_to_button_state( bflags ) ;
                const this_t::three_button_t tb = this_file::map_raw_button_left_to_three_button( bflags ) ;

                // we check if the mouse is 
                // outside the client area for the time of clicking. if so,
                // we just omit the press event. Because the press event is 
                // issued to the original focused window, and not to the newly one.
                if( lbs == this_t::button_state_t::pressed && !outside )
                {
                    _three_button_items.push_back( mouse_button_item_t( tb, lbs ) ) ;
                }
                else if( lbs == this_t::button_state_t::released )
                {
                    _three_button_items.push_back( mouse_button_item_t( tb, lbs ) ) ;
                }
            }

            // right
            {
                const USHORT bflags = raw->data.mouse.usButtonFlags ;
                const this_t::button_state_t lbs = this_file::map_raw_button_state_right_to_button_state( bflags ) ;
                const this_t::three_button_t tb = this_file::map_raw_button_right_to_three_button( bflags ) ;

                if( lbs == this_t::button_state_t::pressed && !outside )
                {
                    _three_button_items.push_back( mouse_button_item_t( tb, lbs ) ) ;
                }
                else if( lbs == this_t::button_state_t::released )
                {
                    _three_button_items.push_back( mouse_button_item_t( tb, lbs ) ) ;
                }
            }

            // middle
            {
                const USHORT bflags = raw->data.mouse.usButtonFlags ;
                const this_t::button_state_t lbs = this_file::map_raw_button_state_middle_to_button_state( bflags ) ;
                const this_t::three_button_t tb = this_file::map_raw_button_middle_to_three_button( bflags ) ;

                if( lbs == this_t::button_state_t::pressed && !outside )
                {
                    _three_button_items.push_back( mouse_button_item_t( tb, lbs ) ) ;
                }
                else if( lbs == this_t::button_state_t::released )
                {
                    _three_button_items.push_back( mouse_button_item_t( tb, lbs ) ) ;
                }
            }

            // wheel
            {
                const USHORT bflags = raw->data.mouse.usButtonFlags ;
                if( bflags == RI_MOUSE_WHEEL )
                {
                    //so_log::global_t::status( std::to_string( short_t(raw->data.mouse.usButtonData) ) ) ;
                    auto const v = raw->data.mouse.usButtonData ;
                    _scroll_items.push_back( v ) ;
                }
            }
        }

        // do global
        {
            motor::math::vec2f_t const dim(
                float_t( GetSystemMetrics( SM_CXSCREEN ) ),
                float_t( GetSystemMetrics( SM_CYSCREEN ) )
            ) ;

            motor::math::vec2f_t const v = motor::math::vec2f_t( 
                float_t( pt.x ), float_t( dim.y() - pt.y ) ) / dim ;
            _pointer_coords_global.push_back( v ) ;
        }

        // do local
        {
            RECT r ;
            GetClientRect( hwnd, &r ) ;
            motor::math::vec2f_t const dim(
                float_t( r.right - r.left ),
                float_t( r.bottom - r.top )
            ) ;

            motor::math::vec2f_t const v = motor::math::vec2f_t( 
                float_t( client_pt.x ), float_t( dim.y() - client_pt.y ) ) / dim ;
            _pointer_coords_local.push_back( v ) ;
        }

        return true ;
    }
    else if( raw->header.dwType == RIM_TYPEKEYBOARD )
    {
        std::lock_guard< std::mutex > lk( _buffer_mtx ) ;

        _ascii_keyboard_keys.push_back( ascii_keyboard_key_item_t(
            this_file::map_virtual_keycode_to_ascii_key( raw->data.keyboard.VKey ),
            this_file::map_key_state_raw_wm_for_vzt_device( raw->data.keyboard.Message )
        ) ) ;

        return true ;
    }

    return true ;
}