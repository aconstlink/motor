// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"

namespace motor::audio
{
    using namespace motor::core::types ;
	class frequency_bands
{
    motor_this_typedefs( frequency_bands );

  private:

    float_t _bin_width = 0.0f;
    size_t _max_bins = 0;

  public:

    frequency_bands( float_t const bin_width, size_t const max_bins ) noexcept
        : _bin_width( bin_width ), _max_bins( max_bins )
    {
        assert( std::isfinite( _bin_width ) && _bin_width > 0 );
        assert( _max_bins > 0 );
    }

    frequency_bands( this_cref_t ) noexcept = default;
    frequency_bands( this_rref_t ) noexcept = default;

    ~frequency_bands( void_t ) noexcept {}

  public:

    using for_each_bin_funk_t = std::function< void_t( size_t const i, size_t const g ) >;
    void_t for_each(
        float_t const hz_begin, float_t const hz_end, for_each_bin_funk_t && f ) noexcept
    {
        assert( std::isfinite( hz_begin ) && hz_begin >= 0.0f );
        assert( std::isfinite( hz_end ) && hz_end >= hz_begin );

        auto const to_index = [ & ]( float_t const hz )
        {
            double const index = std::ceil( double( hz ) / double( _bin_width ) );
            return size_t( std::min( index, double( _max_bins ) ) );
        };

        size_t const s = to_index( hz_begin );
        size_t const e = to_index( hz_end );

        // g : global index [s;e[
        // i : local index [0, num bins in band]
        for( size_t g = s, i = 0; g < e; ++g, ++i )
        {
            f( i, g );
        }
    }
};
}