

#pragma once

#include "islot.h"

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API iinput_slot : public islot
        {
            motor_this_typedefs( iinput_slot ) ;

        public:
            
            virtual ~iinput_slot( void_t ) noexcept ;

        public:

            virtual void_t exchange( void_t ) noexcept = 0 ;
            virtual void_t disconnect( bool_t const propagate = true ) noexcept = 0 ;
            virtual void_t disconnect( motor::wire::ioutput_slot_ptr_t, bool_t const propagate = true  ) noexcept = 0 ;
            virtual bool_t connect( motor::wire::ioutput_slot_mtr_safe_t, bool_t const propagate = true ) noexcept = 0 ;
        };
        motor_typedef( iinput_slot ) ;


        template< typename T >
        class input_slot : public iinput_slot
        {
            motor_this_typedefs( input_slot<T> ) ;
            motor_typedefs( output_slot<T>, this_output_slot ) ;

        private:

            bool_t _has_changed = false ;
            T _value ;
            this_output_slot_mtr_t _output_slot = nullptr ;

        public:

            input_slot( void_t ) noexcept {}
            input_slot( T const v ) noexcept : _value( v ) {}
            input_slot( this_rref_t rhv ) noexcept : _value( rhv._value ), _output_slot( motor::move(rhv._output_slot ) ) {}
            input_slot( this_cref_t ) = delete ;

            virtual ~input_slot( void_t ) noexcept
            {
                if( _output_slot != nullptr ) 
                {
                    _output_slot->disconnect( this ) ;
                    motor::memory::release_ptr( _output_slot ) ;
                }
            }

            virtual void_t exchange( void_t ) noexcept
            {
                if( _output_slot != nullptr ) 
                {
                    _value = _output_slot->get_value() ;
                }
            }

            virtual void_t disconnect( bool_t const propagate = true ) noexcept
            {
                if ( _output_slot == nullptr ) return ;
                if ( propagate ) _output_slot->disconnect( this, false ) ;
                motor::memory::release_ptr( motor::move( _output_slot ) ) ;

                //_value = T( 0 ) ;
            }

            virtual void_t disconnect( motor::wire::ioutput_slot_ptr_t sig, bool_t const propagate = true ) noexcept
            {
                if ( sig == _output_slot && _output_slot != nullptr )
                {
                    if ( propagate ) _output_slot->disconnect( this ) ;
                    motor::memory::release_ptr( motor::move( _output_slot ) ) ;
                }
            }

            virtual bool_t connect( motor::wire::ioutput_slot_mtr_safe_t s, bool_t const propagate = true ) noexcept
            {
                // first check is slot is already connected.
                if( _output_slot == s ) return true ;

                if ( auto * v = dynamic_cast<motor::wire::output_slot<T> *>( s.mtr() ); v != nullptr )
                {
                    // can connect
                    _output_slot = v ;
                    if( propagate )
                    {
                        s->connect( motor::share( this ), false ) ;
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

            // Returns the value of the slot
            // Does not reset the changed flag.
            // @see get_value_and_reset
            T const & get_value( void_t ) const noexcept
            {
                return _value ;
            }

            // get the value in v if the value has changed
            // returns the "changed flag" and also resets it
            bool_t get_value_and_reset( T & v ) noexcept
            {
                if( _has_changed ) 
                {
                    v = _value ;
                    _has_changed = false ;
                    return true ;
                }
                
                return false ;
            }

            // 
            void_t set_value( T const v ) noexcept
            {
                _value = v ;
                _has_changed = true ;
            }
        };
    }
}