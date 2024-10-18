
#pragma once

#include "../types.h"
#include "../object.hpp"

#include "../shader/shader_bindings.hpp"
#include "../shader/shader_set.hpp"
#include "../buffer/vertex_attribute.h"

#include <motor/std/vector>

#include <algorithm>

namespace motor
{
    namespace graphics
    {
        enum class shader_api_type
        {
            unknown,
            glsl_1_4,       // gl 3.1
            glsl_4_0,       // gl 4.0
            glsles_3_0,     // gles 3.1
            hlsl_5_0       // dx 11
            
        };

        class MOTOR_GRAPHICS_API shader_object : public object
        {
            motor_this_typedefs( shader_object ) ;

        private: 
            
            motor::graphics::shader_bindings_t _shader_bindings ;

        private:

            typedef std::pair< motor::graphics::shader_api_type, motor::graphics::shader_set > ss_item_t ;
            typedef motor::vector< ss_item_t > shader_sets_t ;

            shader_sets_t _shader_sets ;

            motor::string_t _name ;
            motor::graphics::streamout_mode _sm = motor::graphics::streamout_mode::unknown ;

        public:
            
            shader_object( void_t ) noexcept {}

            shader_object( motor::string_cref_t name ) noexcept
                : _name( name ) {}

            shader_object( this_cref_t rhv ) noexcept : object( rhv ), _name( rhv._name ),
                _shader_bindings( rhv._shader_bindings ), _shader_sets( rhv._shader_sets ),
                _sm( rhv._sm ) {}

            shader_object( this_rref_t rhv ) noexcept : object( std::move(rhv) ),
                _name( std::move( rhv._name ) ), _shader_bindings( std::move( rhv._shader_bindings ) ),
                _shader_sets( std::move( rhv._shader_sets ) ), _sm( rhv._sm ) {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                object::operator=( rhv ) ;

                _name = rhv._name ;
                _shader_bindings = rhv._shader_bindings ;
                _shader_sets = rhv._shader_sets ;
                _sm = rhv._sm ;

                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                object::operator=( std::move( rhv ) ) ;

                _name = std::move( rhv._name ) ;
                _shader_bindings = std::move( rhv._shader_bindings ) ;
                _shader_sets = std::move( rhv._shader_sets ) ;
                _sm = rhv._sm ;

                return *this ;
            }

            this_ref_t set_name( motor::string_cref_t n ) noexcept 
            {
                _name = n ;
                return *this ;
            }

            //**************************************************************************
            this_ref_t set_streamout_mode( motor::graphics::streamout_mode const sm ) noexcept
            {
                _sm = sm ;
                return *this ;
            }

            //**************************************************************************
            motor::graphics::streamout_mode get_streamout_mode( void_t ) const noexcept
            {
                return _sm ;
            }

        public:
           
            this_ref_t insert( motor::graphics::shader_api_type const bt, motor::graphics::shader_set_in_t ss ) noexcept
            {
                auto iter = std::find_if( _shader_sets.begin(), _shader_sets.end(),
                    [&] ( this_t::ss_item_t const& item )
                {
                    return item.first == bt ;
                } ) ;

                if( iter == _shader_sets.end() )
                {
                    _shader_sets.emplace_back( std::make_pair( bt, ss ) ) ;
                }
                else
                {
                    iter->second = ss ;
                }

                return *this ;
            }

            bool_t shader_set( motor::graphics::shader_api_type const bt, motor::graphics::shader_set_out_t ss ) const noexcept
            {
                auto const iter = std::find_if( _shader_sets.begin(), _shader_sets.end(), 
                    [&] ( this_t::ss_item_t const & item ) 
                { 
                    return item.first == bt ;
                } ) ;

                if( iter == _shader_sets.end() ) return false ;

                ss = iter->second ;

                return true ;
            }

        public:
            
            this_ref_t set( shader_bindings_cref_t sb ) noexcept { _shader_bindings = sb ; return *this ; }
            this_ref_t set( shader_bindings_rref_t sb ) noexcept { _shader_bindings = std::move( sb ) ; return *this ; }
            shader_bindings_cref_t shader_bindings( void_t ) const noexcept{ return _shader_bindings ; }
            shader_bindings_ref_t shader_bindings( void_t ) noexcept{ return _shader_bindings ; }

            motor::string_cref_t name( void_t ) const noexcept
            {
                return _name ;
            }
        };
        motor_typedef( shader_object ) ;
    }
}
