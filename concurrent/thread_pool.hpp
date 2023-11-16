#pragma once

#include "typedefs.h"
#include "task/task.hpp"

namespace motor
{
    namespace concurrent
    {
        class thread_pool
        {
            motor_this_typedefs( thread_pool ) ;
            motor_typedefs( motor::vector< motor::concurrent::task_ptr_t >, tasks ) ;

            struct thread_data
            {
                thread_data( void_t ) noexcept {}
                thread_data( thread_data && rhv ) noexcept
                {
                    thread = std::move( rhv.thread ) ;
                }
                ~thread_data( void_t ) noexcept {}

                std::thread thread ;
                std::mutex mtx ;
            };
            motor_typedef( thread_data ) ;

            struct shared_data
            {
                motor_this_typedefs( shared_data ) ;

                motor::concurrent::mutex_t pool_mtx ;
                motor::vector< thread_data_ptr_t > pool ;
                std::condition_variable pool_cv ;

                motor::concurrent::mutex_t tasks_mtx ;
                motor::vector< motor::concurrent::task_mtr_t > tasks ;
                std::condition_variable tasks_cv ;

                size_t threads_running = 0 ;
                size_t threads_max = 0 ;

                shared_data( void_t ) noexcept {}
                shared_data( this_rref_t rhv ) noexcept
                {
                    pool = std::move( rhv.pool ) ;
                    tasks = std::move( rhv.tasks ) ;
                    threads_running = rhv.threads_running ;
                    threads_max = rhv.threads_max ;
                }

                ~shared_data( void_t ) noexcept
                {
                    for( auto * t : tasks ) motor::memory::release_ptr( t ) ;
                }

            public: // thread pool access

                void_t inc_thread_count( void_t ) noexcept
                {
                    motor::concurrent::lock_guard_t lk( pool_mtx ) ;
                    threads_running++ ;
                }

                void_t dec_thread_count( void_t ) noexcept
                {
                    motor::concurrent::lock_guard_t lk( pool_mtx ) ;
                    threads_running-- ;
                }

                void_t add_task( motor::concurrent::task_mtr_rref_t t ) noexcept
                {
                    {
                        motor::concurrent::lock_guard_t lk( tasks_mtx ) ;
                        tasks.emplace_back( t ) ;
                    }
                    tasks_cv.notify_one() ;
                }

                bool_t contains_thread_id( std::thread::id const & id ) noexcept
                {
                    motor::concurrent::lock_t lk( pool_mtx ) ;
                    for( auto const & td : pool )
                    {
                        if( td->thread.get_id() == id ) return true ;
                    }
                    return false ;
                }

                void_t yield( void_t ) noexcept
                {
                    {
                        motor::concurrent::lock_t lk( pool_mtx ) ;
                        threads_running-- ;
                    }
                    pool_cv.notify_one() ;
                }

                void_t resume( void_t ) noexcept
                {
                    motor::concurrent::lock_t lk( pool_mtx ) ;
                    threads_running++ ;
                }

            public: // worker thread access

                bool_t wait_for_thread_wakeup( void_t ) noexcept
                {
                    motor::concurrent::lock_t lk( pool_mtx ) ;
                    while( threads_running >= threads_max ) 
                    {
                        pool_cv.wait( lk ) ;
                        if( threads_max == 0 ) return false ;
                    }
                    threads_running++ ;

                    return true ;
                }

                bool_t wait_for_tasks( void_t ) noexcept
                {
                    motor::concurrent::lock_t lk( tasks_mtx ) ;
                    while( tasks.size() == 0 ) 
                    {
                        tasks_cv.wait( lk ) ;
                        if( threads_max == 0 ) return false ;
                    }
                    return true ;
                }
            } ;
            motor_typedef( shared_data ) ;
            shared_data_ptr_t _sd = nullptr ;

            typedef std::function< void_t ( void_t ) > thread_funk_t ;
            thread_funk_t _thread_funk ;

        public:

            thread_pool( void_t ) 
            {}

            thread_pool( this_rref_t rhv ) noexcept
            {
                motor_move_member_ptr( _sd, rhv ) ;
                _thread_funk = rhv._thread_funk ;
            }

            ~thread_pool( void_t ) noexcept
            {
                this_t::shutdown() ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                motor_move_member_ptr( _sd, rhv ) ;
                _thread_funk = rhv._thread_funk ;

                return *this ;
            }

        public:

            void_t init( void_t ) 
            {
                size_t const num_threads = std::thread::hardware_concurrency() ;
                size_t const thread_pool_size = num_threads << 2 ;

                {
                    _sd = motor::memory::global_t::alloc( shared_data(), 
                        "[thread_pool] : shared data" ) ;
                
                    _sd->pool.resize( thread_pool_size ) ;
                    _sd->threads_max = num_threads ;
                }

                for( size_t i=0; i<thread_pool_size; ++i )
                {
                    thread_data_ptr_t res = motor::memory::global_t::alloc( 
                        thread_data(), "[motor:concurrent] : thread_data" ) ;

                    auto thd = std::thread( [=]( void_t )
                    {
                        while( _sd->threads_max != 0 )
                        {
                            if( !_sd->wait_for_thread_wakeup() ) break ;
                            if( !_sd->wait_for_tasks() ) break ;

                            // go into taking tasks loop
                            while( _sd->threads_running <= _sd->threads_max )
                            {
                                motor::concurrent::task_ptr_t task ;

                                // take task
                                {
                                    motor::concurrent::lock_guard_t lk( _sd->tasks_mtx ) ;
                                    if( _sd->tasks.size() == 0 ) break ;
                                    
                                    task = _sd->tasks[0] ;
                                    _sd->tasks[0] = _sd->tasks[_sd->tasks.size()-1] ;
                                    _sd->tasks.resize( _sd->tasks.size() - 1 ) ;
                                }

                                if( motor::concurrent::task::scheduler_accessor::will_execute( task ) ) 
                                {
                                    motor::concurrent::task::scheduler_accessor::execute( task ) ;

                                    tasks_t tasks ;
                                    motor::concurrent::task::scheduler_accessor::schedule( task, tasks ) ;

                                    for( auto & t : tasks ) this->schedule( std::move(t)  ) ;
                                }

                                motor::memory::release_ptr( task ) ;
                            }

                            _sd->dec_thread_count() ;
                        }
                    } ) ;

                    _sd->pool[i] = res ;
                    _sd->pool[i]->thread = std::move( thd ) ;
                }
            }
        public:

            // yield as long as the yield function returns true
            bool_t yield( std::function< bool_t ( void_t ) > yield_funk ) noexcept
            {
                if( !_sd->contains_thread_id( std::this_thread::get_id() ) ) return false ;
                
                // swap in a new thread from the pool
                _sd->yield() ;
                while( yield_funk() ) std::this_thread::yield() ;
                // put back one thread back to sleep
                _sd->resume() ;

                return true ;
            }

            // yield while so is NOT signaled
            bool_t yield( motor::concurrent::sync_object_ptr_t so ) noexcept
            {
                if( !_sd->contains_thread_id( std::this_thread::get_id() ) ) return false ;

                // swap in a new thread from the pool
                _sd->yield() ;
                while( !so->is_signaled() ) std::this_thread::yield() ;
                // put back one thread back to sleep
                _sd->resume() ;
                
                return true ;
            }

            void_t schedule( motor::concurrent::task_mtr_moved_t t ) noexcept
            {
                _sd->add_task( std::move( t ) ) ;
            }

            void_t shutdown( void_t ) noexcept 
            {
                if( _sd == nullptr ) return ;
                _sd->threads_max = 0 ;
                _sd->tasks_cv.notify_all() ;
                _sd->pool_cv.notify_all() ;
                
                for( auto & item : _sd->pool )
                {
                    if( item->thread.joinable() ) item->thread.join() ;
                    motor::memory::global_t::dealloc( item ) ;
                }
                motor::memory::global_t::dealloc( _sd ) ;
                _sd = nullptr ;
            }
        };
        motor_typedef( thread_pool ) ;
    }
}