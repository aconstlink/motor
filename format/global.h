

#pragma once

#include "api.h"
#include "typedefs.h"
#include "module_registry.hpp"

namespace motor
{
    namespace format
    {
        MOTOR_FORMAT_API_C void_t register_default_registries( motor::format::module_registry_mtr_t ) ;
    }
}