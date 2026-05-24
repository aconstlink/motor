

#include "cgltf_module.h"
#include "../module_registry.hpp"

#include "../future_items.hpp"

#include <motor/gfx/camera/generic_camera.h>

#include <motor/scene/node/logic_leaf.h>
#include <motor/scene/node/logic_group.h>
#include <motor/scene/node/switch_group.h>

#include <motor/scene/component/name_component.hpp>
#include <motor/scene/component/trafo3d_component.h>
#include <motor/scene/component/camera_component.h>
#include <motor/scene/component/msl_component.h>
#include <motor/scene/component/config_graphics_component.h>

#include <motor/graphics/buffer/vertex_buffer.hpp>
#include <motor/graphics/buffer/index_buffer.hpp>

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

namespace cgltf_module_file
{

// converts cgltf_type -> motor type_struct
static motor::graphics::type_struct to_typestruct( cgltf_type const t ) noexcept
{
    switch( t )
    {
    case cgltf_type::cgltf_type_scalar:
        return motor::graphics::type_struct::vec1;
    case cgltf_type::cgltf_type_vec2:
        return motor::graphics::type_struct::vec2;
    case cgltf_type::cgltf_type_vec3:
        return motor::graphics::type_struct::vec3;
    case cgltf_type::cgltf_type_vec4:
        return motor::graphics::type_struct::vec4;
    case cgltf_type::cgltf_type_mat2:
        return motor::graphics::type_struct::mat2;
    case cgltf_type::cgltf_type_mat3:
        return motor::graphics::type_struct::mat3;
    case cgltf_type::cgltf_type_mat4:
        return motor::graphics::type_struct::mat4;
    default:
        break;
    }
    return motor::graphics::type_struct::undefined;
}

// converts cgltf_component_type -> motor graphics type
static motor::graphics::type to_type( cgltf_component_type const ct ) noexcept
{
    switch( ct )
    {
    case cgltf_component_type::cgltf_component_type_r_8: /* BYTE */
        return motor::graphics::type::tchar;
    case cgltf_component_type::cgltf_component_type_r_8u: /* UNSIGNED_BYTE */
        return motor::graphics::type::tuchar;
    case cgltf_component_type::cgltf_component_type_r_16: /* SHORT */
        return motor::graphics::type::tshort;
    case cgltf_component_type::cgltf_component_type_r_16u: /* UNSIGNED_SHORT */
        return motor::graphics::type::tushort;
    case cgltf_component_type::cgltf_component_type_r_32u: /* UNSIGNED_INT */
        return motor::graphics::type::tuint;
    case cgltf_component_type::cgltf_component_type_r_32f: /* FLOAT */
        return motor::graphics::type::tfloat;
    }
    return motor::graphics::type::undefined;
}

static motor::graphics::primitive_type to_primitive_type( cgltf_primitive_type const pt ) noexcept
{
    switch( pt )
    {
    case cgltf_primitive_type::cgltf_primitive_type_points:
        return motor::graphics::primitive_type::points;
    case cgltf_primitive_type::cgltf_primitive_type_lines:
        return motor::graphics::primitive_type::lines;
    case cgltf_primitive_type::cgltf_primitive_type_line_loop:
        return motor::graphics::primitive_type::undefined;
    case cgltf_primitive_type::cgltf_primitive_type_line_strip:
        return motor::graphics::primitive_type::undefined;
    case cgltf_primitive_type::cgltf_primitive_type_triangles:
        return motor::graphics::primitive_type::triangles;
    case cgltf_primitive_type::cgltf_primitive_type_triangle_strip:
        return motor::graphics::primitive_type::undefined;
    case cgltf_primitive_type::cgltf_primitive_type_triangle_fan:
        return motor::graphics::primitive_type::undefined;
    default:
        break;
    }
    return motor::graphics::primitive_type::undefined;
}
} // namespace cgltf_module_file

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
    return std::async( std::launch::async, [ = ]( void_t ) mutable -> item_mtr_t
    {
        using _clock_t = std::chrono::high_resolution_clock;
        _clock_t::time_point tp_begin = _clock_t::now();

        motor::format::scene_item_t ret;

        motor::mtr_release_guard< motor::property::property_sheet_t > psr( ps );

        motor::string_t data_buffer;

        motor::io::database_t::cache_access_t ca = db->load( loc );
        auto const res = ca.wait_for_operation( [ & ]( char_cptr_t data, size_t const sib, motor::io::result const )
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

        // before starting to create anything, load the buffers data
        {
            struct user_data
            {
                motor::io::location_cref_t loc;
                motor::io::database_mtr_t db;
            };

            user_data ud__ = { loc, db };

            using cgltf_read_funk_t = cgltf_result ( * )( const struct cgltf_memory_options * memory_options,
                                                          const struct cgltf_file_options * file_options,
                                                          const char * path, cgltf_size * size, void ** data );

            cgltf_read_funk_t read = []( const struct cgltf_memory_options * memory_options,
                                         const struct cgltf_file_options * file_options, const char * path,
                                         cgltf_size * size, void ** data )
            {
                user_data * ud = (user_data *)( file_options->user_data );

                auto const new_loc = ud->loc.sub_location( -2 ) + motor::io::location_t::from_path( path );

                motor::io::database_t::cache_access_t ca = ud->db->load( new_loc );

                motor::string_t the_data;
                auto const res =
                    ca.wait_for_operation( [ & ]( char_cptr_t data, size_t const sib, motor::io::result const )
                { the_data = motor::string_t( data, sib ); } );

                if( !res )
                {
                    motor::log::global_t::error( "[cgltf] : can not load location " + new_loc.as_string() );
                    return cgltf_result_file_not_found;
                }

                *size = the_data.size();
                *data = cgltf_default_alloc( nullptr, *size );

                std::memcpy( *data, the_data.data(), *size );

                return cgltf_result_success;
            };

            cgltf_options opts = {};
            cgltf_file_options fro = {};
            fro.read = cgltf_read_funk_t( read );
            fro.user_data = &ud__;
            opts.file = std::move( fro );

            auto const path = motor::filesystem::absolute( loc.as_path() );
            auto const lres = cgltf_load_buffers( &opts, data, (const char *)path.c_str() );
            if( lres != cgltf_result_success )
            {
                cgltf_free( data );
                motor::release( mod_reg );
                return motor::shared( motor::format::status_item_t( "[cgltf] : import failed." ) );
            }
        }

        // going through the file
        {
            using node_node_map_t = motor::hash_map< cgltf_node const *, motor::scene::logic_group_ptr_t >;

            using node_to_idx_map_t = motor::hash_map< cgltf_node const *, size_t >;

            // maps a gltf node idx to a motor scene node
            using node_node_vec_t = motor::vector< motor::scene::logic_group_mtr_t >;

            // mats a gltf node idx to a motor scene trafo node if there is one
            //using node_trafo_vec_t = motor::vector< motor::scene::trafo3d_node_mtr_t >;

            node_to_idx_map_t nti_map;
            node_node_vec_t nn_vec( data->nodes_count, nullptr );

            // using mesh_to_comp_t = motor::hash_map< cgltf_mesh *, motor::scene::msl_component_safe_t >;
            // mesh_to_comp_t mesh_to_comp;

            using node_to_msls_t =
                motor::hash_map< cgltf_mesh const *, motor::vector< motor::graphics::msl_object_ptr_t > >;
            using node_to_geos_t =
                motor::hash_map< cgltf_mesh const *, motor::vector< motor::graphics::geometry_object_ptr_t > >;
            using name_to_geo_t = motor::hash_map< motor::string_t, motor::graphics::geometry_object_ptr_t >;

            node_to_msls_t node_to_msls;
            node_to_geos_t node_to_geos;
            name_to_geo_t name_to_geo;

            // hadle meshes
            // @note per mesh, there are multiple primitives.
            // each primitive is associated with a geometry, msl, shader and more
            {
                for( size_t midx = 0; midx < data->meshes_count; ++midx )
                {
                    auto const & cgltf_mesh = data->meshes[ midx ];

                    motor::string_t name = motor::string_t( cgltf_mesh.name ) + "_";

                    for( size_t pidx = 0; pidx < cgltf_mesh.primitives_count; ++pidx )
                    {
                        auto const & p = cgltf_mesh.primitives[ pidx ];

                        auto const lname = name + motor::to_string( pidx );

                        motor::graphics::vertex_buffer_t vb;
                        motor::graphics::index_buffer_t ib;

                        auto const pt = cgltf_module_file::to_primitive_type( p.type );
                        size_t num_verts_render = 0;

                        // according to the spec, this must be the same for
                        // every attribute.
                        // 1. If no indices are present, this is
                        // the number of vertices to be rendered.
                        // 2. If indecies are present, this is the
                        // max index value.
                        auto const num_vertices = p.attributes[ 0 ].data->count;
                        num_verts_render = num_vertices;

                        // iterate first time for vertex buffer layout
                        for( size_t aidx = 0; aidx < p.attributes_count; ++aidx )
                        {
                            auto const & a = p.attributes[ aidx ];

                            switch( a.type )
                            {
                            case cgltf_attribute_type::cgltf_attribute_type_position:
                                // index of attribute
                                // POSITION(0), NORMAL(0), TEXTCOORD_0(0), ...
                                a.index;
                                // goes to accessor
                                a.data;

                                vb.add_layout_element( motor::graphics::vertex_attribute::position,
                                                       motor::graphics::type::tfloat,
                                                       motor::graphics::type_struct::vec3 );
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_normal:
                                vb.add_layout_element( motor::graphics::vertex_attribute::normal,
                                                       motor::graphics::type::tfloat,
                                                       motor::graphics::type_struct::vec3 );
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_tangent:
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_texcoord:
                                vb.add_layout_element( motor::graphics::texcoord_vertex_attribute_by( a.index ),
                                                       cgltf_module_file::to_type( a.data->component_type ),
                                                       motor::graphics::type_struct::vec2 );
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_color:

                                vb.add_layout_element( motor::graphics::color_vertex_attribute_by( a.index ),
                                                       cgltf_module_file::to_type( a.data->component_type ),
                                                       cgltf_module_file::to_typestruct( a.data->type ) );
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_joints:
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_weights:
                                break;
                            case cgltf_attribute_type::cgltf_attribute_type_custom:
                                break;
                            default:
                                break;
                            }
                        }

                        vb.resize( num_vertices );

                        // iterate a second time for vertex data
                        {
                            // this offset shifts the starting point of
                            // a write operation in the newly created vb.
                            // it is incremented each time an attribute is written.
                            size_t vb_offset_sib = 0;

                            size_t const vertex_sib = vb.get_layout_sib();
                            for( size_t aidx = 0; aidx < p.attributes_count; ++aidx )
                            {
                                // a : attribute
                                // a.data : accessor
                                auto const & a = p.attributes[ aidx ];

                                switch( a.type )
                                {
                                case cgltf_attribute_type::cgltf_attribute_type_position: {
                                    size_t const bo = a.data->offset;
                                    size_t const cnt = a.data->count;
                                    size_t const str = a.data->stride;

                                    auto const & ac = *a.data;
                                    auto const & bv = *ac.buffer_view;
                                    auto const & bf = *bv.buffer;

                                    // write position by position in the interleaved vb
                                    vb.update< byte_t >( [ & ]( byte_ptr_t wt_buf, size_t const /*ne*/ )
                                    {
                                        size_t const wrt_vertex_sib = vb.get_layout_sib();

                                        size_t const rd_offset = bv.offset + ac.offset;
                                        byte_t * rd_buf = reinterpret_cast< byte_ptr_t >( bf.data ) + rd_offset;

                                        wt_buf += vb_offset_sib;

                                        for( size_t i = 0; i < num_vertices; ++i )
                                        {
                                            std::memcpy( wt_buf, rd_buf, sizeof( motor::math::vec3f_t ) );

                                            auto v3 = (motor::math::vec3f_t *)( wt_buf );
                                            v3->z( -v3->z() );

                                            wt_buf += vertex_sib;

                                            // cgftf.h says, if bv.stride == 0, it is automatically
                                            // determined by the accessor. So lets use that one.
                                            rd_buf += bv.stride == 0 ? ac.stride : bv.stride;
                                        }
                                    } );

                                    // according to spec, this is always a vec3f
                                    vb_offset_sib += sizeof( motor::math::vec3f_t );
                                }
                                break;
                                case cgltf_attribute_type::cgltf_attribute_type_normal: {
                                    size_t const bo = a.data->offset;
                                    size_t const cnt = a.data->count;
                                    size_t const str = a.data->stride;

                                    auto const & ac = *a.data;
                                    auto const & bv = *ac.buffer_view;
                                    auto const & bf = *bv.buffer;

                                    // write normal by normal in the interleaved vb
                                    vb.update< byte_t >( [ & ]( byte_ptr_t wt_buf, size_t const /*ne*/ )
                                    {
                                        size_t const wrt_vertex_sib = vb.get_layout_sib();

                                        size_t const rd_offset = bv.offset + ac.offset;
                                        byte_t * rd_buf = reinterpret_cast< byte_ptr_t >( bf.data ) + rd_offset;

                                        wt_buf += vb_offset_sib;

                                        for( size_t i = 0; i < num_vertices; ++i )
                                        {
                                            std::memcpy( wt_buf, rd_buf, sizeof( motor::math::vec3f_t ) );
                                            wt_buf += vertex_sib;
                                            // cgftf.h says, if bv.stride == 0, it is automatically
                                            // determined by the accessor. So lets use that one.
                                            rd_buf += bv.stride == 0 ? ac.stride : bv.stride;
                                        }
                                    } );

                                    // according to spec, this is always a vec3f
                                    vb_offset_sib += sizeof( motor::math::vec3f_t );
                                }
                                break;
                                case cgltf_attribute_type::cgltf_attribute_type_tangent:
                                    break;
                                case cgltf_attribute_type::cgltf_attribute_type_texcoord: {
                                    size_t const bo = a.data->offset;
                                    size_t const cnt = a.data->count;
                                    size_t const str = a.data->stride;

                                    auto const & ac = *a.data;
                                    auto const & bv = *ac.buffer_view;
                                    auto const & bf = *bv.buffer;

                                    // write texcoord by texcoord in the interleaved vb
                                    vb.update< byte_t >( [ & ]( byte_ptr_t wt_buf, size_t const /*ne*/ )
                                    {
                                        size_t const wrt_vertex_sib = vb.get_layout_sib();

                                        size_t const rd_offset = bv.offset + ac.offset;
                                        byte_t * rd_buf = reinterpret_cast< byte_ptr_t >( bf.data ) + rd_offset;

                                        wt_buf += vb_offset_sib;
                                        size_t const elem_sib = cgltf_calc_size( a.data->type, a.data->component_type );

                                        for( size_t i = 0; i < num_vertices; ++i )
                                        {
                                            std::memcpy( wt_buf, rd_buf, elem_sib );
                                            wt_buf += vertex_sib;
                                            // cgftf.h says, if bv.stride == 0, it is automatically
                                            // determined by the accessor. So lets use that one.
                                            rd_buf += bv.stride == 0 ? ac.stride : bv.stride;
                                        }
                                    } );

                                    vb_offset_sib += cgltf_calc_size( a.data->type, a.data->component_type );
                                }
                                break;
                                case cgltf_attribute_type::cgltf_attribute_type_color:
                                    break;
                                case cgltf_attribute_type::cgltf_attribute_type_joints:
                                    break;
                                case cgltf_attribute_type::cgltf_attribute_type_weights:
                                    break;
                                case cgltf_attribute_type::cgltf_attribute_type_custom:
                                    break;
                                default:
                                    break;
                                }
                            }
                        }

                        // has index buffer
                        if( p.indices != nullptr && p.indices->buffer_view != nullptr &&
                            p.indices->buffer_view->buffer != nullptr )
                        {
                            auto const & ac = *p.indices;
                            auto const & bv = *ac.buffer_view;
                            auto const & bf = *bv.buffer;

                            num_verts_render = ac.count;

                            cgltf_module_file::to_type( ac.component_type );

                            ib.set_layout_element( motor::graphics::type::tuint );
                            ib.resize( num_verts_render )
                                .update< uint_t >( [ & ]( uint_ptr_t wt_buf, size_t const /*ne*/ )
                            {
                                size_t const wrt_index_sib = ib.get_element_sib();

                                size_t const rd_offset = bv.offset + ac.offset;
                                byte_t * rd_buf = reinterpret_cast< byte_ptr_t >( bf.data ) + rd_offset;

                                size_t const elem_sib = cgltf_calc_size( ac.type, ac.component_type );

                                for( size_t i = 0; i < num_verts_render; ++i )
                                {
                                    // byte_t bytes[ 4 ] = { 0, 0, 0, 0 };
                                    uint_t bytes = 0;
                                    std::memcpy( &bytes, rd_buf, elem_sib );

                                    *wt_buf = uint_t( bytes );
                                    ++wt_buf;

                                    // cgftf.h says, if bv.stride == 0, it is automatically
                                    // determined by the accessor. So lets use that one.
                                    rd_buf += bv.stride == 0 ? ac.stride : bv.stride;
                                }
                            } );
                        }

                        // has material
                        if( p.material != nullptr )
                        {
                        }

                        // temporarily make shader
                        {
                        }

                        motor::string_t const geo_name =
                            motor::string_t( cgltf_mesh.name ) + "." + motor::to_string( pidx );

                        {
                            motor::graphics::geometry_object_t geo( geo_name, pt, std::move( vb ), std::move( ib ) );
                            // node_to_geos[ &cgltf_mesh ].emplace_back(  );
                            name_to_geo[ geo_name ] = motor::shared( std::move( geo ) );
                        }

                        motor::graphics::msl_object_t mslo( geo_name + ".msl" );
                        mslo.link_geometry( geo_name );

                        {
                            motor::string_t shd = "config " + geo_name + "_msl";
                            shd +=
                                R"(
                            {
                                vertex_shader
                                {
                                    mat4_t proj : projection ;
                                    mat4_t view : view ;
                                    mat4_t world : world ;

                                    in vec3_t pos : position ;
                                    in vec3_t nrm : normal ;
                                    in vec2_t tx : texcoord ;

                                    out vec4_t pos : position ;
                                    out vec2_t tx : texcoord ;
                                    out vec3_t nrm : normal ;

                                    void main()
                                    {
                                        vec3_t pos = in.pos ;
                                        //pos.xyz = pos.xyz;
                                        out.tx = in.tx ;
                                        out.pos = proj * view * world * vec4_t( pos, 1.0 ) ;
                                        out.nrm = normalize( world * vec4_t( in.nrm, 0.0 ) ).xyz ;
                                    }
                                }

                                pixel_shader
                                {
                                    tex2d_t tex ;
                                    vec3_t light_dir ;
                                    vec4_t color ;
                                    float_t time ;

                                    in vec2_t tx : texcoord ;
                                    in vec3_t nrm : normal ;
                                    out vec4_t color : color0 ;
                                    //out vec4_t color1 : color1 ;
                                    //out vec4_t color2 : color2 ;

                                    void main()
                                    {
                                        float_t light = dot( normalize( in.nrm ), normalize( light_dir ) ) ;
                                        out.color = vec4_t( light*time, light, light, 1.0 ) ;
                                        out.color = vec4_t( in.nrm.x, in.nrm.y, in.nrm.z, 1.0 ) ;

                                        //out.color = color ' texture( tex, in.tx ) ;
                                        //out.color1 = vec4_t( in.nrm, 1.0 ) ;
                                        //out.color2 = vec4_t( light, light, light , 1.0 ) ;
                                    }
                                }
                            })";

                            mslo.add( motor::graphics::msl_api_type::msl_4_0, shd );

                            // msl_obj = motor::shared( motor::graphics::msl_object_t( std::move( mslo ) ) );
                            node_to_msls[ &( data->meshes[ midx ] ) ].emplace_back(
                                motor::shared( std::move( mslo ) ) );
                        }

                    } // primitives
                }
            }

            // #1 : fill vectors with nodes
            // no hierarchy is build here. This section just checks the nodes and
            // creates motor node objects.
            // @note components are still handeled here. if a node has a camera/mesh/etc. attached,
            // a corresponding motor component is created here and is attached.
            // The next section handles building up groups. At the moment, I couldn't
            // figure out is a camera tagged gltf node can have children. So the current setup
            // works, if a camera had children.
            {
                for( size_t ni = 0; ni < data->nodes_count; ++ni )
                {
                    auto const & gltf_node = data->nodes[ ni ];
                    auto motor_node = motor::shared( motor::scene::logic_group_t() );

                    nti_map[ &gltf_node ] = ni;

                    // attach name
                    {
                        motor::string_t const node_name =
                            gltf_node.name == nullptr ? "node " + motor::to_string( ni ) : gltf_node.name;

                        motor_node->add_component( motor::shared( motor::scene::name_component_t( node_name ) ) );
                    }

                    // check transformation
                    // make parent trafo3d_node decorator
                    // if a node has a transformation, remember in the
                    // nt_vec array. When building the group hierarchy,
                    // we can check the nt_vec array and determine if
                    // we need to attach the transformation decorator
                    // of the group node. See next section.
                    {
                        bool_t attach = gltf_node.has_matrix || gltf_node.has_scale || gltf_node.has_rotation ||
                                        gltf_node.has_translation;

                        motor::math::m3d::trafof_t trafo;

                        if( gltf_node.has_matrix )
                        {
                            // colum major
                            auto const & glm = gltf_node.matrix;

                            motor::math::vec4f_t const col0( glm[ 0 ], glm[ 1 ], glm[ 2 ], glm[ 3 ] );
                            motor::math::vec4f_t const col1( glm[ 4 ], glm[ 5 ], glm[ 6 ], glm[ 7 ] );
                            motor::math::vec4f_t const col2( glm[ 8 ], glm[ 9 ], glm[ 10 ], glm[ 11 ] );
                            motor::math::vec4f_t const col3( glm[ 12 ], glm[ 13 ], glm[ 14 ], glm[ 15 ] );

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
                        else // has no matrix
                        {
                            if( gltf_node.has_scale )
                            {
                                trafo.set_scale( motor::math::vec3f_t::from_array( gltf_node.scale ) );
                            }

                            if( gltf_node.has_rotation )
                            {
                                auto const & q__ = gltf_node.rotation;

                                // #1 in this engine, z is going into the screen.
                                //      gltf defines is out of the sceen.
                                // #2 I think, the engines' rotation direction is CW.
                                //      so we have to negate.
                                motor::math::quat4f_t const q( -q__[ 3 ], q__[ 0 ], q__[ 1 ], -q__[ 2 ] );

                                trafo.rotate_by_matrix_fl( q.to_matrix() );
                            }

                            if( gltf_node.has_translation )
                            {
                                trafo.set_translation( motor::math::vec3f_t::from_array( gltf_node.translation ) );
                            }
                        }

                        if( attach )
                        {
                            motor::scene::trafo3d_component_t tc( std::move( trafo ) );
                            motor_node->add_component( motor::shared( std::move( tc ) ) );
                        }
                    }

                    // check mesh
                    if( gltf_node.mesh != nullptr )
                    {
                        auto const * gltf_mesh = gltf_node.mesh;

                        // attach components
                        {
                            auto iter = node_to_msls.find( gltf_node.mesh );
                            if( iter != node_to_msls.end() )
                            {
                                for( auto * msl : iter->second )
                                {
                                    auto const & geo_name = msl->get_geometry().front();

                                    auto geo_iter = name_to_geo.find( geo_name );
                                    if( geo_iter != name_to_geo.end() )
                                    {
                                        motor::scene::leaf_t render_node;

                                        // add name compoent
                                        {
                                            motor::scene::name_component_t nc( geo_name );
                                            render_node.add_component( motor::shared( std::move( nc ) ) );
                                        }

                                        // add msl compoent
                                        {
                                            auto comp = motor::scene::msl_component_t( motor::share( msl ), 0 );
                                            render_node.add_component( motor::shared( std::move( comp ) ) );
                                        }

                                        // add msl configuration comp
                                        {
                                            auto comp = motor::scene::config_graphics_component_t();
                                            comp.set_msl( motor::share( msl ) );
                                            comp.set_geo( motor::share( geo_iter->second ) );
                                            render_node.add_component( motor::shared( std::move( comp ) ) );
                                        }

                                        motor_node->add_child( motor::shared( std::move( render_node ) ) );
                                    }
                                    else
                                    {
                                        motor::log::global_t::warning(
                                            "[cgltf_module] : referenced geometry not found : " + geo_name );
                                    }
                                }
                            }
                        }
                    }

                    // check camera
                    if( gltf_node.camera != nullptr )
                    {
                        auto const & gltf_cam = *gltf_node.camera;

                        // make child camera node
                        // attach to current group
                        // motor_node->add_child( /*camera_node*/) ;

                        motor::gfx::generic_camera cam;

                        switch( gltf_cam.type )
                        {
                        case cgltf_camera_type::cgltf_camera_type_perspective: {
                            gltf_cam.data.perspective.znear;
                            gltf_cam.data.perspective.yfov;

                            float_t w = 1000.0f;
                            float_t h = 1000.0f;
                            float_t n = 1.0f;
                            float_t f = 10000.0f;
                            float_t const fov = gltf_cam.data.perspective.yfov;

                            if( gltf_cam.data.perspective.has_aspect_ratio )
                            {
                                w = w * gltf_cam.data.perspective.aspect_ratio;
                            }

                            if( gltf_cam.data.perspective.has_zfar )
                            {
                                f = gltf_cam.data.perspective.zfar;
                            }

                            cam.make_perspective_fov( w, h, fov, n, f );

                            // cam.make_perspective_fov();
                            break;
                        }
                        case cgltf_camera_type::cgltf_camera_type_orthographic: {
                            // cam.make_orthographic() ;
                            break;
                        }
                        default:
                            break;
                        }

                        // handle camera component
                        {
                            auto cam_comp = motor::scene::camera_component_t( motor::shared( std::move( cam ) ) );
                            motor_node->add_component( motor::shared( std::move( cam_comp ) ) );
                        }
                    }
                    nn_vec[ ni ] = motor::move( motor_node );
                }
            }

            // #x : connect group and transformation nodes in order to
            // form a scene graph tree.
            {
                motor::scene::switch_group_t root;

                // #2.1 : start with root nodes. need special handling
                // because those come from the scene data.
                for( size_t si = 0; si < data->scenes_count; ++si )
                {
                    auto const & gltf_scene = data->scenes[ si ];

                    motor::scene::logic_group_t cur_scene;

                    {
                        motor::string_t const name =
                            gltf_scene.name == nullptr ? "scene node " + motor::to_string( si ) : gltf_scene.name;

                        motor::scene::name_component_t nc( name );
                        cur_scene.add_component( motor::shared( std::move( nc ) ) );
                    }

                    for( size_t ni = 0; ni < gltf_scene.nodes_count; ++ni )
                    {
                        auto const * gltf_node = gltf_scene.nodes[ ni ];
                        size_t const idx = nti_map[ gltf_node ];
                        cur_scene.add_child( motor::share( nn_vec[ idx ] ) );
                    }

                    {
                        bool_t const active = ( size_t( &gltf_scene ) - size_t( data->scene ) ) == 0;
                        root.add_child( motor::shared( std::move( cur_scene ) ), active );
                    }
                }

                // #2.2 : continue with connecting the nodes themselfs.
                // - if the node has a transformation, add the trafo node as a child
                // - if the node has no transformation, just add the group node.
                {
                    for( size_t i = 0; i < nn_vec.size(); ++i )
                    {
                        auto const & gltf_node = data->nodes[ i ];

                        auto * group_node = nn_vec[ i ];

                        for( size_t ci = 0; ci < gltf_node.children_count; ++ci )
                        {
                            size_t const idx = nti_map[ gltf_node.children[ ci ] ];
                            group_node->add_child( motor::share( nn_vec[ idx ] ) );
                        }
                    }
                }

                // finally, add the root node to the return value.
                {
                    motor::scene::name_component_t nc( "gltf root" );
                    root.add_component( motor::shared( std::move( nc ) ) );
                    ret.root = motor::shared( std::move( root ) );
                }
            }

            // #3 : handle animaions
            {
                for( size_t i = 0; i < data->animations_count; ++i )
                {
                    auto const & ani = data->animations[ i ];

                    // handle channels. a channel contains the target and
                    // the sampler
                    for( size_t c = 0; c < ani.channels_count; ++c )
                    {
                        auto const & channel = ani.channels[ c ];

                        cgltf_animation_path_type const path = channel.target_path;
                        switch( path )
                        {
                        case cgltf_animation_path_type::cgltf_animation_path_type_rotation:
                            break;
                        case cgltf_animation_path_type::cgltf_animation_path_type_translation:
                            break;
                        case cgltf_animation_path_type::cgltf_animation_path_type_scale:
                            break;
                        default:
                            motor::log::global_t::warning( "[gltf_module] : animation path not supported." );
                            break;
                        }
                    }

                    // handle samplers. a sampler contains the keyframe (input) and
                    // value (output) reference along with the interpolation.
                    for( size_t s = 0; s < ani.samplers_count; ++s )
                    {
                        auto const & sampler = ani.samplers[ s ];
                    }
                }
            }

            // release all object refs
            {
                for( auto i : node_to_msls )
                {
                    for( auto i2 : i.second )
                    {
                        motor::release( motor::move(i2) ) ;
                    }
                    
                }

                for( auto i : node_to_geos )
                {
                    for( auto i2 : i.second )
                    {
                        motor::release( motor::move(i2) ) ;
                    }
                }

                for( auto i : name_to_geo )
                {
                    motor::release( motor::move(i.second) ) ;
                } 

                for( auto i : nn_vec )
                {
                    motor::release( motor::move(i) ) ;
                }
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
    return std::async( std::launch::async, [ = ]( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > rel( what );
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ );

        return motor::shared( motor::format::status_item_t( "Cgltf export not implemented" ) );
    } );
}
