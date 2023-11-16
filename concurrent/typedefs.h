#pragma once

#include <motor/memory/global.h>
#include <motor/memory/allocator.hpp>

#include <motor/std/string>
#include <motor/std/map>
#include <motor/std/vector>

#include <motor/core/types.hpp>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>

#include <thread>
#include <functional>
#include <mutex>
#include <future>

namespace motor
{
    namespace concurrent
    {
        using namespace motor::core::types ;
        
        motor_typedefs( std::thread, thread ) ;
        motor_typedefs( std::mutex, mutex ) ;
        motor_typedefs( std::unique_lock<mutex_t>, lock ) ;
        motor_typedefs( std::lock_guard<mutex_t>, lock_guard ) ;

        using memory = motor::memory::global ;

        template< typename T >
        using allocator = motor::memory::allocator<T, motor::concurrent::memory > ;

        typedef std::function< void_t ( void_t ) > void_funk_t ;

        namespace detail
        {
            //typedef motor::ntd::map< motor::ntd::string_t, itask_ptr_t > task_map_t ;
        }

        //motor_typedefs( motor::concurrent::detail::task_map_t, task_map ) ;

        typedef std::future< void_t > future_t ;

        enum class schedule_type
        {
            loose,
            pool
        };
    }
}
