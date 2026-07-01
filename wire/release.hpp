#pragma once

#include "typedefs.h"

namespace motor
{
namespace wire
{
#if 1
template < typename T >
void_t release( motor::core::mtr_safe< T > & s ) noexcept
{
    if( s != nullptr ) s->disconnect();
    motor::release( motor::move( s ) );
}

template < typename T >
void_t release( motor::core::mtr_safe< T > && s ) noexcept
{
    if( s != nullptr ) s->disconnect();
    motor::release( motor::move( s ) );
}
#else
static void_t release( motor::core::mtr_safe< islot > & s ) noexcept
{
    if( s != nullptr ) s->disconnect();
    motor::release( motor::move( s ) );
}

static void_t release( motor::core::mtr_safe< islot > && s ) noexcept
{
    if( s != nullptr ) s->disconnect();
    motor::release( motor::move( s ) );
}
#endif
} // namespace wire
} // namespace motor