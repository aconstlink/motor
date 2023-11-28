
#pragma once

#include "../../imodule.h"
#include "../../layouts/three_mouse.hpp"
#include "../../layouts/ascii_keyboard.hpp"

#include <motor/std/vector>
#include <motor/math/vector/vector2.hpp>

#include <windows.h>

namespace motor
{
    namespace device
    {
        namespace win32
        {
            class MOTOR_DEVICE_API rawinput_module : public imodule
            {
                motor_this_typedefs( rawinput_module ) ;

            private: // mouse typedefs

                typedef motor::device::layouts::three_mouse_t::button three_button_t ;
                typedef motor::device::components::button_state button_state_t ;

                typedef std::pair< three_button_t, button_state_t> mouse_button_item_t ;
                typedef motor::vector< mouse_button_item_t > mouse_button_items_t ;
                typedef motor::vector< int_t > scroll_items_t ;

                typedef motor::vector< motor::math::vec2f_t > pointer_coords_t ;

            private: // mouse data

                mouse_button_items_t _three_button_items ;
                pointer_coords_t _pointer_coords_global ;
                pointer_coords_t _pointer_coords_local ;
                scroll_items_t _scroll_items ;

            private: // keyboard typedefs 

                typedef motor::device::layouts::ascii_keyboard::ascii_key ascii_key_t ;
                typedef motor::device::components::key_state key_state_t ;

                typedef std::pair< ascii_key_t, key_state_t > ascii_keyboard_key_item_t ;
                typedef motor::vector< ascii_keyboard_key_item_t > ascii_keyboard_keys_t ;

            private: // keyboard data 

                ascii_keyboard_keys_t _ascii_keyboard_keys ;

            private: // devices

                motor::device::three_device_mtr_t _three_device ;
                motor::device::ascii_device_mtr_t _ascii_device ;

            private: // raw input specific

                std::mutex _buffer_mtx ;

                UINT _raw_input_data_size = 0u ;
                LPBYTE _raw_input_buffer = nullptr ;

            public:

                rawinput_module( void_t ) noexcept ;
                rawinput_module( this_cref_t ) = delete ;
                rawinput_module( this_rref_t ) noexcept ;
                virtual ~rawinput_module( void_t ) noexcept ;

                this_ref_t operator = ( this_rref_t ) noexcept ;

            public:

                virtual void_t search( motor::device::imodule::search_funk_t ) noexcept ;
                virtual void_t update( void_t ) noexcept ;
                virtual void_t release( void_t ) noexcept ;

            public:

                bool_t handle_input_event( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) ;
            };
            motor_typedef( rawinput_module ) ;
        }
    }
}