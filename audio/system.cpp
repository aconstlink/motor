
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
    _fptr = motor::move( rhv._fptr ) ;
    _run = rhv._run ; rhv._run = false ;
}

//********************************************************************
system::~system( void_t ) noexcept 
{
    this_t::shutdown_system( true ) ;
    motor::memory::release_ptr( motor::move( _bptr ) ) ;
    motor::memory::global_t::dealloc( _fptr ) ;
}

//********************************************************************
bool_t system::on_audio( std::function< void_t ( motor::audio::frontend_ptr_t fptr ) > funk ) noexcept
{
    if( _bptr == nullptr || !_run ) return false ;

    if( _come.enter_frame() )
    {
        funk( _fptr ) ;
        _come.leave_frame() ;
        return true ;
    }
    return false ;
}

//********************************************************************
void_t system::start_system( motor::audio::backend_mtr_safe_t bptr ) noexcept 
{
    if( _run ) return ;
    
    _bptr = bptr ;
    _thr = std::thread( [&]( void_t )
    {
        motor::log::global_t::status("[audio::system] : started audio thread") ;

        _fptr = motor::memory::global_t::alloc( motor::audio::frontend_t( &_come, _bptr ),
            "[audio::system] : frontend" ) ;

        _run = _bptr->init() ;
        while( _run ) 
        {
            std::this_thread::sleep_for( std::chrono::milliseconds(1) ) ;
            
            _bptr->begin() ;
            if( _come.can_execute() )
            {
                _come.execute_frame() ;
            }
            _bptr->end() ;
        }
        _bptr->release() ;

        motor::log::global_t::status("[audio::system] : shutdown audio thread") ;

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