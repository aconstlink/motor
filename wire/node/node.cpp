

#include "node.h"

#include <motor/log/global.h>

using namespace motor::wire ;

//*****************************************************
inode::inode( void_t ) noexcept
{
    _task = motor::shared( motor::concurrent::task_t( this_t::make_task_funk() ),
        "wire node task" ) ;
}

//*****************************************************
inode::inode( motor::string_in_t n) noexcept : _name( n ) 
{
    _task = motor::shared( motor::concurrent::task_t( this_t::make_task_funk() ),
        "wire node task" ) ;
}

//*****************************************************
inode::inode( this_rref_t rhv ) noexcept : _incoming( std::move( rhv._incoming ) ), 
    _outgoing( std::move( rhv._outgoing ) ), _task( motor::move( rhv._task ) ), 
    _name( std::move(rhv._name) ), _inputs( std::move( rhv._inputs) ), 
    _outputs( std::move( rhv._outputs ) )
{
    _task->set_funk( this_t::make_task_funk() ) ;
}

//*****************************************************
inode::~inode( void_t ) noexcept 
{
    #if 0
    if( _task != nullptr )
    {
        assert( !_task->is_executing() && 
            "The task is still in the loop. So releasing the node needs to wait."
            "The solution right now is to wait for all taks to be full executed in the scheduler."
            "Second solution would be to insert a sync_object in the task so other instances could wait for it." ) ;

        motor::log::global_t::critical( "[inode::~inode] : task did not finished executing." ) ;

        // _task->wait_for_sync() ;
    }
    #elif 1
    // the task could still be in the loop. So before deleting the node,
    // be sure to wait for the task which has a captured this pointer.
    // @see make_task_funk
    if( _task != nullptr ) 
        _task->wait_until_executed() ;
    #endif 

    motor::memory::release_ptr( _task ) ;
    for ( auto ptr : _incoming ) motor::memory::release_ptr( ptr ) ;
    for ( auto ptr : _outgoing ) motor::memory::release_ptr( ptr ) ;
}

//*****************************************************
inode::this_mtr_t inode::then( this_mtr_safe_t other ) noexcept
{
    if( this_t::add_outgoing( other ) )
    {
        other->add_incoming( this ) ;
        _task->then( motor::share( other->task() ) ) ;
    }

    motor::release( other ) ;

    return other ;
}

//*****************************************************
motor::concurrent::task_mtr_safe_t inode::get_task( void_t ) noexcept
{
    return motor::share( _task ) ;
}

//*****************************************************
void_t inode::disconnect( void_t ) noexcept
{
    // disconnect task
    {
        _task->disconnect() ;
    }

    // handle incoming
    // need to remove this from their outgoing
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_in ) ;
        for ( auto * ptr : _incoming ) 
        {
            ptr->remove_outgoing( this ) ;
            motor::memory::release_ptr( ptr ) ;
        }
        _incoming.clear() ;
    }

    // handle outgoing
    // need to remove this from their incoming
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_out ) ;
        for ( auto * ptr : _outgoing )
        {
            ptr->remove_incoming( this ) ;
            motor::memory::release_ptr( ptr ) ;
        }
        _outgoing.clear() ;
    }
}

//*****************************************************
motor::string_cref_t inode::name( void_t ) const noexcept 
{
    return _name ;
}

//*****************************************************
motor::wire::inputs_ref_t inode::inputs( void_t ) noexcept 
{
    return _inputs ;
}

//*****************************************************
motor::wire::outputs_ref_t inode::outputs( void_t ) noexcept 
{
    return _outputs ;
}

//*****************************************************
void_t inode::add_incoming( this_ptr_t other ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_in ) ;
    auto iter = std::find_if( _incoming.begin(), _incoming.end(), 
        [&] ( this_ptr_t r ) { return r == other ; } ) ;

    if ( iter != _incoming.end() ) return ;

    _incoming.push_back( motor::share( other ) ) ;
}

//*****************************************************
void_t inode::remove_incoming( this_ptr_t other ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_in ) ;
    auto iter = std::find_if( _incoming.begin(), _incoming.end(),
        [&] ( this_ptr_t r ) { return r == other ; } ) ;

    if ( iter == _incoming.end() ) return ;

    motor::memory::release_ptr( *iter ) ;
    _incoming.erase( iter ) ;
}

//*****************************************************
bool_t inode::add_outgoing( this_ptr_t other ) noexcept
{
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_in ) ;
        auto iter = std::find_if( _outgoing.begin(), _outgoing.end(), 
            [&] ( this_ptr_t r ) { return r == other ; } ) ;

        if ( iter != _outgoing.end() ) return false ;

        _outgoing.push_back( motor::share( other ) ) ;
    }

    return true ;
}

//*****************************************************
void_t inode::remove_outgoing( this_ptr_t other ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_out ) ;
    auto iter = std::find_if( _outgoing.begin(), _outgoing.end(),
        [&] ( this_ptr_t r ) { return r == other ; } ) ;

    if ( iter == _outgoing.end() ) return ;

    motor::memory::release_ptr( *iter ) ;
    _outgoing.erase( iter ) ;
}

//*****************************************************
motor::concurrent::task_ptr_t inode::task( void_t ) noexcept
{
    return _task ;
}

//*****************************************************
motor::concurrent::task_t::task_funk_t inode::make_task_funk( void_t ) noexcept
{
    return [=] ( motor::concurrent::task_t::task_funk_param_in_t ) 
    {
        // exchange all inputs... (pull)
        //this->inputs().exchange() ;
        this->execute() ;
        // or exchange all outputs (push)
        this->outputs().exchange() ;
    } ;
}

//*****************************************************
void_t node::execute( void_t ) noexcept 
{
    _funk( this ) ;
}