
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../sync_object.hpp"

#include <motor/std/vector>
#include <atomic>

namespace motor
{
    namespace concurrent
    {
        class task
        {
            motor_this_typedefs( task ) ;

        public:

            typedef std::function< void_t ( this_mtr_t ) > task_funk_t ;
            motor_typedefs( motor::vector< this_mtr_t >, tasks ) ;

        private: // user execution function

            task_funk_t _funk ;

        private: // refs

            motor::concurrent::mutex_t _incomings_mtx ;
            size_t _incomings = 0 ;

            bool_t dec_incoming( void_t ) noexcept 
            {
                motor::concurrent::lock_guard_t lk( _incomings_mtx ) ;
                if( --_incomings == size_t(-1) ) _incomings = 0 ;
                return _incomings == 0 ;
            }

            void_t inc_incoming( void_t ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _incomings_mtx ) ;
                ++_incomings ;
            }

        private: //

            tasks_t _inbetweens ;
            tasks_t _thens ;

        public:

            task( void_t ) noexcept : _inbetweens( tasks_t::allocator_type("inbetweens")), _thens( tasks_t::allocator_type("thens"))
            {
                _funk = [=]( this_mtr_t ){} ;
            }

            task( task_funk_t f ) noexcept: _inbetweens( tasks_t::allocator_type("inbetweens")), _thens( tasks_t::allocator_type("thens"))
            {
                _funk = f ;
            }

            task( this_cref_t ) = delete ;
            task( this_rref_t rhv ) noexcept
            {
                _funk = std::move( rhv._funk ) ;
                _inbetweens = std::move( rhv._inbetweens ) ;
                _thens = std::move( rhv._thens ) ;
                _incomings = rhv._incomings ;
            }

            ~task( void_t ) noexcept
            {
                for( auto & t : _inbetweens )
                {
                    motor::memory::release_ptr( t ) ;
                }

                for( auto & t : _thens )
                {
                    motor::memory::release_ptr( t ) ;
                }
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _funk = std::move( rhv._funk ) ;
                _inbetweens = std::move( rhv._inbetweens ) ;
                _thens = std::move( rhv._thens ) ;
                _incomings = rhv._incomings ;
                return *this ;
            }

        public: // this is what the user should use only

            this_ptr_t in_between( this_mtr_t other ) noexcept
            {
                other->inc_incoming() ;
                _inbetweens.emplace_back( motor::memory::copy_ptr(other) ) ;
                return other ;
            }

            this_mtr_unique_t in_between( this_mtr_unique_t other ) noexcept
            {
                other->inc_incoming() ;
                _inbetweens.emplace_back( other ) ;
                return other ;
            }

            this_ptr_t then( this_mtr_t other ) noexcept
            {
                other->inc_incoming() ;
                _thens.emplace_back( motor::memory::copy_ptr(other) ) ;
                return other ;
            }

            this_mtr_unique_t then( this_mtr_unique_t other ) noexcept
            {
                other->inc_incoming() ;
                _thens.emplace_back( other ) ;
                return other ;
            }
            
        public: // accessor

            struct scheduler_accessor
            {
                static void_t schedule( this_mtr_t t, motor::vector< this_mtr_t > & tasks ) noexcept
                {
                    t->schedule( tasks ) ;
                }

                static bool_t will_execute( this_mtr_t t ) noexcept
                {
                    return t->will_execute() ;
                }

                static bool_t execute( this_mtr_t t ) noexcept 
                {
                    return t->execute( t ) ;
                }
            };
            friend struct scheduler_accessor ;

        private:

            void_t schedule( motor::vector< this_mtr_t > & tasks ) noexcept
            {
                tasks.reserve( tasks.capacity() + _inbetweens.size() + _thens.size() ) ;

                if( _inbetweens.size() == 0 )
                {
                    for( auto & t : _thens ) tasks.emplace_back( motor::memory::copy_ptr(t) ) ;
                    return ;
                }

                for( auto & c : _inbetweens )
                {
                    for( auto & t : _thens ) 
                    {
                        c->then( t ) ;
                    }
                    tasks.emplace_back( motor::memory::copy_ptr(c) ) ;
                }

                // t is not scheduled yet it is
                // delayed to the children
                for( auto & t : _thens ) 
                {
                    t->dec_incoming() ; 
                }
            }

            bool_t will_execute( void_t ) noexcept
            {
                return this_t::dec_incoming() ;
            }

            // the schedule will pass the resource handle
            // of this task to this function so the
            // executing function could add further tasks
            bool_t execute( this_ptr_t this_res ) noexcept 
            {
                if( _incomings != 0 ) return false ;

                _funk( this_res ) ;

                return true ;
            }

            bool_t resume( void_t ) noexcept
            {
                return this_t::dec_incoming() ;
            }
        };
        motor_typedef( task ) ;
    }
}
