#pragma once

#include "../../typedefs.h"
#include "../../types/midi_controller.hpp"
#include "../../components/button.hpp"
#include "../../components/button.hpp"
#include "../../components/slider.hpp"
#include "../../components/knob.hpp"
#include "../../components/led.hpp"
#include "../../components/touch.hpp"

namespace motor { namespace controls { namespace midi { namespace layouts
{
    //***************************************************************************************************
    class korg_nanokontrol2
    {
        motor_this_typedefs( korg_nanokontrol2 ) ;

    private:

        motor::controls::midi_device_borrow_t::mtr_t _dev = nullptr ;

    public:

        korg_nanokontrol2( motor::controls::midi_device_borrow_t::mtr_t ptr ) noexcept : _dev( ptr ) 
        {
            assert( ptr != nullptr ) ;
        }

    public: // components by enum

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
            num_leds
        } ;

    public:

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

        //************************************************************************************************
        static bool_t create_layout( motor::string_cref_t name, motor::controls::midi_device_inout_t dev ) noexcept
        {
            if( name != "nanoKONTROL2" ) return false ;

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
    } ;
    motor_typedef( korg_nanokontrol2 ) ;


    //***************************************************************************************************
    class korg_nanopad2
    {
        motor_this_typedefs( korg_nanopad2 ) ;

    private:

        motor::controls::midi_device_borrow_t::mtr_t _dev = nullptr ;

    public:

        korg_nanopad2( motor::controls::midi_device_borrow_t::mtr_t ptr ) noexcept : _dev( ptr ) 
        {
            assert( ptr != nullptr ) ;
        }

    public: // components by enum

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
            num_buttons
        };

        enum class pad : size_t 
        {
            pad_1
        };

    public:

        //*************************************************************************************
        motor::controls::components::button_state get_state( this_t::button const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::button_t>( 
                static_cast< size_t >( e ) )->state() ;
        }

        //*************************************************************************************
        motor::controls::components::touch_state get_state( this_t::pad const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::touch_t>( 
                static_cast< size_t >( e ) )->state() ;
        }

        //*************************************************************************************
        motor::math::vec2f_t get_value( this_t::pad const e ) const noexcept
        {
            return _dev->get_in_component<motor::controls::components::touch_t>( 
                static_cast< size_t >( e ) )->value() ;
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
            if( name != "nanoPAD2" ) return false ;

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

    }  ;
    motor_typedef( korg_nanopad2 ) ;
} 

}}}