// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "object.hpp"
#include "../enums.h"
#include "../buffer.hpp"

#include <motor/std/vector>
#include <motor/math/vector/vector2.hpp>

#include <algorithm>
#include <limits>

namespace motor
{
namespace audio
{
enum class capture_type
{
    undefined,
    what_u_hear,
    microphone
};

// the capture object holds the captured wave form (audio signal) and
// the generated fft frequency bands.
// The number of new samples per update is determined by the backend system
class capture_object : public motor::audio::object
{
    motor_this_typedefs( capture_object );

  private:

    motor::audio::channels _ch = motor::audio::channels::undefined;

    motor_typedefs( motor::vector< float_t >, floats );
    floats_t _samples;
    floats_t _frequencies;

    // = sample rate / num bins
    // => how many hz per bin
    float_t _bin_width_hz = 0.0f;

    motor::math::vec2f_t _mm = motor::math::vec2f_t(
        std::numeric_limits< float_t >::max(), std::numeric_limits< float_t >::min() );

  public:

    capture_object( void_t ) noexcept {}
    capture_object( this_cref_t ) = delete;
    capture_object( this_rref_t rhv ) noexcept
        : object( std::move( rhv ) ), _samples( std::move( rhv._samples ) ),
          _frequencies( std::move( rhv._frequencies ) ), _bin_width_hz( rhv._bin_width_hz ),
          _mm( rhv._mm ), _ch( rhv._ch )

    {
    }

    this_ref_t operator=( this_rref_t rhv ) noexcept
    {
        object::operator=( std::move( rhv ) );

        _ch = rhv._ch;
        _samples = std::move( rhv._samples );
        _frequencies = std::move( rhv._frequencies );
        _bin_width_hz = rhv._bin_width_hz;

        return *this;
    }

    virtual ~capture_object( void_t ) noexcept {}

  public:

    motor::audio::channels get_channels( void_t ) const noexcept
    {
        return _ch;
    }

    size_t get_num_channels( void_t ) const noexcept
    {
        return motor::audio::to_number( _ch );
    }

    void_t set_channels( motor::audio::channels const channels ) noexcept
    {
        _ch = channels;
    }

    motor::math::vec2f_t minmax( void_t ) const noexcept
    {
        return _mm;
    }

    void_t set_minmax( motor::math::vec2f_cref_t v ) noexcept
    {
        _mm = v;
    }

    size_t num_samples( void_t ) const noexcept
    {
        return _samples.size();
    }

    size_t num_frequencies( void_t ) const noexcept
    {
        return _frequencies.size();
    }

    template < typename T >
    T get_bin_width_hz( void_t ) const noexcept
    {
        if constexpr( std::is_integral< T >::value )
            return static_cast< T >( std::floorf( _bin_width_hz ) );

        return static_cast< T >( _bin_width_hz );
    }

    void_t set_bin_width_hz( float_t const bw ) noexcept
    {
        _bin_width_hz = bw;
    }

  public:

    typedef std::function< void_t( size_t const, float_t const ) > for_each_funk_t;
    void_t for_each_sample( for_each_funk_t funk ) const noexcept
    {
        for( size_t i = 0; i < _samples.size(); ++i )
        {
            funk( i, _samples[ i ] );
        }
    }

    void_t for_each_frequency( for_each_funk_t funk ) const noexcept
    {
        for( size_t i = 0; i < _frequencies.size(); ++i )
        {
            funk( i, _samples[ i ] );
        }
    }

    void_t copy_samples_from( floats_cref_t samples ) noexcept
    {
        _samples = samples;
    }

    void_t copy_frequencies_from( floats_cref_t frequencies ) noexcept
    {
        _frequencies = frequencies;
    }

    void_t append_samples_to( motor::audio::buffer_ref_t b ) const noexcept
    {
        b.append( _samples );
    }

    void_t copy_samples_to( motor::vector< float_t > & smps ) const noexcept
    {
        smps.resize( _samples.size() );
        std::memcpy( smps.data(), _samples.data(), _samples.size() * sizeof( float_t ) );
    }

    void_t copy_frequencies_to( motor::vector< float_t > & smps ) const noexcept
    {
        size_t const s = _frequencies.size();
        smps.resize( s );
        std::memcpy( smps.data(), _frequencies.data(), s * sizeof( float_t ) );
    }
};
motor_typedef( capture_object );
} // namespace audio
} // namespace motor