
#pragma once

#include "../../typedefs.h"
#include "../../types/midi_controller.hpp"
#include "../../components/button.hpp"
#include "../../components/button.hpp"
#include "../../components/slider.hpp"
#include "../../components/knob.hpp"
#include "../../components/led.hpp"



namespace motor { namespace controls { namespace midi { namespace layouts
{
    //***************************************************************************************************
    class akai_midi_mix
    {
        motor_this_typedefs( akai_midi_mix ) ;

    private:

        motor::controls::midi_device_borrow_t::mtr_t _dev = nullptr ;

    public:

        akai_midi_mix( motor::controls::midi_device_borrow_t::mtr_t ptr ) noexcept : _dev( ptr ) 
        {
            assert( ptr != nullptr ) ;
        }

    public:

        enum class button : size_t 
        {
            button_1 = 0, 
            button_2, 
            button_3, 
            button_4, 
            button_5, 
            button_6, 
            button_7, 
            button_8, 
            button_9, 
            button_10, 
            button_11, 
            button_12, 
            button_13, 
            button_14, 
            button_15, 
            button_16, 
            button_17, 
            button_18, 
            button_19,
            num_buttons
        };

        enum class slider : size_t 
        {
            slider_1 = static_cast<size_t>(this_t::button::num_buttons), 
            slider_2, 
            slider_3, 
            slider_4, 
            slider_5, 
            slider_6, 
            slider_7, 
            slider_8, 
            slider_9,
            num_sliders
        };

        enum class knob : size_t 
        {
            knob_1 = static_cast<size_t>(this_t::slider::num_sliders), 
            knob_2, 
            knob_3, 
            knob_4, 
            knob_5, 
            knob_6, 
            knob_7,
            knob_8, 
            knob_9, 
            knob_10, 
            knob_11, 
            knob_12, 
            knob_13, 
            knob_14, 
            knob_15, 
            knob_16, 
            knob_17, 
            knob_18, 
            knob_19, 
            knob_20, 
            knob_21, 
            knob_22, 
            knob_23, 
            knob_24,
            num_knobs
        };

        enum class led : size_t 
        {
            led_1 = 0, 
            led_2, 
            led_3, 
            led_4, 
            led_5, 
            led_6, 
            led_7, 
            led_8, 
            led_9, 
            led_10, 
            led_11, 
            led_12, 
            led_13, 
            led_14, 
            led_15, 
            led_16, 
            led_17, 
            led_18, 
            led_19, 
            led_20, 
            led_21, 
            led_22, 
            led_23, 
            led_24
        } ;

        //*************************************************************************************
        motor::controls::components::button_state get_state( this_t::button const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::button_t>( 
                static_cast< size_t >( e ) )->state() ;
        }

        //*************************************************************************************
        float_t get_value( this_t::slider const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::slider_t>( 
                static_cast< size_t >( e ) )->value() ;
        }

        //*************************************************************************************
        bool_t has_changed( this_t::slider const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::slider_t>( 
                static_cast< size_t >( e ) )->has_changed() ;
        }

        //*************************************************************************************
        std::tuple<bool_t, bool_t> get_state( this_t::slider const e ) const noexcept
        {
            return { this_t::has_changed(e), this_t::get_value( e ) } ;
        }

        //*************************************************************************************
        float_t get_value( this_t::knob const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::knob_t>( 
                static_cast< size_t >( e ) )->value() ;
        }

        //*************************************************************************************
        bool_t has_changed( this_t::knob const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::knob_t>( 
                static_cast< size_t >( e ) )->has_changed() ;
        }

        //*************************************************************************************
        std::tuple<bool_t, bool_t> get_state( this_t::knob const e ) const noexcept
        {
            return { this_t::has_changed(e), this_t::get_value( e ) } ;
        }

        //*************************************************************************************
        void_t set( this_t::led const e, bool_t const onoff ) noexcept
        {
            _dev->get_out_component<motor::controls::components::binary_led_t>( 
                static_cast< size_t >( e ) )->operator= ( onoff ) ;
        }

        //*************************************************************************************
        // @return [ changed, plugged ]
        std::tuple<bool_t, bool_t> plug_state( void_t ) const noexcept
        {
            return _dev->plug_state() ;
        }

        //*************************************************************************************
        static bool_t create_layout( motor::string_cref_t name, motor::controls::midi_device_inout_t dev ) noexcept
        {
            if( name != "MIDI Mix" )
            {
                return false ;
            }

            // buttons
            {
                motor::vector< size_t > ids = 
                { 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 16, 18, 19, 21, 22, 24, 25, 26, 27 } ;

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
    };
    motor_typedef( akai_midi_mix ) ;

    //***************************************************************************************************
    class akai_apc_key25
    {
        motor_this_typedefs( akai_apc_key25 ) ;

    private:

        motor::controls::midi_device_borrow_t::mtr_t _dev = nullptr ;

    public:

        akai_apc_key25( motor::controls::midi_device_borrow_t::mtr_t ptr ) noexcept : _dev( ptr ) 
        {
            assert( ptr != nullptr ) ;
        }

    public:
                    
        enum class button : size_t 
        {
            button_1 = 0, 
            button_2, 
            button_3, 
            button_4, 
            button_5, 
            button_6, 
            button_7, 
            button_8, 
            button_9, 
            button_10, 
            button_11, 
            button_12, 
            button_13, 
            button_14, 
            button_15, 
            button_16, 
            button_17, 
            button_18, 
            button_19, 
            button_20, 
            button_21, 
            button_22, 
            button_23, 
            button_24,
            button_25,
            button_26,
            button_27,
            button_28,
            button_29,
            button_30,
            button_31,
            button_32,
            button_33,
            button_34,
            button_35,
            button_36,
            button_37,
            button_38,
            button_39,
            button_40,
            button_41,
            button_42,
            button_43,
            button_44,
            button_45,
            button_46,
            button_47,
            button_48,
            button_49,
            button_50,
            button_51,
            button_52,
            button_53,
            button_54,
            button_55,
            num_buttons
        };

        enum class piano_key : size_t 
        {
            key_1 = static_cast<size_t>(this_t::button::num_buttons), 
            key_2, 
            key_3, 
            key_4, 
            key_5, 
            key_6, 
            key_7, 
            key_8, 
            key_9, 
            key_10, 
            key_11, 
            key_12, 
            key_13, 
            key_14, 
            key_15, 
            key_16, 
            key_17, 
            key_18, 
            key_19, 
            key_20, 
            key_21, 
            key_22, 
            key_23, 
            key_24,
            key_25,
            num_keys
        } ;

        enum class knob : size_t 
        {
            knob_1 = static_cast<size_t>(this_t::piano_key::num_keys),
            knob_2, 
            knob_3, 
            knob_4, 
            knob_5, 
            knob_6, 
            knob_7, 
            knob_8,
            num_knobs
        };

        enum class color_led : size_t 
        {
            led_1 = 0, 
            led_2, 
            led_3, 
            led_4, 
            led_5, 
            led_6, 
            led_7, 
            led_8, 
            led_9, 
            led_10, 
            led_11, 
            led_12, 
            led_13, 
            led_14, 
            led_15, 
            led_16, 
            led_17, 
            led_18, 
            led_19, 
            led_20, 
            led_21, 
            led_22, 
            led_23, 
            led_24,
            led_25,
            led_26,
            led_27,
            led_28,
            led_29,
            led_30,
            led_31,
            led_32,
            led_33,
            led_34,
            led_35,
            led_36,
            led_37,
            led_38,
            led_39,
            led_40,
            num_color_leds
        } ;

        enum class led_color : size_t
        {
            green = 1,
            red,
            yellow
        };

        enum class led_pattern : size_t
        {
            solid = 0,
            blink = 1
        };

        enum class led : size_t 
        {
            led_1 = static_cast<size_t>(this_t::color_led::num_color_leds), 
            led_2
        } ;

        //*************************************************************************************
        motor::controls::components::button_state get_state( this_t::button const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::button_t>( 
                static_cast< size_t >( e ) )->state() ;
        }

        //*************************************************************************************
        float_t get_value( this_t::knob const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::knob_t>( 
                static_cast< size_t >( e ) )->value() ;
        }

        //*************************************************************************************
        bool_t has_changed( this_t::knob const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::knob_t>( 
                static_cast< size_t >( e ) )->has_changed() ;
        }

        //*************************************************************************************
        std::tuple<bool_t, bool_t> get_state( this_t::knob const e ) const noexcept
        {
            return { this_t::has_changed(e), this_t::get_value( e ) } ;
        }

        //*************************************************************************************
        void_t set( this_t::color_led const e, this_t::led_color const c, this_t::led_pattern const p ) noexcept
        {
            _dev->get_out_component<motor::controls::components::multi_led_t>( 
                static_cast< size_t >( e ) )->operator= ( size_t(c) + size_t(p) ) ;
        }

        //*************************************************************************************
        void_t set( this_t::color_led const e, bool_t const onoff ) noexcept
        {
            _dev->get_out_component<motor::controls::components::multi_led_t>( 
                static_cast< size_t >( e ) )->operator= ( onoff ) ;
        }

        //*************************************************************************************
        void_t set( this_t::led const e, bool_t const onoff ) noexcept
        {
            _dev->get_out_component<motor::controls::components::binary_led_t>( 
                static_cast< size_t >( e ) )->operator= ( onoff ) ;
        }

        

        //*************************************************************************************
        // @return [ changed, plugged ]
        std::tuple<bool_t, bool_t> plug_state( void_t ) const noexcept
        {
            return _dev->plug_state() ;
        }

        //************************************************************************************************
        static bool_t create_layout( motor::string_cref_t name, motor::controls::midi_device_inout_t dev ) noexcept
        {
            if( name != "APC Key 25" )
            {
                return false ;
            }

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
    };
    motor_typedef( akai_apc_key25 ) ;
}

}}}