
#include "system.h"

using namespace motor::audio ;


//********************************************************************
system::system( void_t ) noexcept
{
}

//********************************************************************
system::system( this_rref_t rhv ) noexcept 
{
    _bptr = motor::move( rhv._bptr ) ;
    _come = std::move( rhv._come ) ;
    _thr = std::move( rhv._thr ) ;
    _run = rhv._run ; rhv._run = false ;
}

//********************************************************************
system::~system( void_t ) noexcept 
{
    this_t::shutdown_system( true ) ;
}

//********************************************************************
bool_t system::on_audio( std::function< void_t ( motor::audio::frontend_ptr_t fptr ) > funk ) noexcept
{
    if( _bptr == nullptr || !_run ) return false ;

    motor::audio::frontend_t fe( &_come, _bptr )  ;

    if( _come.enter_frame() )
    {
        funk( &fe ) ;
        _come.leave_frame() ;
        return true ;
    }
    return false ;
}

//********************************************************************
void_t system::start_system( motor::audio::backend_ptr_t bptr ) noexcept 
{
    if( _run ) return ;

    _bptr = bptr ;
    _thr = std::thread( [&]( void_t )
    {
        _run = true ;
        while( _run ) 
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(1) ) ;

            if( _come.can_execute() )
            {
                _come.execute_frame() ;
            }
        }

    } ) ;
}

//********************************************************************
void_t system::shutdown_system( bool_t const wait_for_it ) noexcept 
{
    if( !_run ) return ;

    _run = false ;
    if( wait_for_it && _thr.joinable() ) 
    {
        _thr.join() ;
    }
}