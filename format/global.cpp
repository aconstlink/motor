
#include "global.h"
#include "stb/stb_module.h"
#include "msl/msl_module.h"
#include "wav/wav_module.h"
#include "motor/motor_module.h"

#include <motor/memory/global.h>

using namespace motor::format ;

//***
motor::format::module_registry_mtr_t global::register_default_modules( motor::format::module_registry_mtr_t mr ) 
{
    motor::format::stb_module_register::register_module( mr ) ;
    motor::format::msl_module_register::register_module( mr ) ;
    motor::format::wav_module_register::register_module( mr ) ;
    motor::format::motor_module_register::register_module( mr ) ;

    return mr ;
}