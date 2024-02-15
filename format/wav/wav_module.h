#pragma once

#include "../imodule.h"
#include "../module_factory.hpp"
#include "../module_registry.hpp"

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API wav_module_register
        {
            static void_t register_module( motor::format::module_registry_mtr_t reg ) ;
        };

        class MOTOR_FORMAT_API wav_audio_module : public imodule
        {
        public:

            virtual ~wav_audio_module( void_t ) {}
            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t ) noexcept ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t ) noexcept ;

        };
        motor_typedef( wav_audio_module ) ;
        typedef motor::format::module_factory<wav_audio_module> wav_audio_factory_t ;

    }
}
