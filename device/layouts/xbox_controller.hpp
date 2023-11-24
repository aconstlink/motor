#pragma once

#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/stick.hpp"
#include "../components/vibrator.hpp"

#include "../device.hpp"

#include <motor/memory/global.h>
#include <motor/std/string>

namespace motor
{
    namespace device
    {
        namespace layouts
        {
            // physical device layout
            class xbox_controller
            {
                motor_this_typedefs( xbox_controller ) ;
                motor_typedefs( motor::device::device_tpl< this_t >, this_device ) ;

                friend this_device_t ;

            public:

                enum class input_component
                {
                    button_back,            // XINPUT_GAMEPAD_BACK
                    button_start,           // XINPUT_GAMEPAD_START
                    button_a,               // XINPUT_GAMEPAD_A
                    button_b,               // XINPUT_GAMEPAD_B
                    button_x,               // XINPUT_GAMEPAD_X
                    button_y,               // XINPUT_GAMEPAD_Y
                    button_thumb_left,      // XINPUT_GAMEPAD_LEFT_THUMB
                    button_thumb_right,     // XINPUT_GAMEPAD_RIGHT_THUMB
                    button_shoulder_left,   // XINPUT_GAMEPAD_LEFT_SHOULDER
                    button_shoulder_right,  // XINPUT_GAMEPAD_RIGHT_SHOULDER

                    button_dpad_up,         // XINPUT_GAMEPAD_DPAD_UP
                    button_dpad_down,       // XINPUT_GAMEPAD_DPAD_DOWN
                    button_dpad_left,       // XINPUT_GAMEPAD_DPAD_LEFT
                    button_dpad_right,      // XINPUT_GAMEPAD_DPAD_RIGHT

                    stick_left,
                    stick_right,

                    trigger_left,
                    trigger_right,

                    num_components
                };

                enum class output_component
                {
                    motor_left,
                    motor_right,
                    num_components
                };

            private:

                static void_t init_components( this_t::this_device_ref_t dev )
                {
                    // inputs

                    // buttons
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;

                    // sticks
                    dev.add_input_component<motor::device::components::stick_t>() ;
                    dev.add_input_component<motor::device::components::stick_t>() ;

                    // triggers
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;

                    // outputs
                    dev.add_output_component<motor::device::components::vibrator_t>() ;
                    dev.add_output_component<motor::device::components::vibrator_t>() ;
                }

            public: 

                motor::device::components::button_ptr_t comp_button( input_component const ic ) const noexcept
                {
                    return _dev->get_component<motor::device::components::button_t>( ic ) ;
                }

            private:

                // non-managed!
                this_t::this_device_ptr_t _dev ;

            public:

                xbox_controller( this_t::this_device_ptr_t dev ) noexcept : _dev( dev )
                {
                    assert( dev != nullptr ) ;
                }
                xbox_controller( this_cref_t rhv ) noexcept : _dev( rhv._dev ) {}
                xbox_controller( this_rref_t rhv ) noexcept : _dev( rhv._dev ) {}
                ~xbox_controller( void_t ) noexcept {}

            public:

                enum class button
                {
                    none, back, start, a, b, x, y
                };
                
                static motor::string_t to_string( button const b ) noexcept
                {
                    static char_t const * const __names[] = { "none", "back", "start", "a", "b", "x", "y", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( button::y ) ? __names[ size_t( button::y ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class thumb
                {
                    none, left, right
                };

                static motor::string_t to_string( thumb const b ) noexcept
                {
                    static char_t const * const __names[] = { "none", "left", "right", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( thumb::right ) ? __names[ size_t( thumb::right ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class shoulder
                {
                    none, left, right
                };
                static motor::string_t to_string( shoulder const b ) noexcept
                {
                    static char const * const __names[] = { "none", "left", "right", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( shoulder::right ) ? __names[ size_t( shoulder::right ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class stick
                {
                    none, left, right 
                };
                static motor::string_t to_string( stick const b ) noexcept
                {
                    static char_t const * const __names[] = { "none", "left", "right", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( stick::right ) ? __names[ size_t( stick::right ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class trigger
                {
                    none, left, right
                };
                static motor::string_t to_string( trigger const b ) noexcept
                {
                    static char const * const __names[] = { "none", "left", "right", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( trigger::right ) ? __names[ size_t( trigger::right ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class dpad
                {
                    none, left, right, up, down
                };
                static motor::string_t to_string( dpad const b ) noexcept
                {
                    static char_t const * const __names[] = { "none", "left", "right", "up", "down", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( dpad::down ) ? __names[ size_t( dpad::down ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class vibrator
                {
                    none, left, right
                };
                static motor::string_t to_string( vibrator const b ) noexcept
                {
                    static char_t const * const __names[] = { "none", "left", "right", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( vibrator::right ) ? __names[ size_t( vibrator::right ) ] : __names[ size_t( b ) ] ) ;
                }

            public: // is functions


                bool_t is( this_t::button const b, motor::device::components::button_state const bs ) const noexcept
                {
                    return this_t::get_component(b)->state() == bs ;
                }

                bool_t is( this_t::thumb const b, motor::device::components::button_state const bs ) const noexcept
                {
                    return this_t::get_component(b)->state() == bs ;
                }

                bool_t is( this_t::shoulder const b, motor::device::components::button_state const bs ) const noexcept
                {
                    return this_t::get_component(b)->state() == bs ;
                }

                bool_t is( this_t::dpad const b, motor::device::components::button_state const bs ) const noexcept
                {
                    return this_t::get_component(b)->state() == bs ;
                }

                bool_t is( this_t::trigger const b, motor::device::components::button_state const bs, float_out_t v ) const noexcept
                {
                    auto* comp = this_t::get_component( b ) ;

                    v = comp->value() ;
                    return comp->state() == bs ;
                }


                bool_t is( this_t::vibrator const b, motor::device::components::vibrator_state const bs, float_out_t v ) const noexcept
                {
                    auto* comp = this_t::get_component( b ) ;

                    v = comp->value() ;
                    return comp->state() == bs ;
                }

            public: // stick

                bool_t is( this_t::stick const s, motor::device::components::stick_state const ss, motor::math::vec2f_out_t value ) const noexcept
                {
                    if( this_t::get_component(s)->state() == ss )
                    {
                        value = this_t::get_component( s )->value() ;
                        return true ;
                    }

                    return false ;
                }

            public: // motor

                this_ref_t set_motor( this_t::vibrator const m, float_t const v ) noexcept 
                {
                    *this_t::get_component( m ) = v ;

                    return *this ;
                }

            public: // get component

                motor::device::components::button_ptr_t get_component( this_t::button const b ) const noexcept
                {
                    this_t::input_component ic = this_t::input_component::num_components ;

                    switch( b )
                    {
                    case this_t::button::back: ic = this_t::input_component::button_back ; break ;
                    case this_t::button::start: ic = this_t::input_component::button_start ; break ;
                    case this_t::button::a: ic = this_t::input_component::button_a ; break ;
                    case this_t::button::b: ic = this_t::input_component::button_b ; break ;
                    case this_t::button::x: ic = this_t::input_component::button_x ; break ;
                    case this_t::button::y: ic = this_t::input_component::button_y ; break ;
                    default: break ;
                    }

                    if( ic == this_t::input_component::num_components ) return nullptr ;

                    return this_t::comp_button( ic );
                }

                motor::device::components::button_ptr_t get_component( this_t::thumb const b ) const noexcept
                {
                    this_t::input_component ic = this_t::input_component::num_components ;

                    switch( b )
                    {
                    case this_t::thumb::left: ic = this_t::input_component::button_thumb_left ; break ;
                    case this_t::thumb::right: ic = this_t::input_component::button_thumb_right ; break ;
                    default: break ;
                    }

                    if( ic == this_t::input_component::num_components ) return nullptr ;

                    return this_t::comp_button( ic ) ;
                }

                motor::device::components::button_ptr_t get_component( this_t::shoulder const b ) const noexcept
                {
                    this_t::input_component ic = this_t::input_component::num_components ;

                    switch( b )
                    {
                    case this_t::shoulder::left: ic = this_t::input_component::button_shoulder_left ; break ;
                    case this_t::shoulder::right: ic = this_t::input_component::button_shoulder_right ; break ;
                    default: break ;
                    }

                    if( ic == this_t::input_component::num_components ) return nullptr ;

                    return this_t::comp_button( ic ) ;
                }

                motor::device::components::button_ptr_t get_component( this_t::dpad const b ) const noexcept
                {
                    this_t::input_component ic = this_t::input_component::num_components ;

                    switch( b )
                    {
                    case this_t::dpad::left: ic = this_t::input_component::button_dpad_left ; break ;
                    case this_t::dpad::right: ic = this_t::input_component::button_dpad_right ; break ;
                    case this_t::dpad::up: ic = this_t::input_component::button_dpad_up ; break ;
                    case this_t::dpad::down: ic = this_t::input_component::button_dpad_down ; break ;
                    default: break ;
                    }

                    if( ic == this_t::input_component::num_components ) return nullptr ;

                    return this_t::comp_button( ic ) ;
                }

                motor::device::components::button_ptr_t get_component( this_t::trigger const b ) const noexcept
                {
                    this_t::input_component ic = this_t::input_component::num_components ;

                    switch( b )
                    {
                    case this_t::trigger::left: ic = this_t::input_component::trigger_left ; break ;
                    case this_t::trigger::right: ic = this_t::input_component::trigger_right ; break ;
                    default: break ;
                    }

                    if( ic == this_t::input_component::num_components ) return nullptr ;

                    return this_t::comp_button( ic ) ;
                }

                motor::device::components::stick_ptr_t get_component( this_t::stick const s ) const noexcept
                {
                    this_t::input_component ic = this_t::input_component::num_components ;

                    switch( s )
                    {
                    case this_t::stick::left: ic = this_t::input_component::stick_left ; break ;
                    case this_t::stick::right: ic = this_t::input_component::stick_right ; break ;
                    default: break ;
                    }

                    return _dev->get_component<motor::device::components::stick_t>( ic ) ;
                }

                motor::device::components::vibrator_ptr_t get_component( this_t::vibrator const s ) const noexcept
                {
                    this_t::output_component ic = this_t::output_component::num_components ;

                    switch( s )
                    {
                    case this_t::vibrator::left: ic = this_t::output_component::motor_left ; break ;
                    case this_t::vibrator::right: ic = this_t::output_component::motor_right ; break ;
                    default: break ;
                    }

                    return _dev->get_component<motor::device::components::vibrator_t>( ic ) ;
                }

            } ;
            motor_typedef( xbox_controller ) ;
        }
        motor_typedefs( motor::device::device_tpl< motor::device::layouts::xbox_controller_t>, xbc_device ) ;
    }
}
