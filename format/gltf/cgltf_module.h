#pragma once

#include "../imodule.h"
#include "gltf_module_structs.h"

#include "../module_factory.hpp"
#include "../module_registry.hpp"

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API cgltf_module_register
        {
            static void_t register_module( motor::format::module_registry_mtr_t reg ) ;
        };

        class MOTOR_FORMAT_API cgltf_module : public imodule
        {
            motor_this_typedefs( cgltf_module ) ;

        public:

            virtual ~cgltf_module( void_t ) {}
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t, 
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            // properties:
            // normalize_coordinate : normalize coordinates so that they only are in [-1;1]
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc,
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc,
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

        private:

            
        };
        motor_typedef( cgltf_module ) ;
        typedef motor::format::module_factory<cgltf_module> cgltf_factory_t ;
    }
}