#pragma once

#include <motor/std/string>
#include <motor/std/vector>
#include <motor/memory/global.h>
#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>

namespace motor
{
namespace application
{
using namespace motor::core::types;

enum class network_client_type
{
    udp,
    tcp
};

using window_id_t = size_t;

static window_id_t invalid_window_id( void_t ) noexcept
{
    return window_id_t( -1 );
}
} // namespace application
} // namespace motor