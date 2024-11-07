
#pragma once

#include "islot.h"

#include <motor/memory/global.h>

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API ioutput_slot : public islot
        {
            motor_this_typedefs( ioutput_slot ) ;

        public:

            virtual ~ioutput_slot( void_t ) noexcept ;

        public:

            virtual void_t exchange( void_t ) noexcept = 0 ;
            virtual void_t disconnect( bool_t const propagate = true ) noexcept = 0 ;
            virtual void_t disconnect( motor::wire::iinput_slot_ptr_t, bool_t const propagate = true ) noexcept = 0 ;
            virtual bool_t connect( motor::wire::iinput_slot_mtr_safe_t, bool_t const propagate = true ) noexcept = 0 ;
        };
        motor_typedef( ioutput_slot ) ;

        template< typename T >
        class output_slot : public ioutput_slot
        {
            motor_this_typedefs( output_slot<T> ) ;
            motor_typedefs( input_slot<T>, this_input_slot ) ;

        private:

            bool_t _has_changed = false ;
            T _value ;
            motor::vector< this_input_slot_mtr_t > _inputs ;

        public:

            output_slot( void_t ) noexcept {}
            output_slot( T const v ) noexcept : _value(v) {}
            output_slot( this_rref_t rhv ) noexcept : _inputs( std::move( rhv._inputs) ), _value( rhv._value ) {}

            output_slot( this_cref_t ) = delete ;

            virtual ~output_slot( void_t ) noexcept
            {
                this_t::disconnect() ;
            }

            virtual void_t exchange( void_t ) noexcept 
            {
                // only push data if a changed happened.
                if( _has_changed )
                {
                    for ( auto * slot : _inputs )
                    {
                        slot->set_value( _value ) ;
                    }
                    _has_changed = false ;
                }
            }

            virtual void_t disconnect( bool_t const propagate = true ) noexcept
            {
                for ( auto * slot : _inputs )
                {
                    if( propagate ) slot->disconnect( this, false ) ;
                    motor::memory::release_ptr( slot ) ;
                }
                _inputs.clear() ;
            }

            virtual void_t disconnect( motor::wire::iinput_slot_ptr_t s_in, bool_t const propagate = true ) noexcept 
            {
                auto iter = std::find_if( _inputs.begin(), _inputs.end(), [&] ( this_input_slot_mtr_t o ) { return o == s_in ; } ) ;
                if( iter != _inputs.end() ) 
                {
                    if ( propagate ) s_in->disconnect( this, false ) ;
                    motor::memory::release_ptr( *iter ) ;
                    _inputs.erase( iter ) ;
                }
            }

            virtual bool_t connect( motor::wire::iinput_slot_mtr_safe_t s, bool_t const propagate = true ) noexcept
            {
                // first check is slot is already connected.
                for( auto * in_s : _inputs ) 
                {
                    if( in_s == s ) return true ; 
                }

                if( auto * v = dynamic_cast< motor::wire::input_slot<T> * >( s.mtr() ); v != nullptr )
                {
                    // can connect
                    _inputs.push_back( v ) ;
                    if( propagate )
                    {
                        v->connect( motor::share( this ), false ) ;
                    }
                    return true ;
                }

                motor::release( s ) ;
                return false ;
            }

            this_ref_t operator = ( T const & v ) noexcept
            {
                _value = v ;
                _has_changed = true ;
                return *this ;
            }

        public:

            T const & get_value( void_t ) const noexcept
            {
                return _value ;
            }

            void_t set_value( T const v ) noexcept
            {
                _value = v ;
                _has_changed = true ;
            }

            // set the value and immediately exchange with
            // connected input slots.
            void_t set_and_exchange( T const v ) noexcept
            {
                _value = v ;
                _has_changed = true ;
                this->exchange() ;
            }
        };
    }
}