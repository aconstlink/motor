
#pragma once

#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/scroll.hpp"
#include "../components/point.hpp"
#include "../device.hpp"

namespace motor
{
    namespace device
    {
        namespace layouts
        {
            class three_mouse
            {
                motor_this_typedefs( three_mouse ) ;
                motor_typedefs( motor::device::device_tpl< this_t >, this_device ) ;

                friend this_device_t ;

            public:

                enum class input_component
                {
                    left_button,
                    right_button,
                    middle_button,
                    scroll_wheel,
                    local_coords,
                    global_coords,
                    num_components
                };

                enum class output_component
                {
                    num_components
                };

            private:

                static void_t init_components( this_t::this_device_ref_t dev ) 
                {
                    // inputs
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::button_t>() ;
                    dev.add_input_component<motor::device::components::scroll_t>() ;
                    dev.add_input_component<motor::device::components::point_t>() ;
                    dev.add_input_component<motor::device::components::point_t>() ;
                  
                    // outputs
                }

            private:

                // non-managed!
                this_t::this_device_ptr_t _dev ;

            public:

                three_mouse( this_t::this_device_ptr_t dev ) noexcept : _dev( dev )
                {
                    assert( dev != nullptr ) ;
                }
                three_mouse( this_cref_t rhv ) noexcept : _dev( rhv._dev ) {}
                three_mouse( this_rref_t rhv ) noexcept : _dev( rhv._dev ) {}
                ~three_mouse( void_t ) noexcept {}

            public:

                enum class button
                {
                    none, left, right, middle, num_buttons
                };

                static motor::string_t to_string( button const b ) noexcept
                {
                    static char_t const * const __names[] = { "none", "left", "right", "middle", "invalid" } ;
                    return __names[ size_t( b ) >= size_t( button::num_buttons ) ? size_t( button::num_buttons ) : size_t( b ) ] ;
                }

                bool_t get_scroll( float_t & v ) noexcept
                {
                    auto* comp = _dev->get_component<motor::device::components::scroll_t>(
                        this_t::input_component::scroll_wheel ) ;

                    v = comp->value() ;

                    return comp->has_changed() ;
                }

                motor::math::vec2f_t get_local( void_t ) noexcept
                {
                    auto* comp = _dev->get_component<motor::device::components::point_t>(
                        this_t::input_component::local_coords ) ;

                    return motor::math::vec2f_t( comp->value() ) ;
                }

                motor::math::vec2f_t get_global( void_t ) noexcept
                {
                    auto* comp = _dev->get_component<motor::device::components::point_t>(
                        this_t::input_component::global_coords ) ;

                    return motor::math::vec2f_t( comp->value() ) ;
                }

            public:

                 motor::device::components::button_ptr_t get_component( this_t::button const b ) noexcept
                {
                    motor::device::components::button_ptr_t ret = nullptr ;

                    if( b == this_t::button::left )
                    {
                        ret = _dev->get_component<motor::device::components::button_t>(
                            this_t::input_component::left_button ) ;
                    }
                    else if( b == this_t::button::middle )
                    {
                        ret = _dev->get_component<motor::device::components::button_t>(
                            this_t::input_component::middle_button ) ;
                    }
                    else if( b == this_t::button::right )
                    {
                        ret = _dev->get_component<motor::device::components::button_t>(
                            this_t::input_component::right_button ) ;
                    }

                    return ret ;
                }

                motor::device::components::scroll_ptr_t get_scroll_component( void_t ) noexcept
                {
                    return _dev->get_component<motor::device::components::scroll_t>(
                        this_t::input_component::scroll_wheel ) ;
                }

                motor::device::components::point_ptr_t get_local_component( void_t ) noexcept
                {
                    return _dev->get_component<motor::device::components::point_t>(
                        this_t::input_component::local_coords ) ;
                }

                motor::device::components::point_ptr_t get_global_component( void_t ) noexcept
                {
                    return _dev->get_component<motor::device::components::point_t>(
                        this_t::input_component::global_coords ) ;
                }

            public:

                bool_t is_pressed( this_t::button const b ) noexcept
                {
                    bool_t res = false ;

                    switch( b )
                    {
                    case this_t::button::left:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::left_button ) ;

                        res = comp->state() == motor::device::components::button_state::pressed ;
                    }

                    break ;
                    case this_t::button::right:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::right_button ) ;

                        res = comp->state() == motor::device::components::button_state::pressed ;
                        break ;
                    }
                    case this_t::button::middle:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::middle_button ) ;

                        res = comp->state() == motor::device::components::button_state::pressed ;
                        break ;
                    }
                    default: break ;
                    }

                    return res ;
                }

                bool_t is_pressing( this_t::button const b ) noexcept
                {
                    bool_t res = false ;

                    switch( b )
                    {
                    case this_t::button::left:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::left_button ) ;

                        res = comp->state() == motor::device::components::button_state::pressing ;

                        break ;
                    }
                    case this_t::button::right:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::right_button ) ;

                        res = comp->state() == motor::device::components::button_state::pressing ;
                        break ;
                    }
                    case this_t::button::middle:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::middle_button ) ;

                        res = comp->state() == motor::device::components::button_state::pressing ;
                        break ;
                    }
                    default:break ;
                    }

                    return res ;
                }

                bool_t is_released( this_t::button const b ) noexcept
                {
                    bool_t res = false ;

                    switch( b )
                    {
                    case this_t::button::left:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::left_button ) ;

                        res = comp->state() == motor::device::components::button_state::released ;

                        break ;
                    }
                    case this_t::button::right:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::right_button ) ;

                        res = comp->state() == motor::device::components::button_state::released ;
                        break ;
                    }
                    case this_t::button::middle:
                    {
                        auto* comp = _dev->get_component<motor::device::components::button_t>( this_t::input_component::middle_button ) ;

                        res = comp->state() == motor::device::components::button_state::released ;
                        break ;
                    }
                    default: break ;
                    }

                    return res ;
                }
            };
            motor_typedef( three_mouse ) ;
        }
        motor_typedefs( motor::device::device_tpl< motor::device::layouts::three_mouse_t>, three_device ) ;
    }
}
