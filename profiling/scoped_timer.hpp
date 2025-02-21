

#pragma once

#include "typedefs.h"

#include <motor/log/global.h>

#include <chrono>
#include <functional>

namespace motor
{
    namespace profiling
    {
        template< typename T = std::chrono::milliseconds >
        class scoped_timer
        {
            motor_this_typedefs( scoped_timer ) ;

        public:

            using this_clock_t = std::chrono::high_resolution_clock ;
            using end_funk_t = std::function< void_t ( char const *, T const & ) > ;

        private:

            char const * _name ;
            this_clock_t::time_point _start ;
            end_funk_t _funk ;
            
        public:

            // by default, result is printed to motor::log::status
            scoped_timer( char const * name ) noexcept : _name(name),
                _start(this_clock_t::now()),
                _funk([=]( char const * name, T const & dur )
            {
                motor::log::global::status<1024>( "%s : %zu units", 
                    _name, dur.count()) ;
            }){}

            scoped_timer( char const * name, end_funk_t f ) noexcept :
                _name( name ), _start(this_clock_t::now()), _funk( f )
            {}

            scoped_timer( end_funk_t f ) noexcept :
                _start(this_clock_t::now()), _funk( f )
            {}

            scoped_timer( this_cref_t ) noexcept = delete ;
            scoped_timer( this_rref_t rhv ) noexcept :
                _start( rhv._start ){}

            ~scoped_timer( void_t ) noexcept 
            {
                auto const dur = std::chrono::duration_cast<T>( 
                    this_clock_t::now() - _start ) ;

                _funk( _name, dur ) ;
            }
        };
    }
}