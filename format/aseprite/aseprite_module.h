#pragma once

#include "../imodule.h"
#include "../module_factory.hpp"
#include "../module_registry.hpp"

#include "aseprite_structs.h"

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API aseprite_module_register
        {
            static void_t register_module( motor::format::module_registry_mtr_t reg ) noexcept ;
        };

        class MOTOR_FORMAT_API aseprite_module : public imodule
        {
            motor_this_typedefs( aseprite_module ) ;

        public:

            virtual ~aseprite_module( void_t ) {}

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc,
                motor::io::database_mtr_t ) noexcept ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc,
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc,
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t ) noexcept ;

        };
        motor_typedef( aseprite_module ) ;
        typedef motor::format::module_factory<aseprite_module> aseprite_factory_t ;

        struct aseprite_json_item : public item
        {
            aseprite_json_item( void_t ) {}
            aseprite_json_item( motor::format::aseprite::json_document_rref_t g ) : doc( std::move( g ) ) {}

            motor::format::aseprite::json_document_t doc ;
        };
        motor_typedef( aseprite_json_item ) ;
    }
}
