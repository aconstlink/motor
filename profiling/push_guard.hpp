

#pragma once

#include "global.h"

namespace motor{ namespace profiling {

    class push_guard
    {
    public:

        inline push_guard( motor::string_cref_t name ) noexcept
        {
            motor::profiling::global_t::push( name ) ;
        }

        inline ~push_guard( void_t ) noexcept
        {
            motor::profiling::global_t::pop() ;
        }
    };

    #if MOTOR_PROFILING 
    #define MOTOR_PROFILING_CAT_(a, b) a ## b ##_
    #define MOTOR_PROFILING_CAT(a, b) MOTOR_PROFILING_CAT_(a, b)
    #define MOTOR_PROFILING_VARNAME(Var) MOTOR_PROFILING_CAT(Var, __LINE__)
    #define MOTOR_PROFILE( name ) motor::profiling::push_guard MOTOR_PROFILING_VARNAME( _xx_mtr_prof_pg_ ) ( name ) ;
    #else
    #define MOTOR_PROFILE( name )
    #endif

} }