

#pragma once

#include "api.h"
#include "typedefs.h"

#include "manager.h"

namespace motor{ namespace profiling { 

    class MOTOR_PROFILING_API global
    {
        motor_this_typedefs( global ) ;

    private:

        inline static motor::profiling::manager_t _mgr ;

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
        
        #endif

        static void_t deinit( void_t ) noexcept ;
    };
    motor_typedef( global ) ;
} }