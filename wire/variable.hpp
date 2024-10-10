#pragma once

#include "typedefs.h"
#include "slot/input_slot.h"
#include "slot/output_slot.h"
#include "slot/sheet.hpp"

#include <motor/std/string>

namespace motor
{
    namespace wire
    {
        class any
        {
            motor_this_typedefs( any ) ;

        private:

            char const * _name ;

        public:

            struct member_info
            {
                size_t level ;
                motor::string_t full_name ;
            };
            motor_typedef( member_info ) ;
            using for_each_funk_t = std::function< void_t ( this_ref_t, member_info_in_t ) > ;

        public:

            any( char const * name ) noexcept : _name( name ) {}
            any( this_cref_t ) = delete ;
            any( this_rref_t rhv ) noexcept : _name( motor::move( rhv._name ) ){ }
            virtual ~any( void_t ) noexcept {}

            char const * name( void_t ) const noexcept 
            {
                return _name ;
            }


            inputs_t inputs( void_t ) noexcept 
            {
                inputs_t ret ;
                this_t::inspect( [&]( this_ref_t any_in, member_info_in_t info )
                {
                    ret.add( info.full_name, any_in.get_input() ) ;
                } ) ;
                return ret ;
            }

        public: 

            virtual void_t update( void_t ) noexcept = 0 ;
            virtual motor::wire::iinput_slot_mtr_safe_t get_input( void_t ) noexcept = 0 ;
            
            
        public:

            void_t inspect( for_each_funk_t f ) noexcept
            {
                member_info_t info{ 0, motor::string_t( this_t::name() ) } ;
                f( *this, info ) ;
                ++info.level ;

                this->for_each_member( f, info ) ;
            }

            class derived_accessor
            {
                this_ref_t _owner ;
            public:
                derived_accessor( this_ref_t r ) noexcept : _owner( r ) {}
                void_t for_each_member( for_each_funk_t f, member_info_in_t ifo ) noexcept 
                {
                    _owner.for_each_member( f, ifo ) ;
                }
            };
            friend class derived_accessor ;

        private:

            // go through every member motor::wire::variable< some_type >
            // only required for complex types with multiple sub-variables
            virtual void_t for_each_member( for_each_funk_t f, member_info_in_t ) noexcept 
            {
                // do not implement if no sub-variables are used!
            }
        };
        motor_typedef( any ) ;


        template< typename T >
        class variable : public any
        {
            using base_t = any ;
            motor_this_typedefs( variable< T > ) ;

            using in_t = motor::wire::input_slot< T > ;
            using out_t = motor::wire::output_slot< T > ;

        private:

            in_t * _in = motor::shared( in_t() ) ;
            out_t * _out = motor::shared( out_t() ) ;
            

        public:

            variable( char const * const name ) noexcept : base_t( name ) {}
            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept : 
                _out( motor::move( rhv._out ) ), _in( motor::move( rhv._in ) )
            {
            }

            virtual ~variable( void_t ) noexcept 
            { 
                motor::release( motor::move( _in ) ) ;
                motor::release( motor::move( _out ) ) ;
            }


        public: 

            virtual void_t update( void_t ) noexcept 
            {
                _in->exchange() ;
                *_out = _in->get_value() ;
            }

            T const & get_value( void_t ) const noexcept
            {
                return _in->get_value() ;
            }

            virtual motor::wire::iinput_slot_mtr_safe_t get_input( void_t ) noexcept
            {
                return motor::share( _in ) ;
            }
        };

        motor_typedefs( variable< bool_t >, boolv ) ;
        motor_typedefs( variable< int_t >, intv ) ;
        motor_typedefs( variable< uint_t >, uintv ) ;
        motor_typedefs( variable< float_t >, floatv ) ;
        motor_typedefs( variable< double_t >, doublev ) ;
    }
}