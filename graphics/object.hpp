
#pragma once

#include "api.h"
#include "protos.h"
#include "typedefs.h"

#include "id.hpp"

namespace motor
{
    namespace graphics
    {
        class MOTOR_GRAPHICS_API object
        {
            motor_this_typedefs( object ) ;

        private:

            motor::graphics::id_mtr_t _id = nullptr ;

        public:

            object( void_t ) 
            {
            }

            object( id_mtr_shared_t id ) : _id( motor::memory::copy_ptr( id ) ){}
            object( id_mtr_unique_t id ) noexcept : _id( id ){}
            object( this_cref_t rhv ) noexcept : _id( motor::memory::copy_ptr( rhv._id ) ) {}
            object( this_rref_t rhv ) noexcept : _id( motor::move( rhv._id ) ){}
            ~object( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _id = rhv._id ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _id = motor::move( rhv._id ) ;
                return *this ;
            }

        public:
            
            motor::graphics::id_mtr_shared_t get_id( void_t ) const noexcept 
            { 
                return motor::graphics::id_mtr_shared_t( _id ) ; 
            }
        };
    }
}
