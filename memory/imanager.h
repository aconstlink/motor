
#pragma once

#include "../api.h"
#include "typedefs.h"

namespace motor
{
    namespace memory
    {
        class MOTOR_MEMORY_API imanager
        {
        public: // managed interface

            virtual void_ptr_t create( size_t const sib, char_cptr_t purpose ) noexcept  = 0 ;
            virtual void_ptr_t create( size_t const sib ) noexcept = 0 ;

            // duplicates a managed pointer.
            virtual void_ptr_t create( void_ptr_t ) noexcept = 0;

            // returns same pointer if ref count is not 0
            // otherwise nullptr is returned
            virtual void_ptr_t release( void_ptr_t ) noexcept = 0 ;

        public: // raw interface

            virtual void_ptr_t alloc( size_t const sib, char_cptr_t purpose ) = 0 ;
            virtual void_ptr_t alloc( size_t const sib ) = 0 ;

            virtual void_t dealloc( void_ptr_t ) = 0 ;
            virtual size_t get_sib( void_t ) const = 0 ;

            virtual bool_t get_purpose( void_ptr_t, char_cptr_t & ) const = 0 ;
            virtual void_t dump_to_std( void_t ) const = 0 ;

            virtual void_t destroy( void_t ) = 0 ;
        };
    }
}