
#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../sync_object.hpp"
#include "../mrsw.hpp"

#include <motor/std/hash_map>
#include <motor/std/vector_pod>

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
            motor_typedefs( motor::vector_pod< this_mtr_t >, tasks ) ;

        private: // user execution function

            task_funk_t _funk ;

        private: // refs

            motor::concurrent::mutex_t _incomings_mtx ;
            size_t _incomings = 0 ;

            mutable motor::concurrent::mrsw_t _exec_mtx ;
            bool_t _is_executing = false ;

            mutable motor::concurrent::sync_object_t _so ;

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

            task( void_t ) noexcept : _funk( [=]( this_t::task_funk_param_in_t ){} ) 
            {}

            task( task_funk_t f ) noexcept : _funk( std::move( f ) ) 
            {}

            task( this_cref_t ) = delete ;
            task( this_rref_t rhv ) noexcept
            {
                _funk = std::move( rhv._funk ) ;
                _ins = std::move( rhv._ins ) ;
                _outs = std::move( rhv._outs ) ;
                _incomings = rhv._incomings ;
                _is_executing = rhv._is_executing ;
                _so = std::move( rhv._so ) ;
            }

            ~task( void_t ) noexcept
            {
                this_t::release_ins() ;
                this_t::release_outs() ;
            }

            void_t release_ins( void_t ) noexcept
            {
                for ( size_t i = 0; i < _ins.size(); ++i )
                    motor::memory::release_ptr( _ins[ i ] ) ;
            }
            void_t release_outs( void_t ) noexcept
            {
                for ( size_t i = 0; i < _outs.size(); ++i )
                    motor::memory::release_ptr( _outs[ i ] ) ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                this_t::release_ins() ;
                this_t::release_outs() ;

                _ins = std::move( rhv._ins ) ;
                _outs = std::move( rhv._outs ) ;

                _funk = std::move( rhv._funk ) ;
                
                _incomings = rhv._incomings ;
                _is_executing = rhv._is_executing ;
                _so = std::move( rhv._so ) ;
                return *this ;
            }

            void_t set_funk( this_t::task_funk_t f ) noexcept
            {
                _funk = f ;
            }

            bool_t is_executing( void_t ) const noexcept
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( _exec_mtx ) ;
                return _is_executing ;
            }

            void_t wait_until_executed( void_t ) const noexcept
            {
                _so.wait() ;
            }

        private:

            void_t set_executing( bool_t const b ) noexcept
            {
                if( !b ) _so.set_and_signal() ;
                else _so.reset() ;

                motor::concurrent::mrsw_t::writer_lock_t lk( _exec_mtx ) ;
                _is_executing = b ;
            }

            bool_t add_incoming( this_mtr_t ptr ) noexcept
            {
                #if 1
                if( _ins.contains(ptr) ) return false ;
                #else
                auto iter = std::find( _ins.begin(), _ins.end(), ptr ) ;
                if ( iter != _ins.end() ) return false ;
                #endif

                this_t::inc_incoming() ;
                _ins.push_back( motor::memory::copy_ptr( ptr ) ) ;
                return true ;
            }

            void_t disconnect_incoming( this_ptr_t ptr ) noexcept
            {
                #if 1
                _ins.erase( ptr ) ;
                #else
                auto iter = std::find( _ins.begin(), _ins.end(), ptr ) ;
                if ( iter == _ins.end() ) return ;
                _ins.erase( iter ) ;
                #endif
                
                motor::memory::release_ptr( ptr ) ;
                this_t::dec_incoming() ;
            }

            void_t disconnect_outgoing( this_ptr_t ptr ) noexcept
            {
                #if 1
                _outs.erase( ptr ) ;
                #else
                auto iter = std::find( _outs.begin(), _outs.end(), ptr ) ;
                if ( iter == _outs.end() ) return ;
                _outs.erase( iter ) ;
                #endif
                motor::memory::release_ptr( ptr ) ;
            }

        public: // this is what the user should use only 

            this_mtr_safe_t then( this_mtr_safe_t other ) noexcept
            {
                other->add_incoming( this ) ;
                _outs.push_back( other ) ;
                return other ;
            }

            void_t disconnect( void_t ) noexcept
            {
                // disconnect from all incomings
                for( size_t i=0; i<_ins.size(); ++i )
                {
                    // disconnect this from ptr
                    _ins[i]->disconnect_outgoing( this ) ;
                    motor::memory::release_ptr( _ins[i] ) ;
                }
                _ins.clear() ;

                // disconnect from all outgoings
                for( size_t i=0; i<_outs.size(); ++i )
                {
                    // disconnect this from ptr
                    _outs[i]->disconnect_incoming( this ) ;
                    motor::memory::release_ptr( _outs[i] ) ;
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
                static void_t move_out_all( this_mtr_t t, this_t::tasks_t & tasks ) noexcept
                {
                    t->move_out_all( tasks ) ;
                }
            };
            friend struct cleaner_accessor ;


            struct tier_builder
            {
                struct tier
                {
                    tasks_t tasks ;

                    bool_t has_tasks( void_t ) const noexcept { return tasks.size() != 0 ; }
                };
                motor_typedefs( motor::vector< tier >, tiers ) ;
                tiers_t tiers ;

                struct build_result
                {
                    tiers_t tiers ;
                    size_t num_tasks = 0 ;
                    bool_t has_cylce = false ;
                };
                motor_typedef( build_result ) ;

                static void_t build( this_mtr_t start, build_result_out_t res ) noexcept
                {
                    size_t id = 0 ;
                    motor::hash_map< this_mtr_t, size_t > ids ;

                    size_t cur_tier = 0 ;

                    tiers_ref_t tiers = res.tiers ;
                    tiers.clear() ;

                    tiers.push_back( tier{ {start} } ) ;
                    ids[start] = id++ ;

                    while( tiers[cur_tier].has_tasks() )
                    {
                        tiers.resize( tiers.size() + 1 ) ;

                        for( size_t ti=0; ti< tiers[cur_tier].tasks.size(); ++ti )
                        //for( auto * t : tiers[cur_tier].tasks )
                        {
                            auto * t = tiers[cur_tier].tasks[ti] ;

                            for( size_t ti2=0; ti2<t->_outs.size(); ++ti2)
                            //for( auto * t2 : t->_outs )
                            {
                                auto * t2 = t->_outs[ti2] ;

                                // self check
                                {
                                    auto iter = ids.find( t2 ) ;
                                    if( iter != ids.end() ) continue ;
                                }

                                // check if candidate for tier
                                {
                                    bool_t complete = true ;
                                    for( size_t ti3=0; ti3<t2->_ins.size(); ++ti3 )
                                    //for ( auto * input_task : t2->_ins )
                                    {
                                        auto * input_task = t2->_ins[ti3] ;

                                        auto iter = ids.find( input_task ) ;
                                        if ( iter == ids.end() )
                                        {
                                            complete = false ;
                                            break ;
                                        }
                                    }

                                    if ( !complete ) continue ;
                                }

                                // if candidate, check cycles
                                {
                                    bool_t has_cycle = false ;
                                    for( size_t ti3=0; ti3<t2->_outs.size(); ++ti3 )
                                    //for ( auto * output_task : t2->_outs )
                                    {
                                        auto * output_task = t2->_outs[ti3] ;

                                        auto iter = ids.find( output_task ) ;
                                        if ( iter != ids.end() )
                                        {
                                            has_cycle = true ;
                                            break ;
                                        }
                                    }

                                    if( has_cycle ) 
                                    {
                                        //motor::log::global_t::status("graph has cycle.") ;
                                        res.has_cylce = true ;
                                        continue ;
                                    }
                                }

                                tiers[cur_tier+1].tasks.push_back( t2 ) ;
                                ids[ t2 ] = id++ ;
                                ++res.num_tasks ;
                            }
                        }

                        ++cur_tier ;
                    }
                }

                using output_slots_funk_t = std::function< void_t ( this_mtr_t, tasks_cref_t outs ) > ;
                static void_t output_slot_walk( build_result_in_t res, output_slots_funk_t f ) noexcept 
                {
                    for( auto const & tiEr : res.tiers )
                    {
                        for( size_t i=0; i<tiEr.tasks.size(); ++i )
                        //for( auto * t : tiEr.tasks )
                        {
                            auto * t = tiEr.tasks[i] ;
                            f( t, t->_outs ) ;
                        }
                    }
                }
            };
            motor_typedef( tier_builder ) ;

        private:

            void_t schedule( motor::vector< this_mtr_t > & tasks ) noexcept
            {
                tasks.reserve( tasks.capacity() + _outs.size() ) ;

                //for ( auto & t : _outs ) tasks.emplace_back( motor::memory::copy_ptr( t ) ) ;
                for ( size_t i=0; i<_outs.size(); ++i ) tasks.emplace_back( motor::memory::copy_ptr( _outs[i] ) ) ;
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

                this_t::set_executing( true ) ;
                this_t::task_funk_param_t p ;
                _funk( p ) ;

                _incomings = _ins.size() ;
                this_t::set_executing( false ) ;

                return true ;
            }

            bool_t resume( void_t ) noexcept
            {
                return this_t::dec_incoming() ;
            }

            void_t move_out_all( this_t::tasks_t & tasks ) noexcept
            {
                tasks.reserve( tasks.capacity() + _ins.size() + _outs.size() )  ;
                for ( size_t i=0; i<_ins.size(); ++i ) tasks.push_back( _ins[i] ) ;
                for ( size_t i=0; i<_outs.size(); ++i ) tasks.push_back( _outs[i] ) ;
                _incomings = 0 ;
                _outs.clear() ;
                _ins.clear() ;
            }
        };
        motor_typedef( task ) ;
    }
}
