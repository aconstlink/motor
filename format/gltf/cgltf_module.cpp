

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
#include <motor/math/matrix/matrix3.hpp>
#include <motor/math/quaternion/quaternion4.hpp>

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
            cgltf_data * data = NULL;
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

                using node_to_idx_map_t = motor::hash_map< cgltf_node const *, size_t >;

                // maps a gltf node idx to a motor scene node
                using node_node_vec_t = motor::vector< motor::scene::logic_group_mtr_t >;

                // mats a gltf node idx to a motor scene trafo node if there is one
                using node_trafo_vec_t = motor::vector< motor::scene::trafo3d_node_mtr_t >;

                node_to_idx_map_t nti_map;
                node_node_vec_t nn_vec( data->nodes_count, nullptr );
                node_trafo_vec_t nt_vec( data->nodes_count, nullptr );

                // #1 : fill vectors with nodes
                {
                    for( size_t ni = 0; ni < data->nodes_count; ++ni )
                    {
                        auto const & gltf_node = data->nodes[ni];
                        auto motor_node = motor::shared( motor::scene::logic_group_t() );

                        nti_map[&gltf_node] = ni;

                        // attach name
                        {
                            motor::string_t const name =
                                gltf_node.name == nullptr ? "node " + motor::to_string( ni ) : gltf_node.name;

                            motor_node->add_component( motor::shared( motor::scene::name_component_t( name ) ) );
                        }

                        // check for children
                        {
                            // do we have to do it? we just handle
                            // every node as a group node.
                        }

                        // check transformation
                        // make parent trafo3d_node decorator
                        {
                            bool_t attach = gltf_node.has_matrix || gltf_node.has_scale || gltf_node.has_rotation ||
                                            gltf_node.has_translation;

                            motor::math::m3d::trafof_t trafo;

                            if( gltf_node.has_matrix )
                            {
                                // colum major
                                auto const & glm = gltf_node.matrix;

                                motor::math::vec4f_t const col0( glm[0], glm[1], glm[2], glm[3] );
                                motor::math::vec4f_t const col1( glm[4], glm[5], glm[6], glm[7] );
                                motor::math::vec4f_t const col2( glm[8], glm[9], glm[10], glm[11] );
                                motor::math::vec4f_t const col3( glm[12], glm[13], glm[14], glm[15] );

                                motor::math::mat4f_t mat;

                                mat.set_column( 0, col0 );
                                mat.set_column( 1, col1 );
                                mat.set_column( 2, col2 );
                                mat.set_column( 3, col3 );

                                // thoughts:
                                // in a TRS matrix, ths RS matrix should look like this
                                // a b * sx 0  = asx bsy
                                // c d   0  sy   csx dsy
                                // so decomposing requires the length of the column vectors
                                // asx = a * sx
                                // csx   c
                                // so computing the length of the [ac]^T column vector should
                                // result in the sx scaling value.
                                // so to be the rotation matrix R, we need to divide the column
                                // vectors by the length.
                                // I would like to store not the matrix, but the particular
                                // parameters for translation, scaling and roation(quaternion)

                                // has [sx, sy, sz]^T

                                // fortunately, the trafo3 class already does all of this!
                                trafo.set_transformation( mat );
                            }
                            else
                            {
                                if( gltf_node.has_scale )
                                {
                                    trafo.set_scale( motor::math::vec3f_t::from_array( gltf_node.scale ) );
                                }

                                if( gltf_node.has_rotation )
                                {
                                    auto const & q__ = gltf_node.rotation;
                                    motor::math::vec3f_t const axis( q__[0], q__[1], q__[2] );
                                    trafo.rotate_by_axis_fl( motor::math::is_normalized< motor::math::vec3f_t >( axis ),
                                                             q__[3] );
                                }

                                if( gltf_node.has_translation )
                                {
                                    trafo.set_translation( motor::math::vec3f_t::from_array( gltf_node.translation ) );
                                }
                            }

                            if( attach )
                            {
                                nt_vec[ni] = motor::shared( motor::scene::trafo3d_node_t( trafo ) );
                                nt_vec[ni]->set_decorated( motor::share( motor_node ) );
                            }
                        }

                        // check mesh
                        {
                            // make child mesh node
                            // attach to current group
                            // motor_node->add_child( /*mesh_node*/) ;
                        }

                        // check camera
                        {
                            // make child camera node
                            // attach to current group
                            // motor_node->add_child( /*camera_node*/) ;
                        }

                        nn_vec[ni] = motor::move( motor_node );
                    }
                }

                // #2 : connect nodes
                motor::scene::switch_group_t root;

                // #2.1 : start with root nodes. need special handling
                // because those come from the scene data.
                for( size_t si = 0; si < data->scenes_count; ++si )
                {
                    auto const & gltf_scene = data->scenes[si];

                    motor::scene::logic_group_t cur_scene;

                    {
                        motor::string_t const name =
                            gltf_scene.name == nullptr ? "scene node " + motor::to_string( si ) : gltf_scene.name;

                        motor::scene::name_component_t nc( name );
                        cur_scene.add_component( motor::shared( std::move( nc ) ) );
                    }

                    for( size_t ni = 0; ni < gltf_scene.nodes_count; ++ni )
                    {
                        auto const * gltf_node = gltf_scene.nodes[ni];
                        size_t const idx = nti_map[gltf_node];

                        if( nt_vec[idx] == nullptr )
                            cur_scene.add_child( motor::share( nn_vec[idx] ) );
                        else
                            cur_scene.add_child( motor::share( nt_vec[idx] ) );
                    }

                    {
                        bool_t const active = ( size_t( &gltf_scene ) - size_t( data->scene ) ) == 0;
                        root.add_child( motor::shared( std::move( cur_scene ) ), active );
                    }
                }

                // #2.2 : continue with connecting the nodes themselfs.
                {
                    for( size_t i = 0; i < nn_vec.size(); ++i )
                    {
                        auto const & gltf_node = data->nodes[i];

                        auto * group_node = nn_vec[i];

                        for( size_t ci = 0; ci < gltf_node.children_count; ++ci )
                        {
                            size_t const idx = nti_map[gltf_node.children[ci]];

                            if( nt_vec[idx] == nullptr )
                            {
                                group_node->add_child( motor::share( nn_vec[idx] ) );
                            }
                            else
                            {
                                group_node->add_child( motor::share( nt_vec[idx] ) );
                            }
                        }
                    }
                }

                {
                    motor::scene::name_component_t nc( "gltf root" );
                    root.add_component( motor::shared( std::move( nc ) ) );
                    ret.root = motor::shared( std::move( root ) );
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
