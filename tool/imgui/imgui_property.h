

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/std/string>
#include <motor/property/property_sheet.hpp>


namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API imgui_property
        {
        public:

            static void_t handle( motor::string_in_t name, motor::property::property_sheet_inout_t ) noexcept ;
        };
    }
}