
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

    singleton_data( singleton_data && rhv ) noexcept
    {
        tp = std::move( rhv.tp ) ;
        lts = std::move( rhv.lts ) ;
        //arena = std::move( rhv.arena ) ;
    }

    ~singleton_data( void_t ) noexcept
    {}
};

//*************************************************************************************************
global::singleton_data * global::init( void_t ) noexcept
{
    motor::concurrent::lock_guard_t lk( this_t::_mtx ) ;
    if( this_t::_dptr != nullptr ) return this_t::_dptr ;

    _dptr = motor::memory::global_t::alloc( this_t::singleton_data(),
        "[motor::concurrent::global::init] : global singleton lazy initialization" ) ;
    _dptr->tp.init() ;
    _dptr->lts.init() ;

    motor::log::global_t::status( "[online] : natus concurrent" ) ;

    return this_t::_dptr ;
}

//*************************************************************************************************
void_t global::deinit( void_t ) noexcept
{
    motor::concurrent::lock_guard_t lk( this_t::_mtx ) ;
    if( this_t::_dptr == nullptr ) return ;

    motor::memory::global_t::dealloc( _dptr ) ;
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
void_t global::schedule( motor::concurrent::task_mtr_rref_t t, motor::concurrent::schedule_type const st ) noexcept 
{
    if( st == motor::concurrent::schedule_type::pool )
    {
        this_t::init()->tp.schedule( std::move( t ) ) ;
    }
    else if( st == motor::concurrent::schedule_type::loose )
    {
        this_t::init()->lts.schedule( std::move( t ) ) ;
    }
}

//*************************************************************************************************
task_mtr_moved_t global::make_task( motor::concurrent::task_t::task_funk_t f ) noexcept
{
    return motor::memory::global_t::create< task_t >( motor::concurrent::task_t(f), 
        "[motor::concurrent::global::make_task] : task" ) ;
}