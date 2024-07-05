

#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API iinput_slot
        {
            motor_this_typedefs( iinput_slot ) ;

        public:

            
            virtual ~iinput_slot( void_t ) noexcept ;

        public:

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

            T _value ;
            this_output_slot_mtr_t _output_slot = nullptr ;

        public:

            input_slot( void_t ) noexcept : _value( 0 ) {}
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

            virtual void_t disconnect( bool_t const propagate = true ) noexcept
            {
                if ( _output_slot == nullptr ) return ;
                if ( propagate ) _output_slot->disconnect( this ) ;
                motor::memory::release_ptr( motor::move( _output_slot ) ) ;
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
        };
    }
}