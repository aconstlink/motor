
#pragma once

#include "../object.hpp"

#include "../object/shader_object.h"
#include "../buffer/vertex_attribute.h"
#include "../variable/variable_set.hpp"
#include "../state/state_set.h"

#include <motor/std/vector>

#include <algorithm>

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API render_object : public object
        {
            motor_this_typedefs( render_object ) ;

        private:

            motor::string_t _name ;
            motor::vector< motor::string_t > _geo ;
            motor::string_t _shader ;
            motor::vector< motor::string_t > _soo ;

            motor::vector< motor::graphics::variable_set_mtr_t > _vars ;
            motor::vector< motor::graphics::render_state_sets_t > _states ;

        public:
            
            render_object( void_t ) noexcept {}
            render_object( motor::string_cref_t name ) noexcept
                : _name( name ) {}

            ~render_object( void_t ) noexcept 
            {
                for( auto * v : _vars ) motor::memory::release_ptr( v ) ;
            }

            render_object( this_cref_t rhv ) noexcept : object( rhv )
            {
                _name = rhv._name ;
                _geo = rhv._geo;
                _shader = rhv._shader ;
                _vars = rhv._vars ;
                _states = rhv._states ;
                _soo = rhv._soo ;

                for( auto * v : _vars ) motor::memory::copy_ptr( v ) ;
            }

            render_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
            {
                _name = std::move( rhv._name ) ;
                _geo = std::move( rhv._geo ) ;
                _shader = std::move( rhv._shader ) ;
                _vars = std::move( rhv._vars ) ;
                _states = std::move( rhv._states ) ;
                _soo = std::move( rhv._soo ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                object::operator=( rhv ) ;

                _name = rhv._name ;
                _geo = rhv._geo;
                _shader = rhv._shader ;
                _vars = rhv._vars ;
                _states = rhv._states ;
                _soo = rhv._soo ;

                for( auto * v : _vars ) motor::memory::copy_ptr( v ) ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                object::operator=( std::move( rhv ) ) ;

                _name = std::move( rhv._name ) ;
                _geo = std::move( rhv._geo ) ;
                _shader = std::move( rhv._shader ) ;
                _vars = std::move( rhv._vars ) ;
                _states = std::move( rhv._states ) ;
                _soo = std::move( rhv._soo ) ;
                
                return *this ;
            }

        public:
            
            this_ref_t link_geometry( motor::string_cref_t name ) noexcept 
            {
                _geo.emplace_back( name ) ;
                return *this ;
            }

            this_ref_t link_geometry( std::initializer_list< motor::string_t > const & names ) noexcept 
            {
                for( auto const & name : names ) _geo.emplace_back( name ) ;
                return *this ;
            }

            // link to stream out object so geometry can be fed from there.
            // the geometry is then mainly used for geometry layout.
            this_ref_t link_geometry( motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept 
            {
                _geo.emplace_back( name ) ;
                _soo.emplace_back( soo_name ) ;
                return *this ;
            }

            size_t get_num_geometry( void_t ) const noexcept{ return _geo.size() ; }
            size_t get_num_streamout( void_t ) const noexcept{ return _soo.size() ; }

            motor::string_cref_t get_geometry( size_t const i = 0 ) const noexcept
            {
                return _geo[i] ;
            }

            motor::string_cref_t get_streamout( size_t const i = 0 ) const noexcept
            {
                return _soo[i] ;
            }

            bool_t has_streamout_link( void_t ) const noexcept
            {
                return !_soo.empty() ;
            }

            this_ref_t link_shader( motor::string_cref_t name ) noexcept
            {
                _shader = name ;
                return *this ;
            }

            motor::string_cref_t get_shader( void_t ) const noexcept
            {
                return _shader ;
            }

        public: // variable sets

            this_ref_t add_variable_set( motor::graphics::variable_set_mtr_unique_t vs ) noexcept
            {
                _vars.emplace_back( vs ) ;
                return *this ;
            }

            this_ref_t add_variable_set( motor::graphics::variable_set_mtr_shared_t vs ) noexcept
            {
                _vars.emplace_back( motor::memory::copy_ptr( vs ) ) ;
                return *this ;
            }

            this_ref_t remove_variable_sets( void_t ) noexcept 
            {
                for( auto * v : _vars ) motor::memory::release_ptr( v ) ;
                _vars.clear() ;
                return *this ;
            }

            typedef std::function< void_t ( size_t const i, motor::graphics::variable_set_mtr_t ) > for_each_var_funk_t ;

            void_t for_each( for_each_var_funk_t funk ) noexcept
            {
                size_t i = 0 ;
                for( auto const & v : _vars )
                {
                    funk( i++, v ) ;
                }
            }

            size_t get_num_variable_sets( void_t ) const noexcept { return _vars.size() ; }
            motor::graphics::variable_set_mtr_shared_t get_variable_set( size_t const i ) noexcept 
            {
                if( _vars.size() <= i ) return motor::graphics::variable_set_mtr_shared_t::make() ;
                return motor::graphics::variable_set_mtr_shared_t( _vars[i] ) ;
            }

        public: // render state sets

            this_ref_t add_render_state_set( motor::graphics::render_state_sets_cref_t rs ) noexcept
            {
                _states.emplace_back( rs ) ;
                return *this ;
            }

            typedef std::function< void_t ( size_t const i, motor::graphics::render_state_sets_cref_t ) > for_each_render_state_funk_t ;

            void_t for_each( for_each_render_state_funk_t funk ) noexcept
            {
                size_t i = 0 ;
                for( auto const& v : _states )
                {
                    funk( i++, v ) ;
                }
            }

        public:

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }
        };
        motor_typedef( render_object ) ;
    }
}
