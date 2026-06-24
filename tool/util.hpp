#pragma once

#include "typedefs.h"

#include <motor/math/typedefs.h>

namespace motor
{
namespace tool
{
//***************************************************************
static void_t make_time_string( char * buffer, size_t const sib, motor::math::time_ms_t const milli ) noexcept
{
    uint_t const min = uint_t(milli) / 60000;
    uint_t const sec = ( uint_t(milli) / 1000 ) % 60;
    uint_t const mil = uint_t(milli) % 1000;

    std::snprintf( buffer, sib, "%02d:%02d:%03d", min, sec, mil );
}
} // namespace tool
} // namespace motor