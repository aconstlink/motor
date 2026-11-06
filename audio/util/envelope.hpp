// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT
#pragma once

#include "../typedefs.h"

namespace motor::audio
{

using namespace motor::core::types ;

class envelope
{
    motor_this_typedefs( envelope );

  public:

    struct init_data
    {
        float_t attack_sec = 0.01f;
        float_t release_sec = 0.02f;
    };

  private:

    float_t _attack_sec;
    float_t _release_sec;

    float_t _value = 0.0f;

  private:

  public:

    envelope( void_t ) noexcept : envelope( init_data{} ) {}
    envelope( this_t::init_data const & d ) noexcept
        : _attack_sec( std::abs( d.attack_sec ) ), _release_sec( std::abs( d.release_sec ) )
    {
        assert( _attack_sec >= 0.0f );
        assert( _release_sec >= 0.0f );
    }

    envelope( this_cref_t ) noexcept = default;
    envelope( this_rref_t ) noexcept = default;

    ~envelope( void_t ) {}

  public:

    float_t update( float_t const level, float_t const dt ) noexcept
    {
        if( dt <= 0.0f ) return _value;

        float_t const tau = level > _value ? _attack_sec : _release_sec;
        float_t const alpha = 1.0f - std::exp( -dt / tau );
        _value += alpha * ( level - _value );

        return _value;
    }

    float_t value( void_t ) const noexcept
    {
        return _value;
    }

    init_data parameters( void_t ) const noexcept
    {
        return { _attack_sec, _release_sec };
    }

    void_t set_parameters( init_data const & d ) noexcept
    {
        assert( std::isfinite( d.attack_sec ) && d.attack_sec > 0.0f );
        assert( std::isfinite( d.release_sec ) && d.release_sec > 0.0f );
        _attack_sec = d.attack_sec;
        _release_sec = d.release_sec;
    }
};
motor_typedef( envelope );
} // namespace motor::audio