#pragma once

#include "../types/midi_controller.hpp"
#include "../components/button.hpp"
#include "../components/slider.hpp"
#include "../components/knob.hpp"
#include "../components/led.hpp"
#include "../components/touch.hpp"

#include <motor/memory/global.h>

namespace motor
{
    namespace controls
    {
        namespace midi
        {
            //************************************************************************************************
            bool_t create_korg_nanopad2( motor::controls::midi_device_inout_t dev ) noexcept
            {
                // buttons
                {
                    motor::vector< size_t > ids = {
                        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 } ;

                    for( auto i : ids )
                    {
                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            byte_t const value = msg.byte2 ;

                            // press
                            if( msg.compare_s_b1( 144, byte_t( i ) ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                b = float_t(value) / 127.0f ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 128, byte_t( i ), 64, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
                                b = 0 ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // pad
                {
                    dev.add_input_component<motor::controls::components::touch_t>( 
                    [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                    {
                        auto & b = *reinterpret_cast< motor::controls::components::touch_ptr_t >( cptr ) ;

                        byte_t const value = msg.byte2 ;

                        // x direction
                        if( msg.compare_s_b1( 176, 1 ) )
                        {
                            b = motor::controls::components::touch_state::pressed ;

                            auto dir = b.value() ;
                            dir.x( (float_t(value) / 127.0f) * 2.0f - 1.0f ) ;
                            b = dir ;
                            
                            return true ;
                        }
                        // y direction
                        else if( msg.compare_s_b1( 176, 2 ) )
                        {
                            b = motor::controls::components::touch_state::pressed ;

                            auto dir = b.value() ;
                            dir.y( (float_t(value) / 127.0f) * 2.0f - 1.0f ) ;
                            b = dir ;

                            return true ;
                        }

                        return false ;
                    } ) ;
                }

                return true ;
            }

            //************************************************************************************************
            bool_t create_korg_nanokontrol2( motor::controls::midi_device_inout_t dev ) noexcept
            {
                // buttons
                {
                    motor::vector< size_t > ids = {
                        // 3 rows
                        32, 33, 34, 35, 36, 37, 38, 39, 48, 49, 50, 51, 52, 53, 54, 55, 64, 65, 66, 67, 68, 69, 70, 71,
                        // left
                        41, 42, 43, 44, 45, 46, 58, 59, 60, 61, 62 } ;

                    for( auto i : ids )
                    {
                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            // press
                            if( msg == motor::controls::midi_message_t( 176, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 176, byte_t( i ), 0, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
                                return true ;
                            }
                            return false ;
                        } ) ;
                    }
                }

                // sliders
                {
                    motor::vector< size_t > ids = { 0, 1, 2, 3, 4, 5, 6, 7 } ;
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
                    motor::vector< size_t > ids = { 16, 17, 18, 19, 20, 21, 22, 23 } ;
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
                        // 3 rows
                        32, 33, 34, 35, 36, 37, 38, 39, 48, 49, 50, 51, 52, 53, 54, 55, 64, 65, 66, 67, 68, 69, 70, 71,
                        // left
                        41, 42, 43, 44, 45, 46, 58, 59, 60, 61, 62 } ;

                    for( auto i : ids )
                    {
                        dev.add_out_component<motor::controls::components::binary_led_t >(
                        [=]( motor::controls::output_component_ptr_t cptr, motor::controls::midi_message_inout_t msg )
                        {
                            auto & c = *reinterpret_cast< motor::controls::components::binary_led_ptr_t >( cptr ) ;
                            if( c.has_changed() )
                            {
                                byte_t const v = c.value() ? 127 : 0 ;
                                msg = motor::controls::midi_message_t( 176, byte_t( i ), v, v ) ;
                                return true ;
                            }
                            return false ;
                        }) ;
                    }
                }

                return true ;
            }

            //************************************************************************************************
            static bool_t demux_korg( motor::string_cref_t name, motor::controls::midi_device_ref_t dev ) noexcept
            {
                if( name == "nanoPAD2" )
                {
                    return motor::controls::midi::create_korg_nanopad2( dev ) ;
                }
                else if( name == "nanoKONTROL2" )
                {
                    return motor::controls::midi::create_korg_nanokontrol2( dev ) ;
                }
                return false ;
            }
        }
    }
}