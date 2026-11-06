// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#include "global.h"

#include "scheduler.h"
#include "thread_pool.hpp"
#include "task/task.hpp"

#include <motor/memory/global.h>
#include <motor/log/global.h>

using namespace motor::concurrent ;

mutex_t global::_mtx ;
global::singleton_data * global::_dptr = nullptr ;

//*************************************************************************************************
struct global::singleton_data
{
    motor::concurrent::thread_pool_t tp ;
    motor::concurrent::loose_thread_scheduler_t lts ;
    //motor::memory::arena< motor::concurrent::task_t > arena = 
        //motor::memory::arena< motor::concurrent::task_t >(50000) ;

    singleton_data( void_t ) noexcept{}

    singleton_data( singleton_data && rhv ) noexcept : tp( std::move( rhv.tp ) ),
        lts ( std::move( rhv.lts ) ), notifies( std::move( rhv.notifies) )
    {
        //tp = std::move( rhv.tp ) ;
        //lts = std::move( rhv.lts ) ;
        //arena = std::move( rhv.arena ) ;
    }

    ~singleton_data( void_t ) noexcept{
    }

    std::mutex notifies_mtx ;
    motor::vector< global::notify_funk_t > notifies ;
    motor::vector< size_t > lts_notifies ;
};

//*************************************************************************************************
size_t global::register_work_event_notify( notify_funk_t f ) noexcept 
{
    std::lock_guard< std::mutex > lk( this_t::init()->notifies_mtx ) ;
    _dptr->notifies.emplace_back( f ) ;
    _dptr->lts_notifies.emplace_back( _dptr->lts.register_work_notify( f ) ) ;
    return _dptr->notifies.size() - 1 ;
}

//*************************************************************************************************
void_t global::unregister_work_event_notify( size_t const id ) noexcept 
{
    std::lock_guard< std::mutex > lk( this_t::init()->notifies_mtx ) ;
    if( _dptr->notifies.size() <= id ) return ;
    _dptr->lts.unregister_work_notify( _dptr->lts_notifies[id] ) ;
    _dptr->lts_notifies[id] = size_t(-1) ;
    _dptr->notifies[id] = nullptr ;
}

//*************************************************************************************************
global::singleton_data * global::init( void_t ) noexcept
{
    motor::concurrent::lock_guard_t lk( this_t::_mtx ) ;
    if( this_t::_dptr != nullptr ) return this_t::_dptr ;

    _dptr = motor::memory::global_t::alloc( this_t::singleton_data(),
        "[motor::concurrent::global::init] : global singleton lazy initialization" ) ;
    _dptr->tp.init() ;
    _dptr->lts.init() ;

    motor::log::global_t::status( "[online] : motor concurrent" ) ;

    return this_t::_dptr ;
}

//*************************************************************************************************
void_t global::deinit( void_t ) noexcept
{
    motor::concurrent::lock_guard_t lk( this_t::_mtx ) ;
    if( this_t::_dptr == nullptr ) return ;

    motor::memory::global_t::dealloc( _dptr ) ;
    _dptr = nullptr ;
}

//*************************************************************************************************
void_t global::update( void_t ) noexcept
{
    this_t::init()->lts.update() ;
}

//*************************************************************************************************
void_t global::yield( std::function< bool_t ( void_t ) > funk ) noexcept 
{
    if( !_dptr->tp.yield( funk ) ) _dptr->lts.yield( funk ) ;
}

//*************************************************************************************************
void_t global::schedule( motor::concurrent::task_mtr_safe_t t, motor::concurrent::schedule_type const st ) noexcept 
{
    if( st == motor::concurrent::schedule_type::pool )
    {
        this_t::init()->tp.schedule( std::move( t ) ) ;
    }
    else if( st == motor::concurrent::schedule_type::loose )
    {
        this_t::init()->lts.schedule( std::move( t ) ) ;
    }

    {
        std::lock_guard< std::mutex > lk( _dptr->notifies_mtx ) ;
        for( auto & n : _dptr->notifies )
        {
            if( n ) n() ;
        }
    }
}

//*************************************************************************************************
task_mtr_safe_t global::make_task( motor::concurrent::task_t::task_funk_t f ) noexcept
{
    return motor::unique( motor::memory::global_t::create< task_t >( motor::concurrent::task_t(f), 
        "[motor::concurrent::global::make_task] : task" ) ) ;
}