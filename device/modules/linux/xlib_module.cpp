#include "xlib_module.h"

#include <motor/math/vector/vector2.hpp>
#include <X11/keysym.h>

using namespace motor::device ;
using namespace motor::device::xlib ;

namespace this_file
{
    typedef motor::device::layouts::ascii_keyboard::ascii_key ascii_key_t ;
    typedef motor::device::components::key_state key_state_t ;
    typedef motor::device::layouts::ascii_keyboard_t layout_t ;

    static ascii_key_t map_virtual_keycode_to_ascii_key( uint_t const wparam )
    {
        if( wparam >= XK_0 && wparam <= XK_9 )
        {
            size_t const dif = size_t( wparam - XK_0 ) ;
            return layout_t::convert_ascii_number_keys( dif ) ;
        }

        if( wparam >= XK_a && wparam <= XK_z )
        {
            size_t dif = size_t( wparam - XK_a ) ;
            return layout_t::convert_ascii_letter_keys( dif ) ;
        }

        // 0 - 9 on numpad
        if( wparam >= XK_KP_0 && wparam <= XK_KP_9 )
        {
            size_t dif = size_t( wparam - XK_KP_0 ) ;
            return layout_t::convert_ascii_numpad_number_keys( dif ) ;
        }

        if( wparam >= XK_F1 && wparam <= XK_F14 )
        {
            size_t dif = size_t( wparam - XK_F1 ) ;
            return layout_t::convert_ascii_function_keys( dif ) ;
        }

        switch( wparam )
        {
        case XK_BackSpace: return ascii_key_t::back_space ;
        case XK_Tab: return ascii_key_t::tab ;
        case XK_Return: return ascii_key_t::k_return ;
        case XK_Escape: return ascii_key_t::escape ;
        case XK_space: return ascii_key_t::space ;
        case XK_Left: return ascii_key_t::arrow_left ;
        case XK_Up: return ascii_key_t::arrow_up ;
        case XK_Right: return ascii_key_t::arrow_right ;
        case XK_Down: return ascii_key_t::arrow_down ;
        case XK_Shift_L: return ascii_key_t::shift_left ;
        case XK_Shift_R: return ascii_key_t::shift_right ;
        case XK_Control_L: return ascii_key_t::ctrl_left ;
        case XK_Control_R: return ascii_key_t::ctrl_right ;
        case XK_KP_Add: return ascii_key_t::num_add ;
        case XK_KP_Subtract: return ascii_key_t::num_sub ;
        case XK_Menu: return ascii_key_t::context ;
        case XK_minus: return ascii_key_t::minus ;
        case XK_plus: return ascii_key_t::plus ;

        default: return ascii_key_t::none ;
        }
    }
}

xlib_module::xlib_module( void_t ) 
{
    // init devices in xlib?
    motor::log::global_t::status("xlib module online" ) ;

    {
        _three_device = motor::memory::create_ptr( motor::device::three_device_t( "Xlib Three Button Mouse" ),
            "[xlib_module] : three button mouse" );
        _ascii_device = motor::memory::create_ptr( motor::device::ascii_device_t( "Xlib Ascii Keyboard" ),
            "[xlib_module] : ascii keyboard"  ) ;
    }
}

//************************************************************
xlib_module::xlib_module( this_rref_t rhv )
{
    _three_device = motor::move( rhv._three_device ).mtr() ;
    _ascii_device = motor::move( rhv._ascii_device ).mtr() ;
    _three_button_items = std::move( rhv._three_button_items ) ;
    _pointer_coords_global = std::move( rhv._pointer_coords_global ) ;
    _pointer_coords_local = std::move( rhv._pointer_coords_local ) ;
    _scroll_items = std::move( rhv._scroll_items ) ;
}

//************************************************************
xlib_module::this_ref_t xlib_module::operator = ( this_rref_t rhv ) 
{
    _three_device = motor::move( rhv._three_device ).mtr() ;
    _ascii_device = motor::move( rhv._ascii_device ).mtr() ;
    _three_button_items = ::std::move( rhv._three_button_items ) ;
    _pointer_coords_global = ::std::move( rhv._pointer_coords_global ) ;
    _pointer_coords_local = ::std::move( rhv._pointer_coords_local ) ;
    _scroll_items = ::std::move( rhv._scroll_items ) ;
    return *this ;
}

//************************************************************
xlib_module::~xlib_module( void_t )
{
    this_t::release() ;
}

//************************************************************
void_t xlib_module::search( motor::device::imodule::search_funk_t funk ) noexcept
{
    funk( _three_device ) ;
    funk( _ascii_device ) ;
}

//************************************************************
void_t xlib_module::update( void_t ) noexcept
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

        std::lock_guard< std::mutex > lk( _buffer_mtx ) ;

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
                
                if( _scroll_items.back() == -1 ) *scroll = -1.0f ;
                else if( _scroll_items.back() == 1 ) *scroll = 1.0f ;
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

        std::lock_guard< std::mutex > lk( _buffer_mtx ) ;
        
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

//************************************************************
void_t xlib_module::release( void_t ) noexcept
{
    motor::memory::release_ptr( motor::move(_three_device) ) ;
    motor::memory::release_ptr( motor::move(_ascii_device) ) ;
}

//************************************************************
bool_t xlib_module::handle_input_event( XEvent const & event )
{
    auto bs = motor::device::components::button_state::none ;
    auto ks = motor::device::components::key_state::none ;

    switch( event.type )
    {
    case MotionNotify:
        {
            XMotionEvent const & ev = event.xmotion ;

            XWindowAttributes wa ;
            XGetWindowAttributes( ev.display, ev.window, &wa ) ;

            // do global
            {
                std::lock_guard< std::mutex > lk( _buffer_mtx ) ;

                motor::math::vec2f_t const dim(
                     float_t( WidthOfScreen(wa.screen) ),
                     float_t( HeightOfScreen(wa.screen) ) ) ;

                motor::math::vec2f_t const v = motor::math::vec2f_t( 
                     float_t( ev.x_root ), float_t( dim.y() - ev.y_root ) ) / dim ;
                _pointer_coords_global.push_back( v ) ;
            }

            // do local
            {
                std::lock_guard< std::mutex > lk( _buffer_mtx ) ;

                motor::math::vec2f_t const dim(
                     float_t( wa.width ),
                     float_t( wa.height )) ;

                motor::math::vec2f_t const v = motor::math::vec2f_t( 
                    float_t( ev.x ), float_t( dim.y() - ev.y ) ) / dim ;
                _pointer_coords_local.push_back( v ) ;
            }
        }
        break;

    case ButtonPress:
        bs = motor::device::components::button_state::pressed ;
        break ;

    case ButtonRelease:
        bs = motor::device::components::button_state::released ;
        break ;

    case KeyPress:
        ks = motor::device::components::key_state::pressed ;
        break ;

    case KeyRelease:
        ks = motor::device::components::key_state::released ;
        break ;
    }

    if( bs != motor::device::components::button_state::none )
    {
        XButtonEvent const & ev = event.xbutton ;

        this_t::three_button_t tb = this_t::three_button_t::none ;

        if( ev.button == 1 ) tb = this_t::three_button_t::left ;
        else if( ev.button == 2 ) tb = this_t::three_button_t::right ;
        else if( ev.button == 3 ) tb = this_t::three_button_t::middle ;

        if( tb != this_t::three_button_t::none )
        {
            std::lock_guard< std::mutex > lk( _buffer_mtx ) ;
            _three_button_items.push_back( mouse_button_item_t( tb, bs ) ) ;
        }

        if( (ev.button == 4 || ev.button == 5) && 
            bs == motor::device::components::button_state::released )
        {
            std::lock_guard< std::mutex > lk( _buffer_mtx ) ;
            int_t const v = ev.button == 4 ? 1 : -1 ;
            _scroll_items.push_back( v ) ;
        }
    }

    if( ks != motor::device::components::key_state::none )
    {
        XKeyEvent const & ev = event.xkey ;

        auto const keysym = XKeycodeToKeysym( ev.display, ev.keycode, 0 ) ;
        if( keysym != NoSymbol )
        {
            this_file::map_virtual_keycode_to_ascii_key( keysym ) ;

            std::lock_guard< std::mutex > lk( _buffer_mtx ) ;

        _ascii_keyboard_keys.push_back( ascii_keyboard_key_item_t(
            this_file::map_virtual_keycode_to_ascii_key( keysym ), ks ) ) ;
        }

    }

    return true ;
}
