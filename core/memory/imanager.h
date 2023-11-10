
#pragma once

#include "../api.h"
#include "typedefs.h"

namespace motor
{
    namespace memory
    {
        class MOTOR_CORE_API imanager
        {
        public:

            virtual void_ptr_t alloc( size_t const sib, motor::memory::purpose_cref_t purpose ) = 0 ;
            virtual void_ptr_t alloc( size_t const sib ) = 0 ;

            virtual void_t dealloc( void_ptr_t ) = 0 ;
            virtual size_t get_sib( void_t ) const = 0 ;

            virtual bool_t get_purpose( void_ptr_t, motor::memory::purpose_ref_t ) const = 0 ;
            virtual void_t dump_to_std( void_t ) const = 0 ;

            virtual void_t destroy( void_t ) = 0 ;
        };
    }
}