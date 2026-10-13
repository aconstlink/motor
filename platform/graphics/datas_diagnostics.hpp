// SPDX-License-Identifier: MIT
#pragma once

#include "../typedefs.h"
#include <chrono>
#include <cstdint>

namespace motor { namespace platform { namespace diagnostic
{
    using namespace motor::core::types ;
    enum class mode { off, counters, timings } ;
    enum class region { reader, writer, entry, callback } ;

    struct sample
    {
        std::uint64_t calls = 0, nanoseconds = 0 ;
    } ;
    struct counters
    {
        mode recording = mode::off ;
        sample regions[4] ;
        std::uint64_t recursive = 0, failed_try = 0 ;
    } ;
    // Thread-local collection avoids introducing another contended lock.
    // Read on the owning thread; aggregate only after workers have joined.
    inline thread_local counters local ;

    template< bool Enabled >
    class scope
    {
        using clock = std::chrono::steady_clock ;
        region _region ;
        mode _mode ;
        clock::time_point _start ;
    public:
        explicit scope( region const r ) noexcept : _region( r ), _mode( mode::off )
        {
            if constexpr( Enabled )
            {
                _mode = local.recording ;
                if( _mode != mode::off ) ++local.regions[int( r )].calls ;
                if( _mode == mode::timings ) _start = clock::now() ;
            }
        }
        void_t finish( void_t ) noexcept
        {
            if constexpr( Enabled )
            {
                if( _mode == mode::timings ) local.regions[int( _region )].nanoseconds +=
                    std::uint64_t( std::chrono::duration_cast<std::chrono::nanoseconds>( clock::now() - _start ).count() ) ;
                _mode = mode::off ;
            }
        }
        ~scope( void_t ) noexcept { finish() ; }
        scope( scope const & ) = delete ;
        scope & operator=( scope const & ) = delete ;
    } ;

    template<>
    class scope<false>
    {
    public:
        explicit scope( region ) noexcept {}
        void_t finish( void_t ) noexcept {}
    } ;

    template< typename Lock, bool Enabled, region Region >
    class measured_lock
    {
        scope<Enabled> _time ;
        Lock _lock ;
    public:
        template< typename Mutex >
        explicit measured_lock( Mutex & mutex ) noexcept : _time( Region ), _lock( mutex )
        { _time.finish() ; }
    } ;
} } }
