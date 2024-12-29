#pragma once

#include "../imodule.h"
#include "obj_module_structs.h"

#include "../module_factory.hpp"
#include "../module_registry.hpp"

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API wav_obj_module_register
        {
            static void_t register_module( motor::format::module_registry_mtr_t reg ) ;
        };

        class MOTOR_FORMAT_API wav_obj_module : public imodule
        {
        public:

            virtual ~wav_obj_module( void_t ) {}
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t ) noexcept ;

            // properties:
            // normalize_coordinate : normalize coordinates so that they only are in [-1;1]
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc,
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc,
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t ) noexcept ;

        private:

            static mtl_file load_mtl_file( motor::io::location_in_t loc, motor::string_rref_t the_file ) noexcept ;
        };
        motor_typedef( wav_obj_module ) ;
        typedef motor::format::module_factory<wav_obj_module> wav_obj_factory_t ;
    }
}