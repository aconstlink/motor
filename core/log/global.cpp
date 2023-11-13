//------------------------------------------------------------
// natus (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#include "global.h"

#include "system/system.h"

using namespace motor::log ;

std::mutex global::_mtx ;
global::this_ptr_t global::_ptr = nullptr ;

//*************************************************************************************
global::global( void_t ) noexcept
{
    __default_log_system = motor::log::system_t::create() ;
}

//*************************************************************************************
global::global( this_rref_t rhv ) noexcept
{
    motor_move_member_ptr( __default_log_system, rhv ) ;
}

//*************************************************************************************
global::~global( void_t ) noexcept
{
    motor::log::system::destroy( __default_log_system ) ;
}

//*************************************************************************************
global::this_ptr_t global::init( void_t ) noexcept
{
    if( global_t::_ptr != nullptr ) return this_t::_ptr ;

    {
        std::lock_guard<::std::mutex> lk( this_t::_mtx ) ;

        if( global_t::_ptr != nullptr ) return this_t::_ptr ;
        
        this_t::_ptr = new this_t() ;

        this_t::status( "[online] : natus log" ) ;
    }

    return this_t::_ptr ;
}

//*************************************************************************************
void_t global::deinit( void_t ) noexcept
{
    if( global_t::_ptr == nullptr ) return ;

    delete global_t::_ptr ;
    this_t::_ptr = nullptr ;
}

//*************************************************************************************
global::this_ptr_t global::get( void_t ) noexcept
{
    return this_t::init() ;
}

//*************************************************************************************
void_t global::add_logger( motor::log::ilogger_ptr_t ptr ) noexcept
{
    this_t::get()->__default_log_system->add_logger( ptr ) ;
}

//*************************************************************************************
motor::log::store_logger_cptr_t global::get_store( void_t ) noexcept
{
    return this_t::get()->__default_log_system->get_store() ;
}

//*************************************************************************************
void_t global::message( log_level const level, motor::core::string_cref_t msg ) noexcept
{
    this_t::get()->__default_log_system->log( level, msg ) ;
}

//*************************************************************************************
bool_t global::message( bool_t const condition, log_level const level, motor::core::string_cref_t msg ) noexcept
{
    if( condition ) motor::log::global::message( level, msg ) ;
    return condition ;
}

//*************************************************************************************
void_t global::status( motor::core::string_cref_t msg ) noexcept
{
    motor::log::global::message( motor::log::log_level::status, msg ) ;
}

//*************************************************************************************
bool_t global::status( bool_t const condition, motor::core::string_cref_t msg ) noexcept
{
    return motor::log::global::message( condition, motor::log::log_level::status, msg ) ;
}

//*************************************************************************************
void_t global::warning( motor::core::string_cref_t msg ) noexcept
{
    motor::log::global::message( motor::log::log_level::warning, msg ) ;
}

//*************************************************************************************
bool_t global::warning( bool_t const condition, motor::core::string_cref_t msg ) noexcept
{
    return motor::log::global::message( condition, motor::log::log_level::warning, msg ) ;
}

//*************************************************************************************
void_t global::error( motor::core::string_cref_t msg ) noexcept
{
    motor::log::global::message( motor::log::log_level::error, msg ) ;
}

//*************************************************************************************
void_t global::error_and_exit( motor::core::string_cref_t msg ) noexcept
{
    motor::log::global::message( motor::log::log_level::error, msg ) ;
    std::exit( 1 ) ; 
}

//*************************************************************************************
void_t global::error_and_exit( bool_t const condition, motor::core::string_cref_t msg ) noexcept
{
    if( motor::log::global::message( condition, motor::log::log_level::error, msg ) )
        std::exit( 1 ) ;
}

//*************************************************************************************
bool_t global::error( bool_t const condition, motor::core::string_cref_t msg ) noexcept
{
    return motor::log::global::message( condition, motor::log::log_level::error, msg ) ;
}

//*************************************************************************************
void_t global::critical( motor::core::string_cref_t msg ) noexcept
{
    motor::log::global::message( motor::log::log_level::critical, msg ) ;
}

//*************************************************************************************
bool_t global::critical( bool_t const condition, motor::core::string_cref_t msg ) noexcept
{
    return motor::log::global::message( condition, motor::log::log_level::critical, msg ) ;
}
