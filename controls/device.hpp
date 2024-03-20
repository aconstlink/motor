
#pragma once

#include "api.h"
#include "typedefs.h"

#include "component.hpp"

#include "imapping.hpp"

#include <motor/std/vector>

namespace motor
{
    namespace controls
    {
        class device
        {
            motor_this_typedefs( device ) ;

        private:

            motor::string_t _name ;

            motor::vector< motor::controls::input_component_ptr_t > _inputs ;
            motor::vector< motor::controls::output_component_ptr_t > _outputs ;
            motor::vector< motor::controls::imapping_mtr_t  > _mappings ;

        public:

            device( void_t ) noexcept {}

            device( motor::string_cref_t name ) noexcept : _name( name ) {}

            device( this_cref_t rhv ) = delete ;

            device( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;

                _inputs = std::move( rhv._inputs ) ;
                _outputs = std::move( rhv._outputs ) ;
                _mappings = std::move( rhv._mappings ) ;
            }

            virtual ~device( void_t ) noexcept
            {
                for( auto* ptr : _inputs )
                {
                    motor::memory::global_t::dealloc( ptr ) ;
                }

                for( auto* ptr : _outputs )
                {
                    motor::memory::global_t::dealloc( ptr ) ;
                }
                for( auto * mtr : _mappings )
                {
                    motor::memory::release_ptr( mtr ) ;
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

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
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

            motor::controls::input_component_ptr_t get_in_component( size_t const c ) noexcept
            {
                return c < _inputs.size() ? _inputs[ c ] : nullptr ;
            }

            motor::controls::output_component_ptr_t get_out_component( size_t const c ) noexcept
            {
                return c < _outputs.size() ? _outputs[ c ] : nullptr ;
            }

            template< typename T >
            T* get_in_component( size_t const c ) noexcept
            {
                return c < _inputs.size() ? dynamic_cast< T* >( _inputs[ c ] ) : nullptr ;
            }

            template< typename T >
            T* get_out_component( size_t const c ) noexcept
            {
                return c < _outputs.size() ? dynamic_cast< T* >( _outputs[ c ] ) : nullptr ;
            }

            typedef std::function< void_t ( size_t const, motor::controls::input_component_ptr_t ) > input_funk_t ;

            bool_t foreach_input( input_funk_t funk ) noexcept
            {
                for( size_t i = 0; i < _inputs.size(); ++i )
                {
                    funk( i, _inputs[ i ] ) ;
                }

                return true ;
            }

            typedef std::function< void_t ( size_t const, motor::controls::output_component_ptr_t ) > output_funk_t ;

            bool_t foreach_output( output_funk_t funk ) noexcept
            {
                for( size_t i = 0; i < _outputs.size(); ++i )
                {
                    funk( i, _outputs[ i ] ) ;
                }

                return true ;
            }

        public:

            void_t add_mapping( motor::controls::imapping_mtr_safe_t m ) noexcept
            {
                _mappings.emplace_back( m ) ;
            }

            typedef std::function< void_t ( motor::controls::imapping_mtr_t ) > mapping_searach_ft ;
            void_t search( this_t::mapping_searach_ft funk ) 
            {
                for( auto & m : _mappings ) funk( m ) ;
            }

        public:

            void_t update( void_t ) noexcept
            {
                this_t::update_inputs() ;
                this_t::update_outputs() ;
                this_t::update_mappings() ;
            }

            // meant for a module to reset the components
            void_t update_inputs( void_t ) noexcept
            {
                for( auto* comp : _inputs ) comp->update() ;
            }

            void_t update_outputs( void_t ) noexcept
            {
                for( auto* comp : _outputs ) comp->update() ;
            }

            void_t update_mappings( void_t ) noexcept
            {
                for( auto& m : _mappings ) m->update() ;
            }
        };
        motor_typedef( device ) ;


        template< typename T >
        class device_with : public device
        {
            motor_this_typedefs( device_with< T > ) ;

        public:

            using device_type_t = T ;

        public:

            device_with( void_t ) noexcept
            {
                device_type_t::init_components( *this ) ;
            }

            device_with( motor::string_cref_t name ) noexcept : device( name ) 
            {
                device_type_t::init_components( *this ) ;
            }
        };
    }
}
