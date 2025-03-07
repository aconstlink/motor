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
            
            motor::wire::iinput_slot_ptr_t _in = nullptr ;
            motor::wire::ioutput_slot_ptr_t _out = nullptr ;

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
            any( char const * name, motor::wire::iinput_slot_ptr_t in_, motor::wire::ioutput_slot_ptr_t out_ ) noexcept : 
                _name( name ), _in( in_ ), _out( out_ ) {}

            any( this_cref_t ) = delete ;
            any( this_rref_t rhv ) noexcept : _name( motor::move( rhv._name ) ),
                _in( motor::move( rhv._in ) ), _out( motor::move( rhv._out ) ){ }

            virtual ~any( void_t ) noexcept 
            {
                if( _in != nullptr ) _in->disconnect() ;
                if( _out != nullptr ) _out->disconnect() ;

                motor::release( motor::move( _in ) ) ;
                motor::release( motor::move( _out ) ) ;
            }

            char const * name( void_t ) const noexcept 
            {
                return _name ;
            }

            motor::string_t sname( void_t ) const noexcept
            {
                return motor::string_t( _name ) ;
            }

        public: // virtual 

            // return bool if anything changed.
            virtual bool_t update( void_t ) noexcept = 0 ;
            virtual motor::wire::inputs_t inputs( void_t ) noexcept = 0 ;

        public:

            motor::wire::iinput_slot_mtr_safe_t get_is( void_t ) noexcept
            {
                return motor::share( _in ) ;
            }

            motor::wire::ioutput_slot_mtr_safe_t get_os( void_t ) noexcept
            {
                return motor::share( _out ) ;
            }

        protected:

            template< typename T >
            motor::core::mtr_safe< T > get_is( void_t ) noexcept
            {
                return motor::share( reinterpret_cast< T * >( _in ) ) ;
            }

            template< typename T >
            T * borrow_is( void_t ) noexcept
            {
                return reinterpret_cast<T*>( _in ) ;
            }

            template< typename T >
            T const * borrow_is( void_t ) const noexcept
            {
                return reinterpret_cast<T *>( _in ) ;
            }

            template< typename T >
            motor::core::mtr_safe< T > get_os( void_t ) noexcept
            {
                return motor::share( reinterpret_cast<T *>( _out ) ) ;
            }

            template< typename T >
            T * borrow_os( void_t ) noexcept
            {
                return reinterpret_cast<T *>( _out ) ;
            }

            template< typename T >
            T const * borrow_os( void_t ) const noexcept
            {
                return reinterpret_cast<T const *>( _out ) ;
            }

        public:

            motor::wire::ioutput_slot_ptr_t borrow_os( void_t ) noexcept
            {
                return _out ;
            }
        };
        motor_typedef( any ) ;

        template< typename T >
        class variable : public any
        {
            using base_t = any ;
            motor_this_typedefs( variable< T > ) ;

        public:

            using in_t = motor::wire::input_slot< T > ;
            using out_t = motor::wire::output_slot< T > ;

        public:

            variable( char const * const name ) noexcept : base_t( name, 
                motor::shared( in_t() ), motor::shared( out_t() ) ) {}

            variable( this_cref_t ) = delete ;
            variable( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) ){}

            virtual ~variable( void_t ) noexcept {}

        public: 

            // @note inputs should have pushed value.
            virtual bool_t update( void_t ) noexcept 
            {
                T value ;
                if( base_t::borrow_is<in_t>()->get_value_and_reset( value ) )
                {
                    base_t::borrow_os<out_t>()->set_and_exchange( value ) ;
                    return true ;
                }
                return false ;
            }

            virtual motor::wire::inputs_t inputs( void_t ) noexcept 
            {
                return motor::wire::inputs_t( { { base_t::sname(), base_t::get_is() } }, false ) ;
            }


            // get output slot value
            T const & get_value( void_t ) const noexcept
            {
                return base_t::borrow_os<out_t>()->get_value() ;
            }

            // set input slot
            void_t set_value( T const & v ) noexcept
            {
                return base_t::borrow_is<in_t>()->set_value( v ) ;
            }
        };

        motor_typedefs( variable< bool_t >, boolv ) ;
        motor_typedefs( variable< int_t >, intv ) ;
        motor_typedefs( variable< uint_t >, uintv ) ;
        motor_typedefs( variable< float_t >, floatv ) ;
        motor_typedefs( variable< double_t >, doublev ) ;
    }
}