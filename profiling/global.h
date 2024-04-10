

#pragma once

#include "api.h"
#include "typedefs.h"

#include "manager.h"

namespace motor{ namespace profiling { 

    class MOTOR_PROFILING_API global
    {
        motor_this_typedefs( global ) ;

    private:

        static motor::profiling::manager_t _mgr ;

    public:

        global( void_t ) noexcept ;
        global( this_cref_t ) = delete ;
        global( this_rref_t ) noexcept ;
        ~global( void_t ) noexcept ;

    public:

        #if MOTOR_PROFILING

        static motor::profiling::manager_ref_t manager( void_t ) noexcept
        {
            return _mgr ;
        }

        static void_t push( motor::string_cref_t name ) noexcept ;
        static void_t pop( void_t ) noexcept ;

        #else
        static inline void_t push( motor::string_cref_t ) noexcept {}
        static inline void_t pop( void_t ) noexcept {}
        #endif

        static void_t deinit( void_t ) noexcept ;
    };
    motor_typedef( global ) ;
} }