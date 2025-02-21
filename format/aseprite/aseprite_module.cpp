
#include "aseprite_module.h"
#include "../module_registry.hpp"

#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>
#include <motor/graphics/texture/image.hpp>
#include <motor/math/vector/vector4.hpp>

#include <nlohmann/json.hpp>

#include <array>
#include <sstream>

using namespace motor::format ;

namespace this_file
{
   
}

//****************************************************************************************************
void_t aseprite_module_register::register_module( motor::format::module_registry_mtr_t reg ) noexcept
{
    reg->register_import_factory( { "json" }, "aseprite", motor::shared( aseprite_factory_t() ) ) ;
    // only import at the moment
    //reg->register_export_factory( { "motor" }, motor::shared( motor_factory_t() ) ) ;
}

//****************************************************************************************************
motor::format::future_item_t aseprite_module::import_from( motor::io::location_cref_t loc,
    motor::io::database_mtr_t db, motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return this_t::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ), motor::move( mod_reg ) ) ;
}

//****************************************************************************************************
motor::format::future_item_t aseprite_module::import_from( motor::io::location_cref_t loc,
    motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps,
    motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ ) ;

        motor::format::aseprite::json_document_t doc ;

        motor::memory::malloc_guard<char> content ;
        
        // #1 : load file content
        {
            auto const res = db->load( loc ).wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
            {
                motor::string_t file = motor::string_t( data, sib ) ;
                content = motor::memory::malloc_guard<char>( file.c_str(), file.size() + 1 ) ;
            } ) ;

            if ( !res )
            {
                motor::log::global_t::warning( "[aseprite_module] : failed to load file [" + loc.as_string() + "]" ) ;

                return motor::shared( motor::format::status_item_t( "failed to load file" ) ) ;
            }
        }

        nlohmann::json jdoc = nlohmann::json::parse( std::string(content) ) ;

        if( jdoc.contains("frames") )
        {
            auto & frames_obj = jdoc[ "frames" ];

            for( auto & [frame_key, frame_value] : frames_obj.items() )
            {
                //motor::log::global_t::status( motor::string_t( frame_obj.key() ) ) ;
                if( frame_value.contains("frame") )
                {
                    auto const & item = frame_value[ "frame" ] ;
                    int_t const x = item[ "x" ] ;
                    int_t const y = item[ "y" ] ;
                    int_t const w = item[ "w" ] ;
                    int_t const h = item[ "h" ] ;
                }

                if ( frame_value.contains( "rotated" ) )
                {
                }

                if ( frame_value.contains( "trimmed" ) )
                {
                }

                if ( frame_value.contains( "spriteSourceSize" ) )
                {
                    auto const & item = frame_value[ "spriteSourceSize" ] ;
                    int_t const x = item[ "x" ] ;
                    int_t const y = item[ "y" ] ;
                    int_t const w = item[ "w" ] ;
                    int_t const h = item[ "h" ] ;
                }

                if ( frame_value.contains( "sourceSize" ) )
                {
                    auto const & item = frame_value[ "sourceSize" ] ;
                    int_t const x = item[ "x" ] ;
                    int_t const y = item[ "y" ] ;
                    int_t const w = item[ "w" ] ;
                    int_t const h = item[ "h" ] ;
                }

                if ( frame_value.contains( "duration" ) )
                {
                    int_t const d = frame_value[ "duration" ] ;
                }
            }
        }

        if ( jdoc.contains( "meta" ) )
        {
            auto & meta_obj = jdoc[ "meta" ];

            if( meta_obj.contains("app") )
            {

            }

            if ( meta_obj.contains( "version" ) )
            {

            }

            if ( meta_obj.contains( "image" ) )
            {

            }

            if ( meta_obj.contains( "format" ) )
            {

            }

            if ( meta_obj.contains( "size" ) )
            {
                auto const & item = meta_obj[ "size" ] ;
                int_t const w = item[ "w" ] ;
                int_t const h = item[ "h" ] ;
            }

            if ( meta_obj.contains( "scale" ) )
            {
            }

            if ( meta_obj.contains( "frame_tags" ) )
            {
                auto const & ft_obj = meta_obj[ "frame_tags" ] ;

                for( auto & item : meta_obj[ "frame_tags" ] )
                {
                    if( item.contains("name") )
                    {
                        item[ "name" ] ;
                    }

                    if ( item.contains( "from" ) )
                    {
                        item[ "from" ] ;
                    }

                    if ( item.contains( "to" ) )
                    {
                        item[ "to" ] ;
                    }

                    if ( item.contains( "direction" ) )
                    {
                        item[ "direction" ] ;
                    }

                    if ( item.contains( "color" ) )
                    {
                        item[ "color" ] ;
                    }
                }
            }

            if ( meta_obj.contains( "layers" ) )
            {
            }

            if ( meta_obj.contains( "slices" ) )
            {
            }
        }


        return motor::shared( motor::format::aseprite_json_item_t( std::move( doc ) ) ) ;
    } ) ;
}

//****************************************************************************************************
motor::format::future_item_t aseprite_module::export_to( motor::io::location_cref_t loc,
    motor::io::database_mtr_t db, motor::format::item_mtr_safe_t what, motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > resl( what ) ;
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ ) ;
        return motor::shared( motor::format::status_item_t( "Export not implemented" ) ) ;
    } ) ;
}