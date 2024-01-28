#pragma once

namespace motor
{
    namespace audio
    {
        enum class execution_state
        {
            undefined,
            playing,
            paused,
            stopped
        };
        enum class execution_options
        {
            undefined,
            play,
            pause,
            stop,
            replay
        };
        enum class channels
        {
            undefined,
            mono,
            stereo
        };

        enum class frequency
        {
            undefined,
            freq_48k,
            freq_96k,
            freq_128k,
            max_frequencies
        };

        static channels to_channels( size_t const n ) noexcept
        {
            channels ret = channels::undefined ;
            switch( n )
            {
            case 1: ret = channels::mono ; break ;
            case 2: ret = channels::stereo ; break ;
            default: break;
            }
            return ret ;
        }

        static size_t to_number( frequency const f ) noexcept
        {
            static size_t const _numbers[] = { 0, 48000, 96000, 128000 } ;
            return _numbers[ size_t(f) < size_t(frequency::max_frequencies) ? size_t(f) : 0 ] ;
        }

        static size_t to_number( channels const c ) noexcept
        {
            return size_t( c ) ;
        }
    }
}