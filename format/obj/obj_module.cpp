

#include "obj_module.h"

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

using namespace motor::format ;

namespace this_file
{
    struct tripple
    {
        int32_t pid = 0 ;
        int32_t tid = 0 ;
        int32_t nid = 0 ;
    } ;

    // dissect tokens and make indices tripple
    // @param l must be a "f" line
    static void_t dissect_tripples( std::string_view const & l, motor::vector< tripple > & ret ) noexcept
    {
        ret.clear() ;

        motor::vector< motor::string_t > tokens ;

        // tokenize string line for indices
        {
            size_t s = 2 ;
            size_t p = l.find_first_of( ' ', s ) ;
            while ( p != std::string::npos )
            {
                tokens.emplace_back( l.substr( s, p - s ) ) ;
                s = p + 1 ;
                p = l.find_first_of( ' ', s ) ;
            }

            tokens.emplace_back( l.substr( s, l.size() - s ) ) ;
        }

        size_t const sot = 3 ;

        for ( auto const & t : tokens )
        {
            tripple tr ;
            int32_t * ptr = (int32_ptr_t) ( &tr ) ;

            size_t idx = 0 ;
            size_t s = 0 ;
            size_t p = t.find_first_of( '/', s ) ;
            while ( p != std::string::npos )
            {
                motor::string_t const number = t.substr( s, p - s ) ;
                if ( number.empty() ) ++idx ;
                else ptr[ idx++ ] = std::stoi( std::string( number ) ) ;
                s = p + 1 ;
                p = t.find_first_of( '/', s ) ;
            }

            {
                auto const number = t.substr( s, l.size() - s ) ;
                if ( !number.empty() )
                    ptr[ idx ] = std::stol( std::string( number ) ) ;
            }

            ret.emplace_back( std::move( tr ) ) ;
        }

        tokens.clear() ;
    }

    static void_t _pl_( size_t const indent, motor::string_in_t ln, motor::string_ref_t shader, bool_t const b ) noexcept
    {
        if ( !b ) return ;
        for ( size_t i = 0; i < indent << 2; ++i ) shader += " " ;
        shader += ln + "\n" ;
    }

    static void_t _pl_( size_t const indent, motor::string_in_t ln, motor::string_ref_t shader ) noexcept
    {
        return _pl_( indent, ln, shader, true ) ;
    }

    struct material_info
    {
        motor::string_t name ;

        bool_t has_nrm ;
        bool_t has_tx ;
    };
    motor_typedef( material_info ) ;

    // positions
    // no normals
    // no texcoords
    static motor::string_t generate_forward_shader( material_info_in_t mi ) noexcept
    {
        size_t indent = 0 ;
        motor::string_t shader ;

        _pl_( indent, "config " + mi.name, shader ) ;
        _pl_( indent, "{", shader ) ;
        ++indent ;

        // vertex shader
        {
            _pl_( indent, "vertex_shader", shader ) ;
            _pl_( indent, "{", shader ) ;
            ++indent ;


            _pl_( indent, "in vec3_t pos : position ;", shader ) ;
            _pl_( indent, "in vec3_t nrm : normal ;", shader, mi.has_nrm ) ;
            _pl_( indent, "in vec3_t tx : texcoord0 ;", shader, mi.has_tx ) ;

            _pl_( indent, "", shader ) ;
            _pl_( indent, "out vec4_t pos : position ;", shader ) ;
            _pl_( indent, "out vec3_t nrm : normal ;", shader, mi.has_nrm ) ;
            _pl_( indent, "out vec3_t tx : texcoord0 ;", shader, mi.has_tx ) ;

            _pl_( indent, "", shader ) ;
            _pl_( indent, "mat4_t world : world ;", shader ) ;
            _pl_( indent, "mat4_t view : view ;", shader ) ;
            _pl_( indent, "mat4_t proj : proj ;", shader ) ;
            _pl_( indent, "", shader ) ;

            _pl_( indent, "void main()", shader ) ;
            _pl_( indent, "{", shader ) ;
            ++indent ;
            {
                _pl_( indent, "out.nrm = in.nrm ;", shader, mi.has_nrm ) ;
                _pl_( indent, "out.pos = proj * view * world * vec4_t( in.pos, 1.0 ) ;", shader ) ;
            }
            --indent ;
            _pl_( indent, "}", shader ) ;


            --indent ;
            _pl_( indent, "}", shader ) ;
        }

        {
            _pl_( indent, "pixel_shader", shader ) ;
            _pl_( indent, "{", shader ) ;
            ++indent ;

            _pl_( indent, "in vec3_t nrm : normal ;", shader, mi.has_nrm ) ;
            _pl_( indent, "in vec3_t tx : texcoord0 ;", shader, mi.has_tx ) ;

            _pl_( indent, "vec3_t light_dir ;", shader, mi.has_nrm ) ;
            _pl_( indent, "out vec4_t color : color ;", shader ) ;

            _pl_( indent, "void main()", shader ) ;
            _pl_( indent, "{", shader ) ;
            ++indent ;
            if( mi.has_nrm )
            {
                _pl_( indent, "float_t ndl = dot( in.nrm, light_dir ) ;", shader ) ;
                _pl_( indent, "out.color = vec4_t(ndl, ndl, ndl, 1.0 ) ;", shader ) ;
            }
            else
            {
                _pl_( indent, "out.color = vec4_t(1.0, 1.0, 1.0, 1.0 ) ;", shader ) ;
            }
            --indent ;
            _pl_( indent, "}", shader ) ;
            --indent ;
            _pl_( indent, "}", shader ) ;
        }

        --indent ;
        _pl_( indent, "}", shader ) ;

        return shader ;
    }
}

// *****************************************************************************************
void_t wav_obj_module_register::register_module( motor::format::module_registry_mtr_t reg )
{
    reg->register_import_factory( { "obj" }, "wavefront", motor::shared( wav_obj_factory_t(), "wavefront_obj_factory" ) ) ;
    reg->register_export_factory( { "obj" }, motor::shared( wav_obj_factory_t(), "wavefront_obj_factory" ) ) ;
}

// *****************************************************************************************
motor::format::future_item_t wav_obj_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db ) noexcept
{
    return wav_obj_module::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ) ) ;
}

// *****************************************************************************************
motor::format::future_item_t wav_obj_module::import_from( motor::io::location_cref_t loc,
    motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
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

        motor::vector< motor::io::database_t::cache_access_t > mtl_caches ;

        motor::core::document doc( data_buffer ) ;

        motor::vector< motor::math::vec3f_t > positions ;
        motor::vector< motor::math::vec3f_t > normals ;
        motor::vector< motor::math::vec3f_t > texcoords ;

        // only valid if texcoords actually have values.
        size_t num_texcoord_elems = 3 ;

        // for negative indices.
        // we need to remember the number of vertex data
        // loaded up until the moment, 'f' faces are read.
        // So this numbers can be used to compute the absolute
        // index.
        struct index_offset_capture
        {
            size_t num_pos = 0 ;
            size_t num_nrm = 0 ;
            size_t num_tx = 0 ;
        };
        motor::vector< index_offset_capture > offsets ;
        offsets.reserve( 100 ) ;

        // filter lines
        {
            size_t num_positions = 0 ;
            size_t num_texcoords = 0 ;
            size_t num_normals = 0 ;

            bool_t in_faces = false ;

            doc.for_each_line( [&] ( motor::core::document::line_view const & line )
            {
                if ( line.get_line().size() < 2 ) return ;

                {
                    auto const tok = line.get_token( 0 ) ;

                    // I think 's' should not be supported.
                    // do not know how to smooth faces within
                    // a single mesh. This is not supported.
                    if ( tok[ 0 ] == 's' || tok[ 0 ] == '#' ) return ;

                    if ( tok[ 0 ] != 'f' )
                    {
                        // going out 
                        if ( in_faces )
                        {
                            in_faces = false ;
                            offsets.emplace_back( index_offset_capture
                            {
                                num_positions,
                                num_normals,
                                num_texcoords
                            } ) ;
                        }
                    }
                    else
                    {
                        if ( !in_faces )
                        {
                            in_faces = true ;
                        }
                    }

                    if ( tok.size() == 2 && tok[ 0 ] == 'v' && tok[ 1 ] == 'n' )
                    {
                        ++num_normals ;
                    }
                    else if ( tok.size() == 2 && tok[ 0 ] == 'v' && tok[ 1 ] == 't' )
                    {
                        ++num_texcoords ;
                    }
                    else if ( tok.size() == 1 && tok[ 0 ] == 'v' )
                    {
                        ++num_positions ;
                    }
                    else if ( tok.size() >= 6 && tok[ 0 ] == 'm' && tok[ 1 ] == 't' && tok[ 2 ] == 'l' &&
                        tok[ 3 ] == 'l' && tok[ 4 ] == 'i' && tok[ 5 ] == 'b' )
                    {
                       // mtl_files.emplace_back( line.substr( 7 ) ) ;
                    }
                }


                for ( size_t i = 0; i < line.get_num_tokens(); ++i )
                {
                    auto const tok = line.get_token( i ) ;
                    int bp = 0 ;
                }
            } ) ;

            if( in_faces )
            {
                offsets.emplace_back( index_offset_capture
                    {
                        num_positions,
                        num_normals,
                        num_texcoords
                    } ) ;
            }
            
            positions.reserve( num_positions ) ;
            normals.reserve( num_normals ) ;
            texcoords.reserve( num_texcoords ) ;
        }

        // issue material file loading
        #if 0
        {
            if( mtl_files.size() > 0 )
            {
                mtl_caches.emplace_back( db->load( loc ) ) ;
            }

            // files are loaded at the end before generate msl shaders.
            // for the shader, the vertex layout is required.
        }
        #endif

        // read out coords
        {
            auto read_out_vec3 = [] ( size_t s, std::string_view line, size_t & num_elems, char * tmp_buffer )
            {
                float_t coords[ 3 ] = { 0.0f, 0.0f, 0.0f };

                size_t i = 0 ;
                size_t p = line.find_first_of( ' ', s ) ;
                while ( p != std::string::npos && i < 3 )
                {
                    #if 1
                    size_t const dist = p - s ;
                    std::memcpy( tmp_buffer, line.data()+s, dist ) ;
                    tmp_buffer[dist] = '\0' ;
                    coords[ i++ ] = atof( tmp_buffer ) ;
                    #else
                    coords[ i++ ] = std::stof( std::string( line.substr( s, p - s ) ) ) ;
                    #endif
                    s = p + 1 ;
                    p = line.find_first_of( ' ', s ) ;
                }

                if ( s < line.size() && i < 3 )
                {
                    #if 1
                    size_t const dist = line.size() - s ;
                    std::memcpy( tmp_buffer, line.data() + s, dist ) ;
                    tmp_buffer[ dist ] = '\0' ;
                    coords[ i++ ] = atof( tmp_buffer ) ;
                    #else
                    coords[ i++ ] = std::stof( std::string( line.substr( s, line.size() - s ) ) ) ;
                    #endif
                }

                num_elems = i ;

                return motor::math::vec3f_t( coords[ 0 ], coords[ 1 ], coords[ 2 ] ) ;
            } ;

            char tmp_buffer[1024] ;
            
            doc.for_each_line( [&] ( motor::core::document::line_view const & line )
            {
                auto const tok = line.get_token( 0 ) ;

                size_t num_elems = 0 ;
                // normal
                if ( tok.size() == 2 && tok[ 0 ] == 'v' && tok[ 1 ] == 'n' )
                {
                    normals.emplace_back( read_out_vec3( 3, line.get_line(), num_elems, tmp_buffer ) ) ;
                }
                // tex-coord
                else if ( tok.size() == 2 && tok[ 0 ] == 'v' && tok[ 1 ] == 't' )
                {
                    texcoords.emplace_back( read_out_vec3( 3, line.get_line(), num_elems, tmp_buffer ) ) ;
                    num_texcoord_elems = std::min( num_elems, num_texcoord_elems ) ;
                }
                // position
                else if ( tok.size() == 1 && tok[ 0 ] == 'v' )
                {
                    positions.emplace_back( read_out_vec3( 2, line.get_line(), num_elems, tmp_buffer ) ) ;
                }
            } ) ;

            if( texcoords.size() == 0 )
                num_texcoord_elems = 0 ;
        }

        // check if coordinates need to normalized
        {
            auto * prop = psr->borrow_property<bool_t>( "normalize_coordinate" ) ;
            if( prop != nullptr ) 
            {
                if( prop->get() )
                {
                    float_t greatest = 0.0f ;
                    for( auto const & p : positions )
                    {
                        greatest = std::max( std::abs(p.x()), greatest ) ;
                        greatest = std::max( std::abs(p.y()), greatest ) ;
                        greatest = std::max( std::abs(p.z()), greatest ) ;
                    }

                    for ( auto & p : positions )
                    {
                        p = p / greatest ;
                    }
                }
            }
        }

        struct mesh_data
        {
            struct index_tripple
            {
                uint_t pos_idx ;
                uint_t tx_idx ;
                uint_t nrm_idx ;
            };

            struct face
            {
                byte_t num_vertices = 0 ;
                motor::vector< index_tripple > indices ;
            };
            motor::string_t name ;
            motor::string_t material ;
            motor::vector< face > faces ;
        } ;
        motor::vector< mesh_data > meshes ;

        // read out indices
        // analyze object properties
        // name
        // material name
        // faces
        {
            bool_t in_faces = false ;
            size_t offset_idx = size_t(-1) ;

            mesh_data cur_data ;

            // cache
            motor::vector< this_file::tripple > tripples ;

            doc.for_each_line( [&] ( motor::core::document::line_view const & line )
            {
                auto const l = line.get_line() ;

                // I think 's' should not be supported.
                // do not know how to smooth faces within
                // a single mesh. This is not supported.
                if ( l[ 0 ] == 's' || l[ 0 ] == '#' ) return ;

                if ( l[ 0 ] != 'f' && in_faces )
                {
                    in_faces = false ;
                }

                if ( l[ 0 ] == 'o' || l[ 0 ] == 'g' )
                {
                    if ( !cur_data.name.empty() && cur_data.faces.size() != 0 )
                        meshes.emplace_back( std::move( cur_data ) ) ;

                    if( l.size() >= 2 ) cur_data.name = l.substr( 2 ) ;
                    else cur_data.name = "unnamed_group" ;

                    cur_data.faces.clear() ;
                }
                else if ( l.size() >= 6 &&
                    l[ 0 ] == 'u' && l[ 1 ] == 's' && l[ 2 ] == 'e' &&
                    l[ 3 ] == 'm' && l[ 4 ] == 't' && l[ 5 ] == 'l' )
                {
                    cur_data.material = l.substr( 7, l.size() - 7 ) ;
                }
                else if ( l[ 0 ] == 'f' )
                {
                    if ( !in_faces )
                    {
                        ++offset_idx ;
                        in_faces = true ;
                    }

                    tripples.reserve( line.get_num_tokens() ) ;

                    // f usually looks like this:
                    // pos/tx/nrm
                    // example:
                    // 1//1 or 1/2/3 or 1
                    // the latter is position only

                    mesh_data::face f ;

                    // this section extracts the indices of the vertex
                    // attributes. It also converts from relative negative 
                    // to positive absolute indices.
                    {
                        this_file::dissect_tripples( l, tripples ) ;
                        for ( auto & t : tripples )
                        {
                            if ( t.pid == 0 )
                            {
                                t.pid = 1 ;
                                motor::log::global_t::warning( "[wavefront obj importer] : "
                                    "zero based indices not supported/valid." ) ;
                            }

                            uint_t const pid = t.pid < 0 ?
                                uint_t( offsets[ offset_idx ].num_pos + t.pid ) : (uint_t) t.pid - 1 ;

                            uint_t const tid = t.tid < 0 ?
                                uint_t( offsets[ offset_idx ].num_tx + t.tid ) : (uint_t) t.tid - 1 ;

                            uint_t const nid = t.nid < 0 ?
                                uint_t( offsets[ offset_idx ].num_nrm + t.nid ) : (uint_t) t.nid - 1 ;

                            f.indices.emplace_back( mesh_data::index_tripple
                                {
                                    pid,
                                    tid,
                                    nid
                                } ) ;
                        }
                        f.num_vertices = tripples.size() ;
                    }
                    cur_data.faces.emplace_back( std::move( f ) ) ;
                }
            } ) ;

            if( cur_data.faces.size() != 0 ) 
                meshes.push_back( std::move( cur_data ) ) ;
        }

        {
            ret.geos.resize( meshes.size() ) ;
        }

        // make the polygon mesh
        {
            size_t midx = 0 ;
            for( auto & m : meshes )
            {
                if( m.faces.size() == 0 ) continue ;

                motor::geometry::polygon_mesh pm ;
                pm.position_format = motor::geometry::vector_component_format::xyz ;
                pm.normal_format = normals.size() == 0 ?  motor::geometry::vector_component_format::invalid : motor::geometry::vector_component_format::xyz ;
                pm.texcoord_format = motor::geometry::from_num_components( num_texcoord_elems ) ;

                pm.polygons.resize( m.faces.size() ) ;

                bool_t const has_normals = normals.size() != 0 ;
                bool_t const has_texcoords = texcoords.size() != 0 ;

                size_t num_indices = 0 ;
                size_t num_vertices = 0 ;

                // count some numbers
                {
                    size_t i = 0;

                    for ( auto & f : m.faces )
                    {
                        pm.polygons[ i++ ] = f.num_vertices ;
                        num_indices += f.num_vertices ;
                        num_vertices += f.num_vertices ;
                    }

                    pm.indices.resize( num_indices ) ;
                    pm.positions.resize( num_vertices * 3 ) ;

                    if( has_normals )
                    {
                        // only one layer of normals
                        pm.normals_indices.resize( 1 ) ;
                        pm.normals.resize( 1 ) ;
                        pm.normals_indices[ 0 ].resize( num_indices ) ;
                        pm.normals[0].resize( normals.size() * 3 ) ;
                    }
                    
                    if( texcoords.size() != 0 )
                    {
                        // only one layer of texcoords
                        pm.texcoords_indices.resize( 1 ) ;
                        pm.texcoords.resize( 1 ) ;
                        pm.texcoords_indices[ 0 ].resize( num_indices ) ;
                        pm.texcoords[0].resize( texcoords.size() * num_texcoord_elems ) ;
                    }
                }
                
                using remapping_t = motor::hash_map< size_t, size_t > ;

                remapping_t pos_map ;
                remapping_t nrm_map ;
                remapping_t tx_map ;

                // create remap table for vertex indices
                // map : global -> local
                {
                    size_t idx[ 3 ] = { 0, 0, 0 } ;

                    // remap vertex data
                    for ( auto const & f : m.faces )
                    {
                        for ( auto & fidx : f.indices )
                        {
                            if ( pos_map.find( fidx.pos_idx ) == pos_map.end() )
                                pos_map[ fidx.pos_idx ] = idx[ 0 ]++ ;

                            if ( has_normals )
                            {
                                if ( nrm_map.find( fidx.nrm_idx ) == nrm_map.end() )
                                    nrm_map[ fidx.nrm_idx ] = idx[ 1 ]++ ;
                            }

                            if ( has_texcoords )
                            {
                                if ( tx_map.find( fidx.tx_idx ) == tx_map.end() )
                                    tx_map[ fidx.tx_idx ] = idx[ 2 ]++ ;
                            }
                        }
                    }
                }

                // copy indices
                {
                    size_t idx = 0 ;
                    for ( auto const & f : m.faces )
                    {
                        for ( auto & fidx : f.indices )
                        {
                            pm.indices[ idx ] = pos_map[ fidx.pos_idx ] ;

                            if ( has_normals )
                                pm.normals_indices[ 0 ][ idx ] = nrm_map[ fidx.nrm_idx ] ;
                            if ( has_texcoords )
                                pm.texcoords_indices[ 0 ][ idx ] = tx_map[ fidx.tx_idx ] ;

                            ++idx ;
                        }
                    }
                }
                
                // copy vertex data
                // @note this section copys vertex data from global 
                // arrays to local mesh arrays using the remapping tables.
                {
                    for ( auto const & item : pos_map )
                    {
                        size_t const src_idx = item.first ;
                        size_t const dst_idx = item.second ;

                        size_t const dst_pos = dst_idx * 3 ;

                        pm.positions[ dst_pos + 0 ] = positions[ src_idx ][ 0 ] ;
                        pm.positions[ dst_pos + 1 ] = positions[ src_idx ][ 1 ] ;
                        pm.positions[ dst_pos + 2 ] = positions[ src_idx ][ 2 ] ;
                    }

                    for ( auto const & item : nrm_map )
                    {
                        size_t const src_idx = item.first ;
                        size_t const dst_idx = item.second ;

                        size_t const dst_pos = dst_idx * 3 ;

                        pm.normals[ 0 ][ dst_pos + 0 ] = normals[ src_idx ][ 0 ] ;
                        pm.normals[ 0 ][ dst_pos + 1 ] = normals[ src_idx ][ 1 ] ;
                        pm.normals[ 0 ][ dst_pos + 2 ] = normals[ src_idx ][ 2 ] ;
                    }

                    for ( auto const & item : tx_map )
                    {
                        size_t const src_idx = item.first ;
                        size_t const dst_idx = item.second ;

                        size_t const dst_pos = dst_idx * num_texcoord_elems ;

                        pm.texcoords[ 0 ][ dst_pos + 0 ] = texcoords[ src_idx ][ 0 ] ;
                        pm.texcoords[ 0 ][ dst_pos + 1 ] = texcoords[ src_idx ][ 1 ] ;
                        if ( num_texcoord_elems > 2 )
                            pm.texcoords[ 0 ][ dst_pos + 2 ] = texcoords[ src_idx ][ 2 ] ;
                    }
                }

                ret.geos[midx++].poly = std::move( pm ) ;
            }
        }

        
        auto replace_specials = [=]( motor::string_t & name )
        {
            for ( size_t i = 0; i < name.size(); ++i )
            {
                if ( name[ i ] == '(' || name[ i ] == ')' )
                {
                    name[ i ] = '_' ;
                }
            }
        } ;

        ret.name = loc.as_string() ;
        replace_specials( ret.name ) ;
        
        {
            for( auto ca : mtl_caches )
            {
                auto const res = ca.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
                {
                    data_buffer = motor::string_t( data, sib ) ;
                } ) ;

                if ( !res )
                {
                    motor::log::global_t::error( "[obl_import] : can not load location " + loc.as_string() ) ;
                    continue ;
                }
            }
        }

        // put together a shader per material
        // or maybe just some shading infos?
        {
            // expect normals/texcoords are globally there or not
            bool_t const has_pos = positions.size() != 0 ;
            bool_t const has_nrm = normals.size() != 0 ;
            bool_t const has_tx = texcoords.size() != 0 ;

            size_t midx = 0 ;
            for ( auto & m : meshes )
            {
                replace_specials( m.name ) ;
                motor::string_t name = ret.name + "." + m.name ;
                ret.geos[midx].name = name ;
                ret.geos[midx++].shader = this_file::generate_forward_shader( this_file::material_info_t
                    {
                        name,
                        has_nrm,
                        has_tx
                    } ) ;
            }
        }

        return motor::shared( std::move( ret ), "mesh_item" ) ;
    } ) ;
}

// ***************************************************************************
motor::format::future_item_t wav_obj_module::export_to( motor::io::location_cref_t loc,
    motor::io::database_mtr_t, motor::format::item_mtr_safe_t what ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > rel( what ) ;
        return motor::shared( motor::format::status_item_t( "Wavefront OBJ export not implemented" ) ) ;
    } ) ;
}
