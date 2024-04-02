#pragma once

#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/scroll.hpp"
#include "../components/point.hpp"
#include "../device.hpp"

#include "../midi/midi_message.hpp"

namespace motor
{
    namespace controls
    {
        namespace types
        {
            class midi_controller
            {
                motor_this_typedefs( midi_controller ) ;

            private:

                motor::controls::device_borrow_t::mtr_t _dev = nullptr ;

            public:

                enum class input_component
                {};

                enum class output_component 
                {};

                static void_t init_components( motor::controls::device_inout_t dev ) 
                {
                }

            public:

                midi_controller( motor::controls::device_borrow_t::mtr_t dev ) noexcept : _dev( dev ) {}

            public:

                motor::string_cref_t tag( void_t ) const noexcept { _dev->name() ; }
            };
            motor_typedef( midi_controller ) ;
        }

        template<>
        class device_with<motor::controls::types::midi_controller> : public device
        {
            motor_this_typedefs( device_with< motor::controls::types::midi_controller > ) ;
            using base_t = device ;

        public:

            using layout_t = motor::controls::types::midi_controller ;

            using midi_in_update_funk_t = std::function< bool_t ( 
                motor::controls::input_component_ptr_t, motor::controls::midi_message_cref_t ) > ;

            using midi_out_update_funk_t = std::function< bool_t ( 
                motor::controls::output_component_ptr_t, motor::controls::midi_message_inout_t ) > ;

        private:

            struct update_in_data
            {
                size_t id ;
                midi_in_update_funk_t funk ;
            };
            motor::vector< update_in_data > _update_ins ;

            struct update_out_data
            {
                size_t id ;
                midi_out_update_funk_t funk ;
            };
            motor::vector< update_out_data > _update_outs ;

        public:

            device_with( void_t ) noexcept
            {
                layout_t::init_components( *this ) ;
            }

            device_with( motor::string_cref_t name ) noexcept : device( name ) 
            {
                layout_t::init_components( *this ) ;
            }

            device_with( this_rref_t rhv ) noexcept : base_t ( std::move( rhv ) )
            {
                _update_ins = std::move( rhv._update_ins ) ;
                _update_outs = std::move( rhv._update_outs ) ;
            }

            template< typename comp_t >
            this_ref_t add_input_component( comp_t && comp, midi_in_update_funk_t funk ) noexcept
            {
                _update_ins.emplace_back( update_in_data{ _update_ins.size(), funk } ) ;
                device::add_input_component<comp_t>( comp ) ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_input_component( midi_in_update_funk_t funk ) noexcept
            {
                _update_ins.emplace_back( update_in_data{ _update_ins.size(), funk } ) ;
                device::add_input_component<comp_t>() ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_out_component( midi_out_update_funk_t funk ) noexcept
            {
                _update_outs.emplace_back( update_out_data{ _update_outs.size(), funk } ) ;
                device::add_output_component<comp_t>() ;
                return *this ;
            }

            void_t handle_in_message( motor::controls::midi_message_cref_t msg ) noexcept 
            {
                for( auto ud : _update_ins )
                {
                    if( ud.funk( base_t::get_in_component(ud.id), msg ) ) break ;
                }
            }

            void_t handle_out_messages( motor::controls::midi_messages_inout_t msgs ) noexcept
            {
                motor::controls::midi_message_t msg ;

                for( auto ud : _update_outs )
                {
                    if( ud.funk( base_t::get_out_component( ud.id ), msg ) )
                        msgs.emplace_back( std::move( msg ) ) ;
                }
            }
        };
        motor_typedefs( motor::controls::device_with< motor::controls::types::midi_controller>, midi_device ) ;
    }
}