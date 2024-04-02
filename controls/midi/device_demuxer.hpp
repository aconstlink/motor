

#pragma once

#include "akai_demuxer.hpp"
#include "korg_demuxer.hpp"

namespace motor
{
    namespace controls
    {

        namespace midi
        {
            static void_t demux_device( motor::string_cref_t name, 
                motor::controls::midi_device_inout_t dev ) noexcept
            {
                if( motor::controls::midi::demux_akai( name, dev ) ) return ;
                if( motor::controls::midi::demux_korg( name, dev ) ) return ;
            }
        }
    }
}