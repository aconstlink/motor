#pragma once

#include "api.h"
#include "protos.h"
#include "typedefs.h"
#include "obfuscator.hpp"

#include "system.h"

#include <mutex>

namespace motor
{
    namespace io
    {
        class MOTOR_IO_API global
        {
            motor_this_typedefs( global ) ;

        private:

            motor::io::system_ptr_t _ios = nullptr ;

        private:

            static std::mutex _mtx ;
            static this_ptr_t _ptr ;

        public:

            global( void_t ) noexcept;
            global( this_cref_t ) = delete ;
            global( this_rref_t ) noexcept ;
            ~global( void_t ) noexcept ;
        
        public:

            static this_ptr_t init( void_t ) noexcept ;
            static void_t deinit( void_t ) noexcept ;
            static this_ptr_t get( void_t ) noexcept ;
            static system_ptr_t io_system( void_t ) noexcept;

            static motor::io::system_t::load_handle_t load( motor::io::path_cref_t, 
                std::launch const lt = std::launch::deferred, motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) noexcept ;

            static motor::io::system_t::load_handle_t load( motor::io::path_cref_t, size_t const offset, size_t const sib, 
                std::launch const lt = std::launch::deferred, motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) noexcept ;

            static motor::io::system_t::store_handle_t store( motor::io::path_cref_t, char_cptr_t, size_t const, 
                std::launch const lt = std::launch::deferred ) noexcept ;

            static motor::io::system_t::store_handle_t store( motor::io::path_cref_t, char_cptr_t, size_t const, motor::io::obfuscator_rref_t, 
                std::launch const lt = std::launch::deferred ) noexcept ;
        };
        motor_typedef( global ) ;
    }
}