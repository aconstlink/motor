
#include "global.h"
#include "stb/stb_module.h"
#include "msl/msl_module.h"
#include "wav/wav_module.h"
#include "motor/motor_module.h"

#include <motor/memory/global.h>

using namespace motor::format ;

//***
void_t register_default_registries( motor::format::module_registry_mtr_t ) 
{
    //motor::format::stb_module_register::register_module( _mr ) ;
    //motor::format::msl_module_register::register_module( _mr ) ;
    //motor::format::wav_module_register::register_module( _mr ) ;
    //motor::format::motor_module_register::register_module( _mr ) ;
}