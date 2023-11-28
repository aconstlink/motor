
#pragma once

#include "api.h"
#include "typedefs.h"

#include "component.hpp"

#include "imapping.hpp"

#include <motor/std/vector>

namespace motor
{
    namespace device
    {
        class MOTOR_DEVICE_API idevice
        {
        public:

            virtual ~idevice( void_t ) {}
        };
        motor_typedef( idevice ) ;

        template< typename L >
        class device_tpl : public idevice
        {
            motor_this_typedefs( device_tpl<L> ) ;

        public:

            motor_typedefs( L, layout ) ;

        private:

            motor::string_t _name ;

            motor::vector< motor::device::input_component_ptr_t > _inputs ;
            motor::vector< motor::device::output_component_ptr_t > _outputs ;
            motor::vector< motor::device::imapping_mtr_t  > _mappings ;

        public:

            device_tpl( void_t ) noexcept 
            {
                layout_t::init_components(*this) ;
            }

            device_tpl( motor::string_cref_t name ) noexcept : _name( name ) 
            {
                layout_t::init_components(*this) ;
            }

            device_tpl( this_cref_t rhv ) = delete ;

            device_tpl( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;

                _inputs = std::move( rhv._inputs ) ;
                _outputs = std::move( rhv._outputs ) ;
                _mappings = std::move( rhv._mappings ) ;
            }

            virtual ~device_tpl( void_t ) noexcept
            {
                for( auto* ptr : _inputs )
                {
                    motor::memory::global_t::dealloc( ptr ) ;
                }

                for( auto* ptr : _outputs )
                {
                    motor::memory::global_t::dealloc( ptr ) ;
                }
            }

            this_ref_t operator = ( this_cref_t rhv ) = delete ;

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;

                _inputs = std::move( rhv._inputs ) ;
                _outputs = std::move( rhv._outputs ) ;
                _mappings = std::move( rhv._mappings ) ;
                return *this ;
            }

        public: // add input

            template< typename comp_t >
            this_ref_t add_input_component( void_t ) noexcept
            {
                _inputs.emplace_back( motor::memory::global_t::alloc<comp_t>(
                        "[add_input_component] : input component" ) ) ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_input_component( comp_t const & comp ) noexcept
            {
                _inputs.emplace_back( 
                    motor::memory::global_t::alloc( comp, 
                        "[add_input_component] : input component" ) ) ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_input_component( comp_t && comp ) noexcept
            {
                _inputs.emplace_back( 
                    motor::memory::global_t::alloc( std::move(comp), 
                        "[add_input_component] : input component" ) ) ;
                return *this ;
            }

        public: // add output

            template< typename comp_t >
            this_ref_t add_output_component( void_t ) noexcept
            {
                _outputs.emplace_back(
                    motor::memory::global_t::alloc( comp_t(),
                        "[add_output_component] : output component" ) ) ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_output_component( comp_t const& comp )
            {
                _outputs.emplace_back( 
                    motor::memory::global_t::alloc( comp, 
                        "[add_output_component] : output component" ) ) ;
                return *this ;
            }

            template< typename comp_t >
            this_ref_t add_output_component( comp_t && comp )
            {
                _outputs.emplace_back( 
                    motor::memory::global_t::alloc( std::move( comp ), 
                        "[add_output_component] : output component" ) ) ;
                return *this ;
            }

        public:

            motor::device::input_component_ptr_t get_component( typename layout_t::input_component const c ) noexcept
            {
                if( size_t( c ) >= size_t( this_t::layout_t::input_component::num_components ) ) return nullptr ;
                return _inputs[ size_t( c ) ] ;
            }

            motor::device::output_component_ptr_t get_component( typename layout_t::output_component const c ) noexcept
            {
                if( size_t( c ) >= size_t( this_t::layout_t::output_component::num_components ) ) return nullptr ;
                return _outputs[ size_t( c ) ] ;
            }

            template< typename T >
            T* get_component( typename layout_t::input_component const c ) noexcept
            {
                if( size_t( c ) >= size_t(layout_t::input_component::num_components) ) 
                    return nullptr ;
                return dynamic_cast< T* >( _inputs[ size_t( c ) ] ) ;
            }

            template< typename T >
            T* get_component( typename layout_t::output_component const c ) noexcept
            {
                if( size_t( c ) >= size_t(layout_t::output_component::num_components) ) return nullptr ;
                return dynamic_cast< T* >( _outputs[ size_t( c ) ] ) ;
            }

            typedef std::function< void_t ( typename layout_t::input_component const, motor::device::input_component_ptr_t ) > input_funk_t ;

            bool_t foreach_input( input_funk_t funk ) noexcept
            {
                for( size_t i = 0; i < size_t( layout_t::input_component::num_components ); ++i )
                {
                    funk( typename layout_t::input_component( i ), _inputs[ i ] ) ;
                }

                return true ;
            }

            typedef std::function< void_t ( typename layout_t::output_component const, motor::device::output_component_ptr_t ) > output_funk_t ;

            bool_t foreach_output( output_funk_t funk ) noexcept
            {
                for( size_t i = 0; i < size_t( layout_t::output_component::num_components ); ++i )
                {
                    funk( typename layout_t::output_component( i ), _outputs[ i ] ) ;
                }

                return true ;
            }

        public:

            void_t add_mapping( motor::device::imapping_mtr_shared_t m ) noexcept
            {
                _mappings.emplace_back( motor::memory::copy_ptr( m ) ) ;
            }

            void_t add_mapping( motor::device::imapping_mtr_unique_t m ) noexcept
            {
                _mappings.emplace_back( m ) ;
            }

            typedef std::function< void_t ( motor::device::imapping_mtr_t ) > mapping_searach_ft ;
            void_t search( this_t::mapping_searach_ft funk ) 
            {
                for( auto & m : _mappings ) funk( m ) ;
            }

        public:

            // meant for a module to reset the components
            void_t update( void_t ) noexcept
            {
                for( auto* comp : _inputs ) comp->update() ;
                for( auto* comp : _outputs ) comp->update() ;
                for( auto& m : _mappings ) m->update() ;
            }
        };
    }
}
