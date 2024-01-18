
#pragma once

#include "../object.hpp"

#include "render_object.h"

namespace motor
{
    namespace graphics
    {
        enum class msl_api_type
        {
            none,
            msl_4_0         // gen 4 shader and library
        };

        class MOTOR_GRAPHICS_API msl_object : public object
        {
            motor_this_typedefs( msl_object ) ;

        private:

            // msl shader itself
            // render object (variables, render states)
            // shader object (generated shaders and i/o)
            // geometry object maybe

            motor::string_t _name ;

            motor::vector< motor::string_t > _geo ;

            struct data
            {
                msl_api_type t ;
                motor::string_t code ;
            };

            motor::vector< data > _datas ;
            motor::vector< motor::graphics::variable_set_mtr_t > _vars ;

            motor::vector< motor::graphics::render_object_mtr_t > _ros ; 
            motor::vector< motor::graphics::shader_object_mtr_t > _sos ; 

        public:

            msl_object( void_t ) noexcept {}
            msl_object( motor::string_in_t name ) noexcept : _name( name ) {}
            msl_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
            {
                _name = std::move( rhv._name ) ;
                _datas = std::move( rhv._datas ) ;
                _geo = std::move( rhv._geo ) ;
                _sos = std::move( rhv._sos ) ;
                _ros = std::move( rhv._ros ) ;
            }
            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _name = std::move( rhv._name ) ;
                _datas = std::move( rhv._datas ) ;
                _geo = std::move( rhv._geo ) ;
                _sos = std::move( rhv._sos ) ;
                _ros = std::move( rhv._ros ) ;
                return *this ;
            }

            msl_object( this_cref_t ) = delete ;

            virtual ~msl_object( void_t ) noexcept 
            {
                for( auto * ro : _ros )
                    motor::memory::release_ptr( ro ) ;
                
                for( auto * so : _sos )
                    motor::memory::release_ptr( so ) ;

                for( auto * vs : _vars )
                    motor::memory::release_ptr( vs ) ;
            }

            motor::string_cref_t name( void_t ) const noexcept
            { 
                return _name ;
            }

            this_ref_t add( motor::graphics::msl_api_type const t, motor::string_in_t msl_code ) noexcept 
            {
                _datas.emplace_back( this_t::data{ t, msl_code } ) ;
                return *this ;
            }

            typedef std::function< void_t ( motor::string_in_t ) > foreach_funk_t ;
            void_t for_each_msl( motor::graphics::msl_api_type const t, foreach_funk_t funk ) const noexcept 
            {
                for( auto & d : _datas )
                {
                    if( d.t == t ) funk( d.code ) ;
                }
            }

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

            motor::vector< motor::string_t > const & get_geometry( void_t ) const noexcept{ return _geo ; }

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

            motor::vector< motor::graphics::variable_set_mtr_unique_t > get_varibale_sets( void_t ) const noexcept
            {
                motor::vector< motor::graphics::variable_set_mtr_unique_t > ret ;

                for( auto * mtr : _vars ) ret.emplace_back( motor::unique( motor::memory::copy_ptr( mtr ) ) ) ;

                return ret ;
            }

            motor::graphics::render_object_mtr_t get_render_object( size_t const i ) noexcept 
            {
                if( i >= _ros.size() ) return nullptr ;
                return _ros[i] ;
            }

        public:

            class private_accessor
            {
            private: 

                msl_object * _ptr ;

            public:

                private_accessor( msl_object * ptr ) noexcept : _ptr( ptr ) {}

                void_t add_objects( motor::graphics::render_object_mtr_unique_t ro, motor::graphics::shader_object_mtr_unique_t so ) noexcept
                {
                    _ptr->add_objects( std::move( ro ), std::move( so ) ) ;
                }
            };
            friend class private_accessor ;
            
        private:

            void_t add_objects( motor::graphics::render_object_mtr_unique_t ro, motor::graphics::shader_object_mtr_unique_t so ) noexcept
            {
                _ros.emplace_back( ro ) ;
                _sos.emplace_back( so ) ;
            }

        };
        motor_typedef( msl_object ) ;
    }
}
