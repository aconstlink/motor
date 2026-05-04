

#include "cgltf_module.h"
#include "../module_registry.hpp"

#include "../future_items.hpp"

#include <motor/scene/node/logic_leaf.h>
#include <motor/scene/node/logic_group.h>
#include <motor/scene/node/switch_group.h>
#include <motor/scene/node/trafo3d_node.h>

#include <motor/scene/component/name_component.hpp>

#include <motor/geometry/3d/cube.h>
#include <motor/geometry/mesh/flat_tri_mesh.h>
#include <motor/geometry/mesh/polygon_mesh.h>

#include <motor/std/hash_map>
#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>

#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>
#include <motor/math/matrix/matrix4.hpp>

#include <motor/core/document.hpp>

#include <cstdlib>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

using namespace motor::format;

// *****************************************************************************************
void_t cgltf_module_register::register_module( motor::format::module_registry_mtr_t reg )
{
    reg->register_import_factory( { "gltf", "glb" }, motor::shared( cgltf_factory_t(), "cgltf_factory" ) );
    reg->register_export_factory( { "gltf", "glb" }, motor::shared( cgltf_factory_t(), "cgltf_factory" ) );
}

// *****************************************************************************************
motor::format::future_item_t cgltf_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db,
                                                        motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return cgltf_module::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ),
                                      motor::move( mod_reg ) );
}

// *****************************************************************************************
motor::format::future_item_t cgltf_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db,
                                                        motor::property::property_sheet_mtr_safe_t ps,
                                                        motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return std::async(
        std::launch::async,
        [=]( void_t ) mutable -> item_mtr_t
        {
            using _clock_t = std::chrono::high_resolution_clock;
            _clock_t::time_point tp_begin = _clock_t::now();

            motor::format::scene_item_t ret;

            motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps );

            motor::string_t data_buffer;

            motor::io::database_t::cache_access_t ca = db->load( loc );
            auto const res = ca.wait_for_operation( [&]( char_cptr_t data, size_t const sib, motor::io::result const )
                                                    { data_buffer = motor::string_t( data, sib ); } );

            if( !res )
            {
                motor::log::global_t::error( "[cgltf] : can not load location " + loc.as_string() );
                return motor::shared( motor::format::status_item_t( "error" ) );
            }

            cgltf_options options = {};
            cgltf_data *data = NULL;
            cgltf_result result = cgltf_parse( &options, data_buffer.data(), data_buffer.size(), &data );

            if( result != cgltf_result_success )
            {
                cgltf_free( data );
                motor::release( mod_reg );
                return motor::shared( motor::format::status_item_t( "[cgltf] : import failed." ) );
            }

            // cgltf impl here
            {
                using node_node_map_t = motor::hash_map< cgltf_node const *, motor::scene::logic_group_ptr_t >;

                motor::scene::switch_group_t root;
                for( size_t si = 0; si < data->scenes_count; ++si )
                {
                    auto const &gltf_scene = data->scenes[si];

                    bool_t const active = ( size_t( &gltf_scene ) - size_t( data->scene ) ) == 0;

                    motor::scene::logic_group_t cur_scene;

                    {
                        motor::string_t const name =
                            gltf_scene.name == nullptr ? "root node " + motor::to_string( si ) : gltf_scene.name;

                        motor::scene::name_component_t nc( name );
                        cur_scene.add_component( motor::shared( std::move( nc ) ) );
                    }

                    node_node_map_t nnmap;

                    //
                    // #1: go over all root nodes first.
                    //
                    for( size_t ni = 0; ni < gltf_scene.nodes_count; ++ni )
                    {
                        auto const &gltf_node = gltf_scene.nodes[ni];

                        motor::string_t const name =
                            gltf_node->name == nullptr ? "root node " + motor::to_string( ni ) : gltf_scene.name;

                        motor::scene::name_component_t nc( name );

                        if( gltf_node->children_count > 0 )
                        {
                            // group
                            auto lg = motor::shared( motor::scene::logic_group_t() );
                            lg->add_component( motor::shared( std::move( nc ) ) );

                            root.add_child( motor::share( lg ), active );

                            // only store groups
                            nnmap[gltf_node] = lg.move();
                        }
                        else
                        {
                            // leaf node
                            root.add_child( motor::shared( motor::scene::logic_leaf_t() ), active );
                        }
                    }

                    //
                    // #2: construct the node tree
                    //
                    {
                        auto nnmap2 = std::move( nnmap );
                        while( nnmap2.size() > 0 )
                        {
                            for( auto &nn : nnmap2 )
                            {
                                auto const *gltf_parent = nn.first;
                                auto *parent = nn.second;

                                for( size_t ni = 0; ni < gltf_parent->children_count; ++ni )
                                {
                                    auto const *gltf_node = gltf_parent->children[ni];

                                    if( gltf_node->children_count > 0 )
                                    {
                                        motor::string_t const name = gltf_node->name == nullptr
                                                                         ? "node " + motor::to_string( ni )
                                                                         : gltf_scene.name;

                                        motor::scene::name_component_t nc( name );

                                        // group
                                        auto lg = motor::shared( motor::scene::logic_group_t() );
                                        lg->add_component( motor::shared( std::move( nc ) ) );

                                        parent->add_child( motor::share( lg ) );

                                        // only store groups
                                        nnmap[gltf_node] = lg.move();
                                    }
                                    else
                                    {
                                        motor::scene::node_mtr_t child = motor::shared( motor::scene::logic_leaf_t() );

                                        motor::string_t const name = gltf_node->name == nullptr
                                                                         ? "node " + motor::to_string( ni )
                                                                         : gltf_scene.name;

                                        motor::scene::name_component_t nc( name );

                                        if( gltf_node->mesh != nullptr )
                                        {
                                            // attach mesh
                                        }

                                        bool_t has_trafo = false;
                                        motor::math::m3d::trafof_t trafo;

                                        if( gltf_node->has_matrix )
                                        {
                                            // colum major
                                            auto const &glm = gltf_node->matrix;

                                            motor::math::vec4f_t const col0( glm[0], glm[1], glm[2], glm[3] );
                                            motor::math::vec4f_t const col1( glm[4], glm[5], glm[6], glm[7] );
                                            motor::math::vec4f_t const col2( glm[8], glm[9], glm[10], glm[11] );
                                            motor::math::vec4f_t const col3( glm[12], glm[13], glm[14], glm[15] );

                                            motor::math::mat4f_t mat;

                                            mat.set_column( 0, col0 );
                                            mat.set_column( 1, col1 );
                                            mat.set_column( 2, col2 );
                                            mat.set_column( 3, col3 );

                                            has_trafo = true;
                                        }
                                        else
                                        {
                                            if( gltf_node->has_translation )
                                            {
                                                has_trafo = true;
                                            }
                                            if( gltf_node->has_rotation )
                                            {
                                                has_trafo = true;
                                            }
                                            if( gltf_node->has_scale )
                                            {
                                                has_trafo = true;
                                            }
                                        }

                                        if( has_trafo )
                                        {
                                            motor::scene::trafo3d_node_t tn( trafo );
                                            tn.set_decorated( motor::move( child ) );

                                            child = motor::shared( std::move( tn ) );
                                        }

                                        // leaf node
                                        parent->add_child( motor::move( child ) );
                                    }
                                }
                            }

                            nnmap2 = std::move( nnmap );
                        }
                    }

                    ret.root = motor::shared( std::move( root ), "gltf imported root node" );
                }
            }

            {
                size_t const milli =
                    std::chrono::duration_cast< std::chrono::milliseconds >( _clock_t::now() - tp_begin ).count();

                motor::log::global_t::status( "[cgltf] : loading file " + loc.as_string() + " took " +
                                              motor::to_string( milli ) + " ms." );
            }

            cgltf_free( data );
            motor::release( mod_reg );

            return motor::shared( std::move( ret ), "[cgltf] : module item" );
        } );
}

// ***************************************************************************
motor::format::future_item_t cgltf_module::export_to( motor::io::location_cref_t loc, motor::io::database_mtr_t,
                                                      motor::format::item_mtr_safe_t what,
                                                      motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept
{
    return std::async( std::launch::async,
                       [=]( void_t ) mutable -> item_mtr_t
                       {
                           motor::mtr_release_guard< motor::format::item_t > rel( what );
                           motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ );

                           return motor::shared( motor::format::status_item_t( "Cgltf export not implemented" ) );
                       } );
}
