

#pragma once

#include "variable.hpp"

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API islot
        {
            motor_this_typedefs( islot ) ;

        public:

            
            virtual ~islot( void_t ) noexcept ;

        public:

            virtual void_t disconnect( void_t ) noexcept = 0 ;
            virtual void_t disconnect( motor::wire::isignal_ptr_t, bool_t const propagate = true  ) noexcept = 0 ;
        };
        motor_typedef( islot ) ;


        template< typename T >
        class slot : public islot
        {
            motor_this_typedefs( slot<T> ) ;
            motor_typedefs( signal<T>, this_signal ) ;

        private:

            this_signal_mtr_t _signal = nullptr ;

        public:

            virtual ~slot( void_t ) noexcept
            {
                if( _signal != nullptr ) 
                {
                    _signal->disconnect( this ) ;
                }
            }

            virtual void_t disconnect( void_t ) noexcept 
            {
            }

            virtual void_t disconnect( motor::wire::isignal_ptr_t sig, bool_t const propagate = true ) noexcept
            {
                if ( sig == _signal && _signal != nullptr )
                {
                    if ( propagate ) _signal->disconnect( this ) ;
                    motor::memory::release_ptr( motor::move( _signal ) ) ;
                }
            }

            virtual bool_t connect( motor::wire::isignal_mtr_safe_t s, bool_t const propagate = true ) noexcept
            {
                if ( auto * v = dynamic_cast<motor::wire::signal<T> *>( s.mtr() ); v != nullptr )
                {
                    // can connect
                    _signal = v ;
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