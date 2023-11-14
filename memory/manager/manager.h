#pragma once

#include "../imanager.h"

#include <motor/std/map>

#include <mutex>


namespace motor
{
    namespace memory
    {
        class MOTOR_MEMORY_API manager : public motor::memory::imanager
        {
            motor_this_typedefs( manager ) ;

            struct memory_info
            {
                size_t sib ;
                size_t rc ; // refcount
                char_cptr_t purpose ;
            } ;
            typedef motor::map< void_ptr_t, memory_info > ptr_to_info_t ;

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


        public: // managed interface

            virtual void_ptr_t create( size_t const sib, char_cptr_t purpose ) noexcept ;
            virtual void_ptr_t create( size_t const sib ) noexcept ;

            // duplicates a managed pointer.
            virtual void_ptr_t create( void_ptr_t ) noexcept ;

            // returns same pointer if ref count is not 0
            // otherwise nullptr is returned
            virtual void_ptr_t release( void_ptr_t ) noexcept ;

        public: // raw interface
            
            virtual void_ptr_t alloc( size_t const sib, char_cptr_t purpose ) noexcept ;
            virtual void_ptr_t alloc( size_t const sib ) noexcept ;
            virtual void_t dealloc( void_ptr_t ) noexcept ;
            virtual size_t get_sib( void_t ) const noexcept ;
            virtual bool_t get_purpose( void_ptr_t, char_cptr_t & ) const noexcept ;

            virtual void_t dump_to_std( void_t ) const noexcept ;

            virtual void_t destroy( void_t ) noexcept ;

        private:

            void_ptr_t alloc( size_t const sib, char_cptr_t purpose, bool_t const managed ) noexcept ;
            void_ptr_t dealloc( void_ptr_t, bool_t const managed ) noexcept ;
        };
        motor_typedef( manager ) ;
    }
}