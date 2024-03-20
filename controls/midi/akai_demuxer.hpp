
#pragma once

#include "../types/midi_controller.hpp"
#include "../components/button.hpp"
#include "../components/slider.hpp"
#include "../components/knob.hpp"
#include "../components/led.hpp"

#include <motor/memory/global.h>

namespace motor
{
    namespace controls
    {
        namespace midi
        {
            //************************************************************************************************
            bool_t create_akai_midimix( motor::controls::midi_device_inout_t dev ) noexcept
            {
                // buttons
                {
                    motor::vector< size_t > ids = 
                    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 } ;

                    for( auto i : ids )
                    {
                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            // press
                            if( msg == motor::controls::midi_message_t( 144, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                b = 1.0f ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 128, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
                                b = 0.0f ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // slider
                {
                    motor::vector< size_t > ids = { 19, 23, 27, 31, 49, 53, 57, 61, 62 } ;
                    for( auto i : ids )
                    {
                        dev.add_input_component<motor::controls::components::slider_t>(
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & c = *reinterpret_cast< motor::controls::components::slider_ptr_t >( cptr ) ;

                            if( msg.compare_s_b1( 176, byte_t( i ) ) )
                            {
                                c = float_t( msg.byte2 ) / 127.0f ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // knobs
                {
                    motor::vector< size_t > ids = { 
                        16, 17, 18, 20, 21, 22, 24, 25, 26, 28, 29, 30, 46, 47, 48, 50, 51, 52, 54, 55, 56, 58, 59, 60 } ;
                    for( auto i : ids )
                    {
                        dev.add_input_component<motor::controls::components::knob_t>(
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & c = *reinterpret_cast< motor::controls::components::knob_ptr_t >( cptr ) ;

                            if( msg.compare_s_b1( 176, byte_t( i ) ) )
                            {
                                c = float_t( msg.byte2 ) / 127.0f ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // output leds
                {
                    motor::vector< size_t > ids = { 
                        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 } ;

                    for( auto i : ids )
                    {
                        dev.add_out_component<motor::controls::components::binary_led_t >(
                        [=]( motor::controls::output_component_ptr_t cptr, motor::controls::midi_message_inout_t msg )
                        {
                            auto & c = *reinterpret_cast< motor::controls::components::binary_led_ptr_t >( cptr ) ;
                            if( c.has_changed() )
                            {
                                byte_t const v = c.value() ? 127 : 0 ;
                                msg = motor::controls::midi_message_t( 144, byte_t( i ), v, 0 ) ;
                                return true ;
                            }
                            return false ;
                        }) ;
                    }
                }

                return true ;
            }

            //************************************************************************************************
            bool_t create_akai_apc_k25( motor::controls::midi_device_inout_t dev ) noexcept
            {
                // buttons
                {
                    for( size_t i=0; i<=39; ++i )
                    {
                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            // press
                            if( msg == motor::controls::midi_message_t( 144, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                b = 1.0f ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 128, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
                                b = 0.0f ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // rouge binary buttons
                {
                    motor::vector< size_t > const ids = 
                    { 64, 65, 66, 67, 68, 69, 70, 71, 82, 83, 84, 85, 86, 91, 93, 98 } ;

                    for( auto i : ids )
                    {
                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            // press
                            if( msg.compare_s_b1( 144, byte_t( i ) ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                b = 1.0f ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 128, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
                                b = 0.0f ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // piano value buttons
                {
                    for( size_t i = 48; i <= 72; ++i )
                    {   
                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            byte_t const value = msg.byte2 ;

                            // press
                            if( msg.compare_s_b1( 145, byte_t( i ) ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                b = float_t(value) / 127.0f ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 129, byte_t( i ), 0, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // knobs
                {
                    for( size_t i=48; i<=55; ++i )
                    {
                        dev.add_input_component<motor::controls::components::knob_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::knob_ptr_t >( cptr ) ;

                            if( msg.compare_s_b1( 176, byte_t( i ) ) )
                            {
                                b = float_t(msg.byte2) / 127.0f ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // multi led colors:
                // 0: off
                // 1: green
                // 2: green blink
                // 3: red
                // 4: red blink
                // 5: yellow 
                // 6: yellow blink

                // multi leds
                {
                    for( size_t i = 0; i <= 39; ++i )
                    {
                        dev.add_out_component<motor::controls::components::multi_led_t >(
                        [=]( motor::controls::output_component_ptr_t cptr, motor::controls::midi_message_inout_t msg )
                        {
                            auto & c = *reinterpret_cast< motor::controls::components::multi_led_ptr_t >( cptr ) ;
                            if( c.has_changed() )
                            {
                                byte_t const v = c.led_color() ;
                                msg = motor::controls::midi_message_t( 144, byte_t( i ), v, 0 ) ;
                                return true ;
                            }
                            return false ;
                        }) ;
                    }
                }

                // output leds
                {
                    motor::vector< size_t > ids = { 64, 81 } ;

                    for( auto i : ids )
                    {
                        dev.add_out_component<motor::controls::components::binary_led_t >(
                        [=]( motor::controls::output_component_ptr_t cptr, motor::controls::midi_message_inout_t msg )
                        {
                            auto & c = *reinterpret_cast< motor::controls::components::binary_led_ptr_t >( cptr ) ;
                            if( c.has_changed() )
                            {
                                byte_t const v = c.value() ? 127 : 0 ;
                                msg = motor::controls::midi_message_t( 144, byte_t( i ), v, 0 ) ;
                                return true ;
                            }
                            return false ;
                        }) ;
                    }
                }

                return true ;
            }

            //************************************************************************************************
            static bool_t demux_akai( motor::string_cref_t name, motor::controls::midi_device_ref_t dev ) noexcept
            {
                if( name == "MIDI Mix" )
                {
                    return motor::controls::midi::create_akai_midimix( dev ) ;
                }
                else if( name == "APC Key 25" )
                {
                    return motor::controls::midi::create_akai_apc_k25( dev ) ;
                }
                return false ;
            }
        }
    }
}