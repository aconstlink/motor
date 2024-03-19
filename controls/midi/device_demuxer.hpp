

#pragma once

#include "akai_midimix.hpp"
#include "akai_apc_k25.hpp"
#include "korg_nanokontrol2.hpp"

namespace motor
{
    namespace controls
    {

        namespace midi
        {
            static motor::controls::midi_device_t demux_device( motor::string_cref_t name, 
                motor::controls::midi_device_rref_t dev ) noexcept
            {
                if( name == "MIDI Mix" )
                {
                    return motor::controls::midi::create_akai_midimix( std::move( dev ) ) ;
                }


                return std::move( dev ) ;
            }
        }
    }
}