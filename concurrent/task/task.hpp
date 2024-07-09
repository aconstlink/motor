
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

            struct task_funk_param {};
            motor_typedef( task_funk_param ) ;

            typedef std::function< void_t ( task_funk_param_in_t ) > task_funk_t ;
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

            tasks_t _ins ;
            tasks_t _outs;

        public:

            task( void_t ) noexcept : _ins( tasks_t::allocator_type("incomings")), 
                _outs( tasks_t::allocator_type("outgoings") )
            {
                _funk = [=]( this_t::task_funk_param_in_t ){} ;
            }

            task( task_funk_t f ) noexcept: _ins( tasks_t::allocator_type("incomings")), 
                _outs( tasks_t::allocator_type("outgoings"))
            {
                _funk = f ;
            }

            task( this_cref_t ) = delete ;
            task( this_rref_t rhv ) noexcept
            {
                _funk = std::move( rhv._funk ) ;
                _ins = std::move( rhv._ins ) ;
                _outs = std::move( rhv._outs ) ;
                _incomings = rhv._incomings ;
            }

            ~task( void_t ) noexcept
            {
                for( auto & t : _ins )
                {
                    motor::memory::release_ptr( t ) ;
                }

                for( auto & t : _outs )
                {
                    motor::memory::release_ptr( t ) ;
                }
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _funk = std::move( rhv._funk ) ;
                _ins = std::move( rhv._ins ) ;
                _outs = std::move( rhv._outs ) ;
                _incomings = rhv._incomings ;
                return *this ;
            }

            void_t set_funk( this_t::task_funk_t f ) noexcept
            {
                _funk = f ;
            }

        private:

            bool_t add_incoming( this_mtr_t ptr ) noexcept
            {
                auto iter = std::find( _ins.begin(), _ins.end(), ptr ) ;
                if ( iter != _ins.end() ) return false ;

                this_t::inc_incoming() ;
                _ins.push_back( motor::memory::copy_ptr( ptr ) ) ;
                return true ;
            }

            void_t disconnect_incoming( this_ptr_t ptr ) noexcept
            {
                auto iter = std::find( _ins.begin(), _ins.end(), ptr ) ;
                if ( iter == _ins.end() ) return ;

                _ins.erase( iter ) ;
                motor::memory::release_ptr( ptr ) ;
                this_t::dec_incoming() ;
            }

            void_t disconnect_outgoing( this_ptr_t ptr ) noexcept
            {
                auto iter = std::find( _outs.begin(), _outs.end(), ptr ) ;
                if ( iter == _outs.end() ) return ;

                _outs.erase( iter ) ;
                motor::memory::release_ptr( ptr ) ;
            }

        public: // this is what the user should use only 

            this_mtr_safe_t then( this_mtr_safe_t other ) noexcept
            {
                other->add_incoming( this ) ;
                _outs.emplace_back( other ) ;
                return other ;
            }

            void_t disconnect( void_t ) noexcept
            {
                // disconnect from all incomings
                for( auto * ptr : _ins )
                {
                    // disconnect this from ptr
                    ptr->disconnect_outgoing( this ) ;
                    motor::memory::release_ptr( ptr ) ;
                }
                _ins.clear() ;

                // disconnect from all outgoings
                for ( auto * ptr : _outs )
                {
                    // disconnect this from ptr
                    ptr->disconnect_incoming( this ) ;
                    motor::memory::release_ptr( ptr ) ;
                }
                _outs.clear() ;
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

            struct cleaner_accessor
            {
                static void_t move_out_all( this_mtr_t t, motor::vector< this_mtr_t > & tasks ) noexcept
                {
                    t->move_out_all( tasks ) ;
                }
            };
            friend struct cleaner_accessor ;

        private:

            void_t schedule( motor::vector< this_mtr_t > & tasks ) noexcept
            {
                tasks.reserve( tasks.capacity() + _outs.size() ) ;

                for ( auto & t : _outs ) tasks.emplace_back( motor::memory::copy_ptr( t ) ) ;
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

                this_t::task_funk_param_t p ;
                _funk( p ) ;

                _incomings = _ins.size() ;

                return true ;
            }

            bool_t resume( void_t ) noexcept
            {
                return this_t::dec_incoming() ;
            }

            void_t move_out_all( motor::vector< this_mtr_t > & tasks ) noexcept
            {
                tasks.reserve( tasks.capacity() + _ins.size() + _outs.size() )  ;
                for ( auto & t : _ins ) tasks.emplace_back( t ) ;
                for ( auto & t : _outs ) tasks.emplace_back( t ) ;
                _incomings = 0 ;
                _outs.clear() ;
                _ins.clear() ;
            }
        };
        motor_typedef( task ) ;
    }
}
