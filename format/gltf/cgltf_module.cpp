

#include "cgltf_module.h"
#include "../module_registry.hpp"

#include "../future_items.hpp"

#include <motor/geometry/3d/cube.h>
#include <motor/geometry/mesh/flat_tri_mesh.h>
#include <motor/geometry/mesh/polygon_mesh.h>

#include <motor/std/hash_map>
#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>

#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

#include <motor/core/document.hpp>

#include <cstdlib>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

using namespace motor::format ;

// *****************************************************************************************
void_t cgltf_module_register::register_module( motor::format::module_registry_mtr_t reg )
{
    reg->register_import_factory( { "gltf", "glb" }, motor::shared( cgltf_factory_t(), "cgltf_factory" ) ) ;
    reg->register_export_factory( { "gltf", "glb" }, motor::shared( cgltf_factory_t(), "cgltf_factory" ) ) ;
}

// *****************************************************************************************
motor::format::future_item_t cgltf_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db,
    motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return cgltf_module::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ), motor::move( mod_reg ) ) ;
}

// *****************************************************************************************
motor::format::future_item_t cgltf_module::import_from( motor::io::location_cref_t loc,
    motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps,
    motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        using _clock_t = std::chrono::high_resolution_clock ;
        _clock_t::time_point tp_begin = _clock_t::now() ;

        // can be reused ?
        motor::format::mesh_item_t ret ;

        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps ) ;

        motor::string_t data_buffer ;

        motor::io::database_t::cache_access_t ca = db->load( loc ) ;
        auto const res = ca.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
        {
            data_buffer = motor::string_t( data, sib ) ;
        } ) ;

        if ( !res )
        {
            motor::log::global_t::error( "[wav_import] : can not load location " + loc.as_string() ) ;
            return motor::shared( motor::format::status_item_t( "error" ) ) ;
        }

        cgltf_options options = {};
        cgltf_data* data = NULL;
        cgltf_result result = cgltf_parse(&options, data_buffer.data(), data_buffer.size(), &data);
        if (result == cgltf_result_success)
        {
	        /* TODO make awesome stuff */
	        cgltf_free(data);
        }

        // cgltf impl here
        {
        }

        {
            size_t const milli = std::chrono::duration_cast< std::chrono::milliseconds > 
                ( _clock_t::now() - tp_begin ).count() ;

            motor::log::global_t::status( "[obj_loader] : loading file " + loc.as_string() + " took " + 
                motor::to_string(milli) + " ms."  ) ;
        }

        motor::release( mod_reg ) ;

        //return motor::shared( std::move( ret ), "cgltf mesh_item" ) ;
        return motor::shared( motor::format::status_item_t( "Cgltf export not implemented" ) ) ;
    } ) ;
}

// ***************************************************************************
motor::format::future_item_t cgltf_module::export_to( motor::io::location_cref_t loc,
    motor::io::database_mtr_t, motor::format::item_mtr_safe_t what,
    motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > rel( what ) ;
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ ) ;

        return motor::shared( motor::format::status_item_t( "Cgltf export not implemented" ) ) ;
    } ) ;
}
