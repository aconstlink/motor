// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include "api.h"
#include "typedefs.h"
#include "protos.h"
#include "sync_object.hpp"
#include "task/task.hpp"

namespace motor
{
    namespace concurrent
    {
        class MOTOR_CONCURRENT_API global
        {
            motor_this_typedefs( global ) ;

        public:

            using notify_funk_t = std::function< void_t ( void_t ) > ;

        private:

            
            struct singleton_data ;
            static singleton_data * _dptr ;
            static mutex_t _mtx ;

        private:

            static singleton_data * init( void_t ) noexcept ;

        public: 

            static size_t register_work_event_notify( notify_funk_t ) noexcept ;
            static void_t unregister_work_event_notify( size_t const ) noexcept ;

            static void_t deinit( void_t ) noexcept ;
            static void_t update( void_t ) noexcept ;

            // yield the current thread of execution 
            // until the sync object is signaled
            // if the thread is part of the thread pool, the thread
            // will be exchanged with another worker until the old thread  
            // resumes execution.
            static void_t yield( std::function< bool_t ( void_t ) > funk ) noexcept ;

            static void_t schedule( motor::concurrent::task_mtr_safe_t, motor::concurrent::schedule_type const ) noexcept ;


            // creates a task and leaves ownership to caller.
            static task_mtr_safe_t make_task( motor::concurrent::task_t::task_funk_t f ) noexcept ;
        };
        motor_typedef( global ) ;
    }
}