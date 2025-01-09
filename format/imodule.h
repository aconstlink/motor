#pragma once 

#include "api.h"
#include "typedefs.h"
#include "future_items.hpp"

#include <motor/property/property_sheet.hpp>
#include <motor/io/database.h>
namespace motor

{
    namespace format
    {
        class MOTOR_FORMAT_API imodule
        {
        public:

            //imodule( void_t ) {}
            virtual ~imodule( void_t ) {}

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::module_registry_mtr_safe_t ) noexcept = 0 ;

            virtual motor::format::future_item_t import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::property::property_sheet_mtr_safe_t, motor::format::module_registry_mtr_safe_t ) noexcept = 0 ;

            virtual motor::format::future_item_t export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t, motor::format::module_registry_mtr_safe_t ) noexcept = 0 ;
        };
        motor_typedef( imodule ) ;
    }
}