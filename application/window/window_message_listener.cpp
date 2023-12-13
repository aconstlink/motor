
#include "window_message_listener.h"

using namespace motor::application ;


//***************************************************************************
window_message_listener::window_message_listener( void_t ) noexcept {}

//***************************************************************************
window_message_listener::window_message_listener( this_rref_t rhv ) noexcept
{
    _states = std::move( rhv._states ) ;
    _has_any_change = rhv._has_any_change ;
}

//***************************************************************************
window_message_listener::window_message_listener( this_cref_t rhv ) noexcept
{
    _states = rhv._states ;
    _has_any_change = rhv._has_any_change ;
}

//***************************************************************************
window_message_listener::~window_message_listener( void_t ) noexcept {}

//***************************************************************************
bool_t window_message_listener::swap_and_reset( state_vector_out_t states ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    bool_t changed = _has_any_change ;
    if( changed )
    {
        states = _states ;
    }
    this_t::reset_change_flags() ;
    return changed ;
}

//***************************************************************************
bool_t window_message_listener::has_any_change( void_t ) const noexcept { return _has_any_change ; }

//***************************************************************************
void_t window_message_listener::reset_change_flags( void_t ) noexcept
{
    _states.resize_changed = false ;
    _states.show_changed = false ;
    _states.close_changed = false ;
    _states.msize_msg_changed = false ;
    _states.dpi_msg_changed = false ;
    _states.vsync_msg_changed = false ;
    _states.fulls_msg_changed = false ;
    _has_any_change = false ;
}

//***************************************************************************
void_t window_message_listener::on_message( screen_dpi_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.dpi_msg_changed = true ;
    _states.dpi_msg = msg ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( screen_size_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.msize_msg_changed = true ;
    _states.msize_msg = msg ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( resize_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.resize_changed = true ;
    _states.resize_msg.combine( msg ) ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( show_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.show_changed = true ;
    _states.show_msg = msg ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( motor::application::create_message_cref_t msg ) noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.create_changed = true ;
    _states.create_msg = msg ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( close_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.close_changed = true ;
    _states.close_msg = msg ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( vsync_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.vsync_msg_changed = true ;
    _states.vsync_msg = msg ;
    _has_any_change = true ;
}

//***************************************************************************
void_t window_message_listener::on_message( fullscreen_message_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx ) ;
    _states.fulls_msg_changed = true ;
    _states.fulls_msg = msg ;
    _has_any_change = true ;
}