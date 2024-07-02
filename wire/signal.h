
#pragma once

#include "typedefs.h"
#include "variable.hpp"

#include <motor/memory/global.h>

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API isignal
        {
            motor_this_typedefs( isignal ) ;

        private:

            

        public:
            
            
            ~isignal( void_t ) noexcept ;

        public:

            virtual void_t disconnect( void_t ) noexcept = 0 ;
            virtual void_t disconnect( motor::wire::islot_ptr_t, bool_t const propagate = true ) noexcept = 0 ;
            virtual bool_t connect( motor::wire::islot_mtr_safe_t, bool_t const propagate = true ) noexcept = 0 ;


        };
        motor_typedef( isignal ) ;

        template< typename T >
        class signal : public isignal
        {
            motor_this_typedefs( signal<T> ) ;
            motor_typedefs( slot<T>, this_slot ) ;

        private:

            motor::vector< this_slot_mtr_t > _slots ;

        public:

            virtual ~signal( void_t ) noexcept
            {
                this_t::disconnect() ;
            }

            virtual void_t disconnect( void_t ) noexcept 
            {
                for ( auto * slot : _slots )
                {
                    slot->disconnect( this, false ) ;
                    motor::memory::release_ptr( slot ) ;
                }
                _slots.clear() ;
            }

            virtual void_t disconnect( motor::wire::islot_ptr_t s_in, bool_t const propagate = true ) noexcept 
            {
                auto iter = std::find_if( _slots.begin(), _slots.end(), [&] ( this_slot_mtr_t o ) { return o == s_in ; } ) ;
                if( iter != _slots.end() ) 
                {
                    if ( propagate ) s_in->disconnect( this, false ) ;
                    motor::memory::release_ptr( *iter ) ;
                    _slots.erase( iter ) ;
                }
            }

            virtual bool_t connect( motor::wire::islot_mtr_safe_t s, bool_t const propagate = true ) noexcept
            {
                if( auto * v = dynamic_cast< motor::wire::slot<T> * >( s.mtr() ); v != nullptr )
                {
                    // can connect
                    _slots.push_back( v ) ;
                    if( propagate )
                    {
                        v->connect( motor::share( this ), false ) ;
                    }
                    return true ;
                }

                motor::release( s ) ;
                return false ;
            }
        };
    }
}