#pragma once

#include "../imanager.h"

#include <motor/core/std/map>

#include <mutex>


namespace motor
{
    namespace memory
    {
        class MOTOR_CORE_API manager : public motor::memory::imanager
        {
            motor_this_typedefs( manager ) ;

            struct memory_info
            {
                size_t sib ;
                motor::memory::purpose_t purpose ;
            } ;
            typedef motor::core::map< void_ptr_t, memory_info > ptr_to_info_t ;

            typedef std::mutex mutex_t ;
            typedef std::unique_lock< mutex_t > lock_t ;

        private:

            mutable mutex_t _mtx ;
            size_t _allocated_sib = 0 ;
            ptr_to_info_t _ptr_to_info ;

        public:

            manager( void_t ) noexcept ;
            virtual ~manager( void_t ) noexcept ;

        public:

            static this_ptr_t create( void_t ) noexcept ;
            static void_t destroy( this_ptr_t ) noexcept ;

        public:

            virtual void_ptr_t alloc( size_t const sib, motor::memory::purpose_cref_t purpose ) noexcept ;
            virtual void_ptr_t alloc( size_t const sib ) noexcept ;
            virtual void_t dealloc( void_ptr_t ) noexcept ;
            virtual size_t get_sib( void_t ) const noexcept ;
            virtual bool_t get_purpose( void_ptr_t, motor::memory::purpose_ref_t ) const noexcept ;

            virtual void_t dump_to_std( void_t ) const noexcept ;

            virtual void_t destroy( void_t ) noexcept ;
        };
        motor_typedef( manager ) ;
    }
}