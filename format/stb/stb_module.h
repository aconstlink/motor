#pragma once

#include "../imodule.h"
#include "../module_factory.hpp"
#include "../module_registry.hpp"

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API stb_module_register
        {
            static void_t register_module( motor::format::module_registry_mtr_t reg ) ;
        };

        class MOTOR_FORMAT_API stb_image_module : public imodule
        {
        public:

            virtual ~stb_image_module( void_t ) {}
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t, 
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;
            
        };
        motor_typedef( stb_image_module ) ;
        typedef motor::format::module_factory<stb_image_module> stb_image_factory_t ;

        class MOTOR_FORMAT_API stb_audio_module : public imodule
        {
        public:

            virtual ~stb_audio_module( void_t ) noexcept {}
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;
        };
        motor_typedef( stb_audio_module ) ;
        typedef motor::format::module_factory<stb_audio_module_t> stb_audio_factory_t ;

        class MOTOR_FORMAT_API stb_font_module : public imodule
        {
        public:

            virtual ~stb_font_module( void_t ) noexcept {}
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t,
                motor::format::module_registry_mtr_safe_t ) noexcept ;
        };
        motor_typedef( stb_font_module ) ;
        typedef motor::format::module_factory<stb_font_module_t> stb_font_factory_t ;
    }
}
