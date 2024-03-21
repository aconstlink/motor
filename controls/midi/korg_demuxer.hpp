#pragma once

#include "../types/midi_controller.hpp"
#include "../components/button.hpp"
#include "../components/slider.hpp"
#include "../components/knob.hpp"
#include "../components/led.hpp"
#include "../components/touch.hpp"

#include "layouts/korg_layouts.hpp"

#include <motor/memory/global.h>

namespace motor
{
    namespace controls
    {
        namespace midi
        {
            //************************************************************************************************
            static bool_t demux_korg( motor::string_cref_t name, motor::controls::midi_device_ref_t dev ) noexcept
            {
                if( motor::controls::midi::layouts::korg_nanokontrol2::create_layout( name, dev ) )
                {
                    return true ;
                }
                else if( motor::controls::midi::layouts::korg_nanopad2::create_layout( name, dev )  )
                {
                    return true ;
                }
                return false ;
            }
        }
    }
}