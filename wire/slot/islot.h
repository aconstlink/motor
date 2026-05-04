#pragma once

#include "../api.h"
#include "../typedefs.h"

namespace motor
{
    namespace wire
    {
        class MOTOR_WIRE_API islot
        {
            motor_this_typedefs( islot ) ;

        private:

            motor::wire::inode_ptr_t _owner = nullptr ;

        public:

            islot( void_t ) noexcept {}
            islot( motor::wire::inode_ptr_t owner ) noexcept : _owner( owner ) {}
            islot( this_rref_t rhv ) noexcept : _owner( motor::move( rhv._owner ) ){}
            islot( this_cref_t ) = delete ;

            virtual ~islot( void_t ) noexcept ;

        public:

            virtual void_t exchange( void_t ) noexcept = 0 ;
            virtual void_t disconnect( bool_t const propagate = true ) noexcept = 0 ;

        };

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
    }
}