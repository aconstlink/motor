#pragma once

#include "api.h"
#include "protos.h"
#include "typedefs.h"
#include "obfuscator.hpp"

#include "handle.h"

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

            global( void_t ) ;
            global( this_cref_t ) = delete ;
            global( this_rref_t ) ;
            ~global( void_t ) ;
        
        public:

            static this_ptr_t init( void_t ) ;
            static void_t deinit( void_t ) ;
            static this_ptr_t get( void_t ) ;
            static system_ptr_t io_system( void_t ) ;

            static motor::io::load_handle_t load( motor::io::path_cref_t, motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) ;
            static motor::io::load_handle_t load( motor::io::path_cref_t, size_t const offset, size_t const sib, motor::io::obfuscator_rref_t = motor::io::obfuscator_t() ) ;

            static motor::io::store_handle_t store( motor::io::path_cref_t, char_cptr_t, size_t const ) ;
            static motor::io::store_handle_t store( motor::io::path_cref_t, char_cptr_t, size_t const, motor::io::obfuscator_rref_t ) ;
        };
        motor_typedef( global ) ;
    }
}