#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "../observer.hpp"

#include <map>
#include <unordered_map>
#include <mutex>

#define MOTOR_MEMORY_USE_LINEAR_INFO_CONTAINER 0

#if MOTOR_MEMORY_USE_LINEAR_INFO_CONTAINER
#include <motor/concurrent/mrsw.hpp>
#endif

namespace motor
{
    namespace memory
    {
        class manager
        {
            motor_this_typedefs( manager ) ;

            // can not use motor std because that would trigger 
            // this manager which requires this map type.
            #if MOTOR_MEMORY_USE_LINEAR_INFO_CONTAINER
            struct memory_info
            {
                void_ptr_t ptr ;
                size_t sib ;
                size_t rc ; // refcount
                char_cptr_t purpose ;
            } ;
            using ptr_to_info_t = std::vector< memory_info > ;
            #else
            struct memory_info
            {
                size_t sib ;
                size_t rc ; // refcount
                char_cptr_t purpose ;
            } ;
            typedef std::unordered_map< void_ptr_t, memory_info > ptr_to_info_t ;
            #endif

            typedef std::mutex mutex_t ;
            typedef std::unique_lock< mutex_t > lock_t ;

            motor::memory::observer_ptr_t _observer = nullptr ;

        private:

            #if MOTOR_MEMORY_USE_LINEAR_INFO_CONTAINER
            mutable motor::concurrent::mrsw_t _mtx ;
            #else
            mutable mutex_t _mtx ;
            #endif

            size_t _allocated_sib = 0 ;
            ptr_to_info_t _ptr_to_info ;

            struct pimpl ;
            pimpl * _pimpl = nullptr ;

        public:

            manager( void_t ) noexcept ;
            manager( this_cref_t ) = delete ;
            manager( this_rref_t ) noexcept ;
            ~manager( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t  ) noexcept ;

        public: // managed interface

            void_ptr_t create( size_t const sib, char_cptr_t purpose ) noexcept ;
            void_ptr_t create( size_t const sib ) noexcept ;

            // duplicates a managed pointer.
            void_ptr_t create( void_ptr_t ) noexcept ;

            // returns same pointer if ref count is not 0
            // otherwise nullptr is returned
            void_ptr_t release( void_ptr_t, motor::memory::void_funk_t ) noexcept ;

        public: // raw interface
            
            void_ptr_t alloc( size_t const sib, char_cptr_t purpose ) noexcept ;
            void_ptr_t alloc( size_t const sib ) noexcept ;
            void_t dealloc( void_ptr_t ) noexcept ;
            size_t get_sib( void_t ) const noexcept ;
            bool_t get_purpose( void_ptr_t, char_cptr_t & ) const noexcept ;

        public:

#if MOTOR_MEMORY_OBSERVER
            motor::memory::observer_ptr_t get_observer( void_t ) noexcept ;
#endif

            void_t dump_to_std( void_t ) const noexcept ;

        private:

            void_ptr_t alloc( size_t const sib, char_cptr_t purpose, bool_t const managed ) noexcept ;

            void_t create_entry( void_ptr_t, size_t const, this_t::memory_info && ) noexcept ;
            size_t release_entry( void_ptr_t ) noexcept ;
            size_t release_entry_dealloc( void_ptr_t ) noexcept ;
        };
        motor_typedefs( manager, default_manager ) ;
    }
}