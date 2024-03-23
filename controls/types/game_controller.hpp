
#pragma once

#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/stick.hpp"
#include "../components/vibrator.hpp"

#include "../device.hpp"

namespace motor
{
    namespace controls
    {
        namespace types
        {
            // logical device layout
            class game_controller
            {
                motor_this_typedefs( game_controller ) ;

            private:

                motor::controls::device_borrow_t::mtr_t _dev ;

            public:

                enum class input_component
                {
                    // 1d 
                    jump,
                    crouch,
                    shoot,
                    interact,

                    pause,
                    menu,
                    home,

                    action_a,
                    action_b,
                    action_c,
                    action_d,
                    action_e,
                    action_f,
                    action_g,
                    action_h,
                    action_i,

                    // 2d
                    movement,
                    aim,

                    num_components
                };

                static size_t as_id( input_component const ic ) noexcept { return size_t( ic ) ; }

                enum class output_component
                {
                    vibration_a,
                    vibration_b,
                    num_components
                };

                static size_t as_id( output_component const oc ) noexcept { return size_t( oc ) ; }

            public:

                static void_t init_components( motor::controls::device_inout_t dev )
                {
                    // inputs

                    // buttons
                    dev.add_input_component<motor::controls::components::button_t>() ; // jump
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ;
                    dev.add_input_component<motor::controls::components::button_t>() ; // action_i

                    // sticks
                    dev.add_input_component<motor::controls::components::stick_t>() ; // movement
                    dev.add_input_component<motor::controls::components::stick_t>() ; // aim
                   

                    // outputs
                    dev.add_output_component<motor::controls::components::vibrator_t>() ; // vibration_a
                    dev.add_output_component<motor::controls::components::vibrator_t>() ; // vibration_b
                }

            private:

                motor::controls::components::button_ptr_t comp_button( this_t::input_component const ic ) const noexcept
                {
                    return _dev->get_in_component<motor::controls::components::button_t>( this_t::as_id( ic ) ) ;
                }

            public:

                game_controller( motor::controls::device_borrow_t::mtr_t dev ) noexcept : 
                    _dev( dev )
                {
                    assert( _dev != nullptr ) ;
                }

                game_controller( this_cref_t rhv ) noexcept : _dev( rhv._dev ) {}
                game_controller( this_rref_t rhv ) noexcept : _dev( rhv._dev ) {}
                ~game_controller( void_t ) noexcept {}

            public:

                enum class button
                {
                    jump,
                    crouch,
                    shoot,
                    interact,

                    pause,
                    menu,
                    home,

                    action_a,
                    action_b,
                    action_c,
                    action_d,
                    action_e,
                    action_f,
                    action_g,
                    action_h,
                    action_i,

                    num_buttons
                };

                static motor::string_t to_string( this_t::button const b ) noexcept
                {
                    static char_t const * const __names[] = { "jump", "crouch", "shoot", "interact", "pause", 
                        "menu", "home", "action_a", "action_b", "action_c", "action_d", "action_e", "action_f", "action_g",
                        "action_h", "action_i", "invalid" } ;
                    return motor::string_t( size_t( b ) >= size_t( button::num_buttons ) ? __names[ size_t( button::num_buttons ) ] : __names[ size_t( b ) ] ) ;
                }

                enum class directional
                {
                    movement, aim, num_dirs
                };
                static motor::string_t to_string( this_t::directional const b ) noexcept
                {
                    static char_t const * const __names[] = { "movement", "aim", "invalid" } ;
                    return size_t( b ) >= size_t( directional::num_dirs ) ? __names[ size_t( directional::num_dirs ) ] : __names[ size_t( b ) ] ;
                }

                enum class vibration
                {
                    a, b, num_vibs
                };
                static motor::string_t to_string( this_t::vibration const b ) noexcept
                {
                    static char const * const __names[] = { "a", "b", "invalid" } ;
                    return size_t( b ) >= size_t( vibration::num_vibs ) ? __names[ size_t( vibration::num_vibs ) ] : __names[ size_t( b ) ] ;
                }

            public: // is

                bool_t is( this_t::button const b, motor::controls::components::button_state const bs, float_out_t value ) const noexcept
                {
                    motor::controls::components::button_ptr_t btn = nullptr ;

                    switch( b )
                    {
                    case this_t::button::jump: btn = this_t::comp_button( this_t::input_component::jump ) ; break ;
                    case this_t::button::crouch: btn = this_t::comp_button( this_t::input_component::crouch ) ; break ;
                    case this_t::button::shoot: btn = this_t::comp_button( this_t::input_component::shoot ) ; break ;
                    case this_t::button::interact: btn = this_t::comp_button( this_t::input_component::interact ) ; break ;
                    case this_t::button::pause: btn = this_t::comp_button( this_t::input_component::pause ) ; break ;
                    case this_t::button::menu: btn = this_t::comp_button( this_t::input_component::menu ) ; break ;
                    case this_t::button::home: btn = this_t::comp_button( this_t::input_component::home ) ; break ;
                    case this_t::button::action_a: btn = this_t::comp_button( this_t::input_component::action_a ) ; break ;
                    case this_t::button::action_b: btn = this_t::comp_button( this_t::input_component::action_b ) ; break ;
                    case this_t::button::action_c: btn = this_t::comp_button( this_t::input_component::action_c ) ; break ;
                    case this_t::button::action_d: btn = this_t::comp_button( this_t::input_component::action_d ) ; break ;
                    case this_t::button::action_e: btn = this_t::comp_button( this_t::input_component::action_e ) ; break ;
                    case this_t::button::action_f: btn = this_t::comp_button( this_t::input_component::action_f ) ; break ;
                    case this_t::button::action_g: btn = this_t::comp_button( this_t::input_component::action_g ) ; break ;
                    case this_t::button::action_h: btn = this_t::comp_button( this_t::input_component::action_h ) ; break ;
                    case this_t::button::action_i: btn = this_t::comp_button( this_t::input_component::action_i ) ; break ;
                    default: break ;
                    }
                    if( btn == nullptr ) return false ;
                    if( btn->state() != bs ) return false ;
                     
                    value = btn->value() ;

                    return true ;
                }

                bool_t is( this_t::directional const d, motor::controls::components::stick_state const bs, motor::math::vec2f_out_t value ) const noexcept
                {
                    using comp_t = motor::controls::components::stick_t ;
                    comp_t * comp = nullptr ;

                    switch( d )
                    {
                    case this_t::directional::movement: comp = _dev->get_in_component<comp_t>( this_t::as_id(this_t::input_component::movement) ) ; break ;
                    case this_t::directional::aim: comp = _dev->get_in_component<comp_t>( this_t::as_id(this_t::input_component::aim) ) ; break ;
                    
                    default: break ;
                    }
                    if( comp == nullptr ) return false ;
                    if( comp->state() != bs ) return false ;

                    value = comp->value() ;

                    return true ;
                }

            public: // vibration

                bool_t is( this_t::vibration const b, motor::controls::components::vibrator_state const bs, float_out_t value ) const noexcept
                {
                    using comp_t = motor::controls::components::vibrator_t ;
                    comp_t* comp = nullptr ;

                    switch( b )
                    {
                    case this_t::vibration::a: comp = _dev->get_out_component<comp_t>( this_t::as_id( this_t::output_component::vibration_a ) ) ; break ;
                    case this_t::vibration::b: comp = _dev->get_out_component<comp_t>( this_t::as_id( this_t::output_component::vibration_b ) ) ; break ;

                    default: break ;
                    }
                    if( comp == nullptr ) return false ;
                    if( comp->state() != bs ) return false ;

                    value = comp->value() ;

                    return true ;
                }

                bool_t set( this_t::vibration const v, float_t const value ) noexcept
                {
                    using comp_t = motor::controls::components::vibrator_t ;
                    comp_t* comp = nullptr ;

                    switch( v )
                    {
                    case this_t::vibration::a: comp = _dev->get_out_component<comp_t>( this_t::as_id( this_t::output_component::vibration_a ) ) ; break ;
                    case this_t::vibration::b: comp = _dev->get_out_component<comp_t>( this_t::as_id( this_t::output_component::vibration_b ) ) ; break ;

                    default: break ;
                    }
                    if( comp == nullptr ) return false ;
                    *comp = value ;

                    return true ;
                }
            };
            motor_typedef( game_controller ) ;
        }
        motor_typedefs( motor::controls::device_with< motor::controls::types::game_controller >, game_device ) ;
    }
}