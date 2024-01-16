
#include "window.h"

using namespace motor::application ;

//***************************************************************************
window::window( void_t ) noexcept {}

//***************************************************************************
window::window( this_rref_t rhv ) noexcept
{
    _ins = std::move( rhv._ins ) ;
    _outs = std::move( rhv._outs ) ;
    _re = motor::move( rhv._re ) ;
    _fe = motor::move( rhv._fe ) ;
}

//***************************************************************************
window::~window( void_t ) noexcept 
{
    for( auto * mtr : _ins ) motor::memory::release_ptr( mtr ) ;
    for( auto * mtr : _outs ) motor::memory::release_ptr( mtr ) ;
}

//***************************************************************************
void_t window::register_in( motor::application::iwindow_message_listener_mtr_shared_t l ) noexcept
{
    _ins.push_back( motor::memory::copy_ptr( l ) ) ;
}

//***************************************************************************
void_t window::register_in( motor::application::iwindow_message_listener_mtr_unique_t l ) noexcept
{
    _ins.push_back( l ) ;
}

//***************************************************************************
void_t window::unregister_in( motor::application::iwindow_message_listener_mtr_t l ) noexcept
{
    auto iter = std::find_if( _ins.begin(), _ins.end(), [&]( motor::application::iwindow_message_listener_mtr_t ls )
    {
        return ls == l ;
    } ) ;

    if( iter == _ins.end() )
        return ;

    motor::memory::release_ptr( *iter ) ;
    _ins.erase( iter ) ;
}

//***************************************************************************
void_t window::foreach_in( foreach_listener_funk_t funk ) noexcept
{
    for( auto & l : _ins )
    {
        funk( l ) ;
    }
}

//***************************************************************************
void_t window::register_out( motor::application::iwindow_message_listener_mtr_shared_t l ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_outs ) ;
    _outs.push_back( motor::memory::copy_ptr( l ) ) ;
}

//***************************************************************************
void_t window::register_out( motor::application::iwindow_message_listener_mtr_unique_t l ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_outs ) ;
    _outs.push_back( l ) ;
}

//***************************************************************************
void_t window::unregister_out( motor::application::iwindow_message_listener_mtr_t l ) noexcept
{
    auto iter = std::find_if( _outs.begin(), _outs.end(), [&] ( motor::application::iwindow_message_listener_mtr_t ls )
    {
        return ls == l ;
    } ) ;

    if( iter == _outs.end() )
        return ;

    motor::memory::release_ptr( *iter ) ;
    _outs.erase( iter ) ;
}

//***************************************************************************
void_t window::foreach_out( foreach_out_funk_t funk ) noexcept
{
    std::lock_guard< std::mutex > lk( _mtx_outs ) ;
    for( auto& l : _outs )
    {
        funk( l ) ;
    }
}

//***************************************************************************
void_t window::send_message( motor::application::show_message_cref_t msg ) noexcept 
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::resize_message_cref_t msg ) noexcept 
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::create_message_cref_t msg ) noexcept
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::close_message_cref_t msg ) noexcept
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::screen_dpi_message_cref_t msg ) noexcept
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::screen_size_message_cref_t msg ) noexcept
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::vsync_message_cref_t msg ) noexcept
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::fullscreen_message_cref_t msg ) noexcept
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
void_t window::send_message( motor::application::cursor_message_cref_t msg ) noexcept 
{
    this_t::foreach_in( [&]( motor::application::iwindow_message_listener_mtr_t lsn )
    {
        lsn->on_message( msg ) ;
    } ) ;
}

//***************************************************************************
bool_t window::render_frame_virt( motor::application::iwindow_t::render_frame_funk_t funk ) noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx_rnd ) ;

    if( _re == nullptr ) return false ;

    if( _re->enter_frame() )
    {
        funk( _fe ) ;
        _re->leave_frame() ;
        return true ;
    }
    return false ;
}

//***************************************************************************
void_t window::set_renderable( motor::graphics::render_engine_ptr_t re, motor::graphics::ifrontend_ptr_t fe ) noexcept 
{
    std::lock_guard< std::mutex > lk( _mtx_rnd ) ;
    _re = re ;_fe = fe ;
}