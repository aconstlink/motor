
#include "render_engine.h"

using namespace motor::graphics ;

render_engine::render_engine( void_t ) noexcept {}
render_engine::render_engine( this_rref_t rhv ) noexcept
{
    _has_commands = rhv._has_commands ;
    _coms_up = std::move( rhv._coms_up ) ;
    _coms_ex = std::move( rhv._coms_ex ) ;
}
render_engine::~render_engine( void_t ) noexcept {}

bool_t render_engine::can_execute( void_t ) noexcept 
{
    return this_t::has_frame_commands() ;
}

bool_t render_engine::execute_frame( void_t ) noexcept 
{
    assert( _has_commands == true ) ;

    // 1. execute upstream commands
    for( auto & c : _coms_up )
    {
        c() ;
    }
    _coms_up.clear() ;    

    // 2. execute "execute" commands
    for( auto & c : _coms_ex )
    {
        c() ;
    }
    _coms_ex.clear() ;

    // 3. swap when upstream commands are processed
    // At this point, the outer entity can write in the back buffer again
    this_t::swap_has_commands() ;

    return true ;
}

bool_t render_engine::can_enter_frame( void_t ) const noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    return !_has_commands ;
}

bool_t render_engine::enter_frame( void_t ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    return !_has_commands ;
}

void_t render_engine::leave_frame( void_t ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    _has_commands = true ;
}

void_t render_engine::send_upstream( this_t::command_t c ) noexcept
{
    _coms_up.emplace_back( std::move( c ) ) ;
}

void_t render_engine::send_execute( this_t::command_t c ) noexcept
{
    _coms_ex.emplace_back( std::move( c ) ) ;
}
            
void_t render_engine::swap_has_commands( void_t ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    _has_commands = false ;
}

bool_t render_engine::has_frame_commands( void_t ) const noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    return _has_commands ;
}