// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT
#pragma once

#include "envelope.hpp"

namespace motor::audio
{
using namespace motor::core::types;

class onset_detector
{
    motor_this_typedefs( onset_detector );

  public:

    struct init_data
    {
        envelope::init_data env_dat = { 0.0016f, 0.0748f };

        float_t reference_sec = 0.0087f;
        float_t minimum_level = 0.0001f;
        float_t trigger_ratio = 1.5f;
        float_t rearm_ratio = 1.1f;
    };

  private:

    motor::audio::envelope _env;
    float_t _reference = 0.0f;
    bool_t _armed = true;

    float_t _reference_sec;
    float_t _minimum_level;
    float_t _trigger_ratio;
    float_t _rearm_ratio;

private:

    float_t _raw = 0.0f ;
    float_t _onset = 0.0f ;

  public:

    onset_detector( void_t ) noexcept : onset_detector( init_data{} ) {}
    onset_detector( init_data const & d ) noexcept
        : _env( d.env_dat ), _reference_sec( d.reference_sec ), _minimum_level( d.minimum_level ),
          _trigger_ratio( d.trigger_ratio ), _rearm_ratio( d.rearm_ratio )
    {
        assert( std::isfinite( _reference_sec ) && _reference_sec > 0.0f );
        assert( std::isfinite( _minimum_level ) && _minimum_level > 0.0f );
        assert( std::isfinite( _trigger_ratio ) );
        assert( std::isfinite( _rearm_ratio ) );
        assert( _rearm_ratio >= 1.0f && _trigger_ratio > _rearm_ratio );
    }

    onset_detector( this_cref_t ) noexcept = default;
    onset_detector( this_rref_t ) noexcept = default;

    ~onset_detector( void_t ) noexcept {}

  public:

    float_t minimum_level( void_t ) const noexcept
    {
        return _minimum_level;
    }

    float_t envelope_value( void_t ) const noexcept
    {
        return _env.value();
    }

    init_data parameters( void_t ) const noexcept
    {
        return { _env.parameters(), _reference_sec, _minimum_level, _trigger_ratio, _rearm_ratio };
    }

    void_t set_parameters( init_data const & d ) noexcept
    {
        assert( std::isfinite( d.reference_sec ) && d.reference_sec > 0.0f );
        assert( std::isfinite( d.minimum_level ) && d.minimum_level > 0.0f );
        assert( std::isfinite( d.trigger_ratio ) && std::isfinite( d.rearm_ratio ) );
        assert( d.rearm_ratio >= 1.0f && d.trigger_ratio > d.rearm_ratio );
        _env.set_parameters( d.env_dat );
        _reference_sec = d.reference_sec;
        _minimum_level = d.minimum_level;
        _trigger_ratio = d.trigger_ratio;
        _rearm_ratio = d.rearm_ratio;
    }

    bool_t update( float_t const level, float_t const dt ) noexcept
    {
        assert( std::isfinite( level ) && level >= 0.0f );
        assert( std::isfinite( dt ) );

        _raw = level ;

        if( dt <= 0.0f )
        {
            return false;
        }

        float_t const fast = _env.update( level, dt );
        bool_t triggered = false;

        // Compare against the reference from the previous update.
        if( _armed )
        {
            if( fast > _minimum_level && fast > _reference * _trigger_ratio )
            {
                triggered = true;
                _armed = false;
            }
        }
        else if( fast <= _minimum_level || fast <= _reference * _rearm_ratio )
        {
            _armed = true;
        }

        float_t const alpha = 1.0f - std::exp( -dt / _reference_sec );

        _reference += alpha * ( fast - _reference );

        if( triggered )
        {
            _onset = 1.0f ;
        }
        _onset *= std::exp( -dt / 0.1f );

        return triggered;
    }

    float_t raw_value( void_t) const noexcept
    {
        return _raw ;
    }

    // the onset will be set if the detector is triggered.
    // the onset will decay over time.
    float_t onset_value( void_t) const noexcept
    {
        return _onset ;
    }
};
motor_typedef( onset_detector ) ;
} // namespace motor::audio