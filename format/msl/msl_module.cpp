
#include "msl_module.h"

#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/graphics/texture/image.hpp>
#include <motor/math/vector/vector4.hpp>

#include <motor/msl/parser.h>
#include <motor/graphics/shader/msl_bridge.hpp>

using namespace motor::format ;


// ***
void_t msl_module_register::register_module( motor::format::module_registry_mtr_t reg )
{
    reg->register_import_factory( { "msl" }, motor::shared( msl_factory_t() ) ) ;
    reg->register_export_factory( { "msl" }, motor::shared( msl_factory_t() ) ) ;
}

// ***
motor::format::future_item_t msl_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db ) noexcept
{
    return this_t::import_from( loc, std::move( db ), motor::shared( motor::property::property_sheet_t() ) ) ;
}

// ***
motor::format::future_item_t msl_module::import_from( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;

        motor::msl::post_parse::document_t doc ;

        motor::io::database_t::cache_access_t ca = db->load( loc ) ;
        {
            auto const res = ca.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
            {
                motor::string_t file = motor::string_t( data, sib ) ;
                doc = motor::msl::parser_t( loc.as_string() ).process( std::move( file ) ) ;
            } ) ;

            if( !res )
            {
                motor::log::global_t::warning( "[msl_module] : failed to load file [" + loc.as_string() + "]" ) ;

                return motor::shared( motor::format::status_item_t( "failed to load file" ) ) ;
            }
        }

        return motor::shared( motor::format::msl_item_t( std::move( doc ) ) ) ;
    } ) ;
}

// ***
motor::format::future_item_t msl_module::export_to( motor::io::location_cref_t loc, 
                motor::io::database_mtr_t, motor::format::item_mtr_safe_t what ) noexcept 
{
    return std::async( std::launch::async, [=] ( void_t ) mutable ->item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > releaser( what ) ;
        return motor::shared( motor::format::status_item_t( "Export not implemented" ) ) ;
    } ) ;
}