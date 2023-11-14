
#include "system.h"
#include "../logger/std_cerr_logger.h"

#include <algorithm>

using namespace motor::log ;

//*************************************************************************************
system::system( void_t ) noexcept : _loggers( this_t::loggers_t::allocator_type(
    "[log::system::system] : log system loggers")) 
{
    _this_default_logger = new std_cerr_logger( ) ;
    _default_logger = _this_default_logger ;
    this_t::add_logger( _default_logger ) ;
    this_t::add_logger( &_slogger ) ;
}

//*************************************************************************************
system::system( this_rref_t rhv ) noexcept
{
    motor_move_member_ptr( _this_default_logger, rhv ) ;
    motor_move_member_ptr( _default_logger, rhv ) ;
    _loggers = std::move( rhv._loggers ) ;
    _slogger = std::move( rhv._slogger ) ;
}

//*************************************************************************************
system::~system( void_t ) noexcept
{
    delete _this_default_logger ;
}

//*************************************************************************************
system::this_ptr_t system::create( void_t ) noexcept
{
    /// do not use natus memory
    /// at the moment, natus memory is above log
    return new this_t() ;
}

//*************************************************************************************
void_t system::destroy( this_ptr_t ptr ) noexcept
{
    delete ptr ;
}

//*************************************************************************************
motor::log::result system::log( motor::log::log_level const ll, motor::string_cref_t msg ) noexcept
{
    for( auto * logger : _loggers ) logger->log( ll, msg ) ;
    return motor::log::result::ok ;
}

//*************************************************************************************
motor::log::result system::add_logger( motor::log::ilogger_ptr_t lptr ) noexcept
{
    if( lptr == nullptr )
    {
        return motor::log::result::invalid_argument ;
    }

    auto iter = std::find( _loggers.begin(), _loggers.end(), lptr ) ;
    if( iter != _loggers.end() )
    {
        return motor::log::result::ok ;
    }

    _loggers.push_back( lptr ) ;

    return lptr->log( motor::log::log_level::status, "Logger online" ) ;
}

//*************************************************************************************
motor::log::ilogger_ptr_t system::set_default_logger( motor::log::ilogger_ptr_t lptr ) noexcept
{
    motor::log::ilogger_ptr_t ret_ptr = _default_logger ;
    _default_logger = lptr == nullptr ? _this_default_logger : lptr ;
    return ret_ptr == _this_default_logger ? nullptr : ret_ptr ;
}

//*************************************************************************************
motor::log::store_logger_cptr_t system::get_store( void_t ) const noexcept
{
    return &_slogger ;
}