
#pragma once

#include "task.hpp"

#include <motor/memory/global.h>
#include <future>

namespace motor
{
    namespace concurrent
    {
        class loose_thread_scheduler
        {
            motor_this_typedefs( loose_thread_scheduler ) ;

        public:

            motor_typedefs( motor::vector< motor::concurrent::task_mtr_t >, tasks ) ;

        private:

            struct shared_data
            {
                motor::concurrent::mutex_t mtx ;
                this_ptr_t owner = nullptr ;

                shared_data( void_t ) noexcept {}
                shared_data( shared_data && rhv ) noexcept
                {
                    motor_move_member_ptr( owner, rhv ) ;
                }
            } ;
            motor_typedef( shared_data ) ;
            shared_data_ptr_t _sd = nullptr ;

            motor::concurrent::mutex_t _mtx ;
            tasks_t _tasks ;

            motor::concurrent::mutex_t _mtx_f ;
            motor::vector< std::future<void_t> > _futures ;

        public:

            loose_thread_scheduler( void_t ) noexcept
            {}

            loose_thread_scheduler( this_cref_t ) = delete ;
            loose_thread_scheduler( this_rref_t rhv ) noexcept
            {
                {
                    motor::concurrent::lock_t lk(rhv._sd->mtx) ;
                    rhv._sd->owner = this ;
                }
                motor_move_member_ptr( _sd, rhv ) ;
                _tasks = std::move( rhv._tasks ) ;
            }

            ~loose_thread_scheduler( void_t ) noexcept
            {
                this_t::deinit() ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                this_t::deinit() ;

                if( rhv._sd != nullptr )
                {
                    motor::concurrent::lock_t lk(rhv._sd->mtx) ;
                    rhv._sd->owner = this ;
                }
                motor_move_member_ptr( _sd, rhv ) ;
                _tasks = std::move( rhv._tasks ) ;
                return *this ;
            }


            void_t init( void_t ) noexcept
            {
                if( _sd != nullptr ) return ;

                shared_data sd ;
                sd.owner = this ;
                
                _sd = motor::memory::global_t::alloc( std::move( sd ), 
                    "[loose_thread_scheduler] : shared data" ) ;
            }

            void_t deinit( void_t ) noexcept
            {
                // need to wait for all futures to complete.
                {
                    for ( auto & f : _futures )
                    {
                        f.wait() ;
                    }
                    _futures.clear() ;
                }

                motor::memory::global_t::dealloc( _sd ) ;
                _sd = nullptr ;

                for( auto & t : _tasks ) motor::memory::release_ptr( t ) ;
            }

        private:

            void_t place_future( std::future< void_t > && f_ ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx_f ) ;
                for( auto & f : _futures )
                {
                    if( f.wait_for( std::chrono::microseconds(0) ) == std::future_status::ready )
                    {
                        f = std::move( f_ ) ;
                        return ;
                    }
                }
                _futures.emplace_back( std::move( f_ ) ) ;
            }

            void_t execute( task_mtr_rref_t t ) noexcept
            {
                if( motor::concurrent::task::scheduler_accessor::will_execute( t ) ) 
                {
                    auto f = std::async( std::launch::async, [=]( void_t )
                    {
                        motor::concurrent::task::scheduler_accessor::execute( t ) ;

                        tasks_t tasks ;
                        motor::concurrent::task::scheduler_accessor::schedule( t, tasks ) ;

                        // note. Here the tasks are passed by rvalue.
                        // this scheduler needs to take care of the task pointers.
                        _sd->owner->schedule( std::move( tasks ) ) ;

                        motor::memory::release_ptr( t ) ;
                    } ) ;
                    this_t::place_future( std::move(f) ) ;
                }
                else
                {
                    motor::memory::release_ptr( t ) ;
                }
            }

        public:

            // the schedule takes over the pointer
            void_t schedule( task_mtr_safe_t t ) noexcept
            {
                motor::concurrent::lock_t lk( _mtx ) ;
                _tasks.emplace_back( std::move( t ) ) ;
            }

            // the schedule takes over the pointers
            void_t schedule( tasks_rref_t tasks ) noexcept
            {
                motor::concurrent::lock_t lk( _mtx ) ;
                for( auto t : tasks )
                {
                    _tasks.emplace_back( std::move( t ) ) ;
                }
            }

            // can be called anytime tasks need to be send away
            // for execution.
            void_t update( void_t ) noexcept
            {
                tasks_t tasks ;
                {
                    motor::concurrent::lock_t lk( _mtx ) ;
                    tasks = std::move( _tasks ) ;
                }

                for( auto & t : tasks )
                {
                    this_t::execute( std::move( t ) ) ;
                }
            }

            void_t yield( std::function< bool_t ( void_t ) > funk ) noexcept 
            {
                while( funk() ) std::this_thread::yield() ;
            }
        };
        motor_typedef( loose_thread_scheduler ) ;
    }
}