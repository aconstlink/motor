#pragma once

#include "../typedefs.h"
#include "../components/button.hpp"
#include "../components/scroll.hpp"
#include "../components/point.hpp"
#include "../device.hpp"

#include "../midi_message.hpp"

namespace motor
{
    namespace controls
    {
        namespace layouts
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
        class device_with<motor::controls::layouts::midi_controller> : public device
        {
            motor_this_typedefs( device_with< motor::controls::layouts::midi_controller > ) ;
            using base_t = device ;

        public:

            using layout_t = motor::controls::layouts::midi_controller ;

            using midi_update_funk_t = std::function< bool_t ( 
                motor::controls::input_component_ptr_t, motor::controls::midi_message_cref_t ) > ;

        private:

            struct update_data
            {
                size_t id ;
                midi_update_funk_t funk ;
            };
            motor::vector< update_data > _update_ins ;

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
            }

            template< typename comp_t >
            this_ref_t add_input_component( comp_t && comp, midi_update_funk_t funk ) noexcept
            {
                _update_ins.emplace_back( update_data{ _update_ins.size(), funk } ) ;
                device::add_input_component<comp_t>( comp ) ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_input_component( midi_update_funk_t funk ) noexcept
            {
                _update_ins.emplace_back( update_data{ _update_ins.size(), funk } ) ;
                device::add_input_component<comp_t>() ;
                return *this ;
            }

            void_t handle_in_message( motor::controls::midi_message_cref_t msg ) noexcept 
            {
                for( auto ud : _update_ins )
                {
                    if( ud.funk( base_t::get_in_component(ud.id), msg ) ) break ;
                }
            }
        };
        motor_typedefs( motor::controls::device_with< motor::controls::layouts::midi_controller>, midi_device ) ;
    }
}