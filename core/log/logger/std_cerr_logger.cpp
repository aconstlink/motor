
#include "std_cerr_logger.h"

#include <iostream>

using namespace motor::log ;

//*************************************************************************************
std_cerr_logger::std_cerr_logger( void_t ) noexcept {}

//*************************************************************************************
std_cerr_logger::~std_cerr_logger( void_t ) noexcept {}

//*************************************************************************************
motor::log::result std_cerr_logger::log( motor::log::log_level const ll, motor::core::string_cref_t msg ) noexcept
{
    std::lock_guard< std::mutex > lk(_mtx) ;
    std::cerr << motor::log::to_string_short(ll) << ((ll == motor::log::log_level::raw) ? "" : " ") << msg << std::endl ;

    return motor::log::result::ok ;
}

