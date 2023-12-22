
#include "render_engine.h"

using namespace motor::graphics ;

render_engine::render_engine( void_t ) noexcept {}
render_engine::render_engine( this_rref_t rhv ) noexcept
{
    _ridx = rhv._ridx ;
    _has_commands = rhv._has_commands ;
    _coms_up[0] = std::move( rhv._coms_up[0] ) ;
    _coms_up[1] = std::move( rhv._coms_up[1] ) ;
    _coms_ex[0] = std::move( rhv._coms_ex[0] ) ;
    _coms_ex[1] = std::move( rhv._coms_ex[1] ) ;
}
render_engine::~render_engine( void_t ) noexcept {}

bool_t render_engine::can_execute( void_t ) noexcept 
{
    if( !this_t::has_frame_commands() ) return false ;
    return true ;
}

bool_t render_engine::execute_frame( void_t ) noexcept 
{
    size_t const ridx = this_t::swap_read_idx() ;

    // 1. execute upstream commands
    for( auto & c : _coms_up[ridx] )
    {
        c() ;
    }
    _coms_up[ridx].clear() ;

    // 2. swap when upstream commands are processed
    // At this point, the outer entity can write in the back buffer again
    if( !this_t::swap_has_commands() )
    {
        motor::log::global_t::critical( "race condition not in sync" ) ;
    }

    // 3. execute "execute" commands
    for( auto & c : _coms_ex[ridx] )
    {
        c() ;
    }
    _coms_ex[ridx].clear() ;

    return true ;
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
    _coms_up[this_t::widx()].emplace_back( std::move( c ) ) ;
}

void_t render_engine::send_execute( this_t::command_t c ) noexcept
{
    _coms_ex[this_t::widx()].emplace_back( std::move( c ) ) ;
}

size_t render_engine::widx( void_t ) const noexcept
{
    return (_ridx+1)%2 ;
}

size_t render_engine::swap_read_idx( void_t ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_ridx ) ;
    _ridx = ++_ridx % 2 ;
    return _ridx ;
}
            
bool_t render_engine::swap_has_commands( void_t ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    if( _has_commands )
    {
        _has_commands = false ;
        return true ;
    }
    return false ;
}

bool_t render_engine::has_frame_commands( void_t ) const noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx_has_commands ) ;
    return _has_commands ;
}