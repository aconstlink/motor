
#include "carrier.h"
//#include "../app.h"

#include <motor/profiling/global.h>
#include <motor/scene/global.h>
#include <motor/concurrent/global.h>
#include <motor/memory/global.h>
#include <motor/log/global.h>

using namespace motor ;
using namespace motor::application ;

std::chrono::microseconds carrier::sleep_for_micro = std::chrono::microseconds(0) ;

//******************************************************
std::chrono::microseconds carrier::set_cpu_sleep( std::chrono::microseconds const micro ) noexcept 
{
    static std::mutex mtx ;
    std::lock_guard< std::mutex > lk( mtx ) ;

    carrier::sleep_for_micro = std::min( micro, std::chrono::microseconds(30) ) ;

    return carrier::sleep_for_micro ;
}

//******************************************************
std::chrono::microseconds carrier::get_cpu_sleep( void_t ) noexcept 
{
    return carrier::sleep_for_micro ;
}

//******************************************************
carrier::carrier( void_t ) noexcept
{
    _sd = motor::memory::global::alloc< this_t::shared_data >("shared data") ;
    _dev_system = motor::memory::global_t::alloc( motor::controls::system_t(), 
        "[carrier] : device system" ) ;
    _audio_system = motor::memory::global_t::alloc( motor::audio::system_t(), 
        "[carrier] : audio system" ) ;
    _network_system = motor::memory::global_t::alloc( motor::network::system_t(),
        "[carrier] : network system" ) ;
}

//******************************************************
carrier::carrier( this_rref_t rhv ) noexcept
{
    _thr = std::move( rhv._thr ) ;
    _sd = motor::move( rhv._sd ) ;
    assert( _sd->update_running == false ) ;
    _app = motor::move( rhv._app ) ;

    _dev_system = motor::move( rhv._dev_system ) ;
    _audio_system = motor::move( rhv._audio_system ) ;
    _network_system = motor::move( rhv._network_system ) ;
}

//******************************************************
carrier::carrier( motor::application::app_mtr_safe_t app ) noexcept : this_t()
{
    this_t::set( motor::move( app ) ) ;
}

//******************************************************
carrier::~carrier( void_t ) noexcept
{
    motor::memory::global_t::dealloc( _sd ) ;
    motor::memory::global_t::dealloc( _dev_system ) ;
    motor::memory::global_t::dealloc( _audio_system ) ;
    motor::memory::global_t::dealloc( _network_system ) ;

    motor::memory::release_ptr( _app ) ;
}

//******************************************************
motor::application::result carrier::set( motor::application::app_mtr_safe_t app ) noexcept
{
    if( _app != nullptr )
        return motor::application::result::failed ;

    _app = app ;

    return motor::application::result::ok ;
}

//******************************************************
motor::application::result carrier::start_update_thread( void_t ) noexcept 
{
    if( _app == nullptr )
    {
        motor::log::global_t::error( motor_log_fn("No app object") ) ;
        return motor::application::result::no_app ;
    }

    _sd->update_running = true ;
    
    _thr = std::thread( [=]( void_t )
    {
        motor::application::app_t::carrier_accessor ca( _app ) ;
        ca.init( this ) ;

        while( _sd->update_running )
        {
            {
                auto const sleep_micro = motor::application::carrier::get_cpu_sleep() ;
                if( sleep_micro > std::chrono::microseconds(0) ) 
                    std::this_thread::sleep_for( sleep_micro ) ;
            }
            

            ca.update() ;

            // check user closed app
            if( ca.has_closed() )
            {
                _sd->update_running = false ;
            }
        }
        
        while( !ca.shutdown() ) ;
        
        this->close() ;
    } ) ;

    return motor::application::result::ok ;
}

//******************************************************
void_t carrier::stop_update_thread( void_t ) noexcept
{
    if( !_thr.joinable() ) return ;

    _sd->update_running = false ;
    _thr.join() ;
}

//******************************************************
int_t carrier::exec( void_t ) noexcept
{
    motor::scene::global::init() ;

    {
        auto const res = this_t::start_update_thread() ;
        if( motor::application::no_success( res ) )
        {
            motor::log::global_t::warning( motor_log_fn(
                "No update thread started. Proceed anyway.") ) ;
        }
    }

    // virtual call
    {
        this->on_exec() ;
    }

    this_t::stop_update_thread() ;

    motor::scene::global::deinit() ;
    motor::profiling::global::deinit() ;
    motor::concurrent::global::deinit() ;
    motor::log::global::deinit() ;

    return 0 ;
}

//******************************************************
motor::controls::system_ptr_t carrier::get_dev_system( void_t ) noexcept 
{
    return _dev_system ;
}

//******************************************************
motor::audio::system_ptr_t carrier::get_audio_system( void_t ) noexcept 
{
    return _audio_system ;
}

//******************************************************
motor::network::system_ptr_t carrier::get_network_system( void_t ) noexcept 
{
    return _network_system ;
}

//******************************************************
void_t carrier::update_device_system( void_t ) noexcept 
{
    _dev_system->update() ;
}

//******************************************************
motor::controls::system_mtr_t carrier::device_system( void_t ) noexcept 
{
    return _dev_system ;
}

//******************************************************
motor::audio::system_ptr_t carrier::audio_system( void_t ) noexcept 
{
    return _audio_system ;
}

//******************************************************
motor::network::system_ptr_t carrier::network_system( void_t ) noexcept 
{
    return _network_system ;
}