
#pragma once

#include "../layouts/midi_controller.hpp"
#include "../components/button.hpp"
#include "../components/slider.hpp"
#include "../components/knob.hpp"

#include <motor/memory/global.h>

namespace motor
{
    namespace controls
    {
        namespace midi
        {
            motor::controls::midi_device_t create_akai_midimix( motor::controls::midi_device_rref_t dev ) noexcept
            {
                // buttons
                {
                    motor::vector< size_t > ids = 
                    { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 } ;

                    for( auto i : ids )
                    {
                        motor::string_t i_str = motor::to_string( i ) ;

                        dev.add_input_component<motor::controls::components::button_t>( 
                        [=]( motor::controls::input_component_ptr_t cptr, motor::controls::midi_message_cref_t msg )
                        {
                            auto & b = *reinterpret_cast< motor::controls::components::button_ptr_t >( cptr ) ;

                            // press
                            if( msg == motor::controls::midi_message_t( 144, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::pressed ;
                                return true ;
                            }
                            // release
                            else if( msg == motor::controls::midi_message_t( 128, byte_t( i ), 127, 0 ) )
                            {
                                b = motor::controls::components::button_state::released ;
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

                return std::move( dev ) ;
            }
        }
    }
}