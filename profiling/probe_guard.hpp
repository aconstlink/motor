

#pragma once

#include "global.h"

namespace motor{ namespace profiling {

    class probe_guard
    {
    private:

        // static probe id
        size_t _id ;

        // run-time probe array idx
        size_t _idx ;

    public:

        inline probe_guard( size_t const id ) noexcept : _id( id )
        {
            _idx = motor::profiling::global_t::manager().begin_probe( id ) ;
        }

        inline ~probe_guard( void_t ) noexcept
        {
            motor::profiling::global_t::manager().end_probe( _idx ) ;
        }
    };

    #if MOTOR_PROFILING 
    #define MOTOR_PROFILING_CAT_(a, b) a ## b ##_
    #define MOTOR_PROFILING_CAT(a, b) MOTOR_PROFILING_CAT_(a, b)
    #define MOTOR_PROFILING_VARNAME(Var) MOTOR_PROFILING_CAT( Var, __LINE__ )

    #define MOTOR_PROBE( category, name ) \
        static size_t const MOTOR_PROFILING_VARNAME( _motor_probe_id_ ) = \
            motor::profiling::global_t::manager().gen_id( category, name ) ;\
        motor::profiling::probe_guard MOTOR_PROFILING_VARNAME( _motor_probe_guard_ )( MOTOR_PROFILING_VARNAME(_motor_probe_id_) ) ;
    #else
    #define MOTOR_PROBE( name )
    #endif

} }