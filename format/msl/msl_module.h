#pragma once

#include "../imodule.h"
#include "../module_factory.hpp"
#include "../module_registry.hpp"

#include <motor/msl/parser_structs.hpp>
#include <motor/graphics/object/shader_object.h>

namespace motor
{
    namespace format
    {
        struct MOTOR_FORMAT_API msl_module_register
        {
            static void_t register_module( motor::format::module_registry_safe_t::mtr_t reg ) ;
        };

        class MOTOR_FORMAT_API msl_module : public imodule
        {
            motor_this_typedefs( msl_module ) ;

        public:

            virtual ~msl_module( void_t ) {}

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_safe_t::mtr_t ) noexcept ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_safe_t::mtr_t, motor::property::property_sheet_safe_t::mtr_t ) noexcept ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc, 
                motor::io::database_safe_t::mtr_t, motor::format::item_safe_t::mtr_t ) noexcept ;

        };
        motor_typedef( msl_module ) ;
        typedef motor::format::module_factory<msl_module> msl_factory_t ;

        struct msl_item : public item
        {
            msl_item( void_t ) {}
            msl_item( motor::msl::post_parse::document_t g ) : doc( g ){}

            motor::msl::post_parse::document_t doc ;
        };
        motor_typedef( msl_item ) ;
    }
}
