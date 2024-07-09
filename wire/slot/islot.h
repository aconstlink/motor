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
    }
}