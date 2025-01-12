

#include "obj_module.h"
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
    static size_t dissect_tripples( motor::core::document::line_view const & line, motor::vector< tripple > & ret ) noexcept
    {
        ret.resize( line.get_num_tokens() ) ;

        char tmp_buf[1024] ;

        size_t tr_pos = 0 ;
        tripple tr ;

        for( size_t i=1; i<line.get_num_tokens(); ++i )
        {
            auto const & t = line.get_token( i ) ;

            size_t s = 0 ;
            size_t p = 0 ;
            
            do
            {
                while ( p < t.size() && t[ p ] != '/' ) ++p ;

                size_t const dist = p - s ;
                std::memcpy( tmp_buf, t.data() + s, dist ) ;
                tmp_buf[ dist ] = '\0' ;

                ((int32_ptr_t)(&tr))[tr_pos++] = std::atoi( tmp_buf ) ;
                s = ++p ;
                
            } while ( p < t.size() ) ;
            
            tr_pos = 0 ;
            ret[i-1] = tr ;

            std::memset( (void_ptr_t)&tr, 0, sizeof(tripple) ) ;
        }

        return line.get_num_tokens() - 1 ;
    }
}

// *****************************************************************************************
void_t wav_obj_module_register::register_module( motor::format::module_registry_mtr_t reg )
{
    reg->register_import_factory( { "obj" }, "wavefront", motor::shared( wav_obj_factory_t(), "wavefront_obj_factory" ) ) ;
    reg->register_export_factory( { "obj" }, motor::shared( wav_obj_factory_t(), "wavefront_obj_factory" ) ) ;
}

// *****************************************************************************************
motor::format::future_item_t wav_obj_module::import_from( motor::io::location_cref_t loc, motor::io::database_mtr_t db,
    motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return wav_obj_module::import_from( loc, db, motor::shared( motor::property::property_sheet_t() ), motor::move( mod_reg ) ) ;
}

// *****************************************************************************************
motor::format::future_item_t wav_obj_module::import_from( motor::io::location_cref_t loc,
    motor::io::database_mtr_t db, motor::property::property_sheet_mtr_safe_t ps,
    motor::format::module_registry_mtr_safe_t mod_reg ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        using _clock_t = std::chrono::high_resolution_clock ;
        _clock_t::time_point tp_begin = _clock_t::now() ;

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

        motor::vector< std::pair< motor::io::location_t, 
            motor::io::database_t::cache_access_t > > mtl_caches ;

        motor::core::document const doc( data_buffer ) ;

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
            // abuse this struct for 
            // counting the number of faces.
            size_t num_faces = 0 ;
            size_t num_pos = 0 ;
            size_t num_nrm = 0 ;
            size_t num_tx = 0 ;
        };
        motor::vector< index_offset_capture > offsets ;
        offsets.reserve( 100 ) ;

        // filter lines
        {
            motor::vector< motor::string_t > mtl_files ;

            size_t num_faces = 0 ;
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
                                num_faces,
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
                            num_faces = 1 ;
                        }
                        ++num_faces;
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
                       mtl_files.emplace_back( line.get_line().substr( 7 ) ) ;
                    }
                }
            } ) ;

            if( in_faces )
            {
                offsets.emplace_back( index_offset_capture
                    {
                        num_faces,
                        num_positions,
                        num_normals,
                        num_texcoords
                    } ) ;
            }
            
            positions.reserve( num_positions ) ;
            normals.reserve( num_normals ) ;
            texcoords.reserve( num_texcoords ) ;

            // issue material file loading
            if ( mtl_files.size() > 0 )
            {
                for( auto const & mtlf : mtl_files )
                {
                    motor::io::location_t mtl_loc = motor::io::location_t::from_path( 
                        loc.as_path().parent_path().append( mtlf ) ) ;

                    mtl_caches.emplace_back( std::make_pair( mtl_loc, db->load( mtl_loc ) ) ) ;
                }
            }
        }

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

            size_t cur_mesh_idx = 0 ;
            mesh_data cur_data ;
            //cur_data.faces.reserve( offsets[0].num_faces ) ;

            // cache
            motor::vector< this_file::tripple > tripples ;

            // cache for every face
            mesh_data::face cache_f ;

            motor::string_t group_name ;
            size_t count = 0 ;

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
                    #if 0
                    if ( !cur_data.name.empty() && cur_data.faces.size() != 0 )
                        meshes.emplace_back( std::move( cur_data ) ) ;

                    if( l.size() >= 2 ) cur_data.name = l.substr( 2 ) ;
                    else cur_data.name = "unnamed_group_" + motor::to_string( cur_mesh_idx++ ) ;

                    cur_data.faces.clear() ;
                    #endif

                    if ( l.size() >= 2 ) group_name = l.substr( 2 ) ;
                    else group_name = "unnamed_group_" + motor::to_string( cur_mesh_idx++ ) + ".";

                }
                else if ( l.size() >= 6 &&
                    l[ 0 ] == 'u' && l[ 1 ] == 's' && l[ 2 ] == 'e' &&
                    l[ 3 ] == 'm' && l[ 4 ] == 't' && l[ 5 ] == 'l' )
                {
                    if ( !cur_data.name.empty() && cur_data.faces.size() != 0 )
                        meshes.emplace_back( std::move( cur_data ) ) ;

                    cur_data.material = l.substr( 7, l.size() - 7 ) ;
                    cur_data.name = group_name + "." + cur_data.material + "." + motor::to_string( count++ ) ;
                    cur_data.faces.clear() ;
                }

                // f usually looks like this:
                // pos/tx/nrm
                // example:
                // 1//1 or 1/2/3 or 1
                // the latter is position only
                else if ( l[ 0 ] == 'f' )
                {
                    cache_f.num_vertices = 0 ;

                    if ( !in_faces )
                    {
                        ++offset_idx ;
                        in_faces = true ;
                        //cur_data.faces.reserve( offsets[offset_idx].num_faces ) ;
                    }

                    // this section extracts the indices of the vertex
                    // attributes. It also converts from relative negative 
                    // to positive absolute indices.
                    {
                        auto num_tripples = this_file::dissect_tripples( line, tripples ) ;
                        cache_f.indices.reserve( num_tripples ) ;

                        for ( size_t nt = 0; nt < num_tripples; ++nt )
                        {
                            auto & t = tripples[nt] ;

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

                            cache_f.indices.emplace_back( mesh_data::index_tripple
                                {
                                    pid,
                                    tid,
                                    nid
                                } ) ;
                        }
                        cache_f.num_vertices = num_tripples ;
                    }
                    cur_data.faces.emplace_back( std::move( cache_f ) ) ;
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
        
        motor::vector< motor::format::mtl_file > mtl_files ;

        {
            for( auto ca : mtl_caches )
            {
                motor::string_t mtl_file_content ;

                auto const res = ca.second.wait_for_operation( [&] ( char_cptr_t data, size_t const sib, motor::io::result const )
                {
                    mtl_file_content = motor::string_t( data, sib ) ;
                } ) ;

                if ( !res )
                {
                    motor::log::global_t::error( "[obl_import] : can not load material at location " + ca.first.as_string() ) ;
                    continue ;
                }

                mtl_files.emplace_back( motor::format::wav_obj_module::load_mtl_file( 
                    ca.first, std::move( mtl_file_content ), db, motor::share( mod_reg ) ) ) ;
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
                motor::format::mtl_file::material mtl ;

                bool_t material_found = false ;

                for( auto const & mtlf : mtl_files )
                {
                    auto const iter = std::find_if( mtlf.materials.begin(), mtlf.materials.end(), 
                        [&]( motor::format::mtl_file::material const & v ) 
                    {
                        return v.name == m.material ;
                    } ) ;

                    if( iter != mtlf.materials.end() ) 
                    {
                        mtl = *iter ;
                        material_found = true ;
                        break ;
                    }
                }

                if( !material_found )
                    motor::log::global::warning( "[wav_obj_module] : could not find material " + m.material ) ;

                replace_specials( m.name ) ;
                motor::string_t name = ret.name + "." + m.name ;
                ret.geos[midx].name = name ;
                ret.geos[midx++].shader = this_t::generate_forward_shader( motor::format::material_info_t
                    {
                        name,
                        has_nrm,
                        has_tx,
                        byte_t(num_texcoord_elems),
                        mtl
                    } ) ;
            }
        }

        {
            size_t const milli = std::chrono::duration_cast< std::chrono::milliseconds > 
                ( _clock_t::now() - tp_begin ).count() ;

            motor::log::global_t::status( "[obj_loader] : loading file " + loc.as_string() + " took " + 
                motor::to_string(milli) + " ms."  ) ;
        }

        motor::release( mod_reg ) ;

        return motor::shared( std::move( ret ), "mesh_item" ) ;
    } ) ;
}

// ***************************************************************************
motor::format::future_item_t wav_obj_module::export_to( motor::io::location_cref_t loc,
    motor::io::database_mtr_t, motor::format::item_mtr_safe_t what,
    motor::format::module_registry_mtr_safe_t mod_reg_ ) noexcept
{
    return std::async( std::launch::async, [=] ( void_t ) mutable -> item_mtr_t
    {
        motor::mtr_release_guard< motor::format::item_t > rel( what ) ;
        motor::mtr_release_guard< motor::format::module_registry_t > mod_reg( mod_reg_ ) ;

        return motor::shared( motor::format::status_item_t( "Wavefront OBJ export not implemented" ) ) ;
    } ) ;
}
