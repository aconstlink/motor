

#include "obj_module.h"

#include "../future_items.hpp"

#include <motor/geometry/3d/cube.h>
#include <motor/geometry/mesh/flat_tri_mesh.h>
#include <motor/geometry/mesh/polygon_mesh.h>

#include <motor/io/database.h>
#include <motor/memory/malloc_guard.hpp>

#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

using namespace motor::format ;

namespace this_file
{
    struct tripple
    {
        uint16_t pid = 0 ;
        uint16_t tid = 0 ;
        uint16_t nid = 0 ;
    } ;

    // dissect tokens and make indices tripple
    // @param l must be a "f" line
    static motor::vector< tripple > dissect_tripples( motor::string_in_t l ) noexcept
    {
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

        motor::vector< tripple > ret ;

        for ( auto const & t : tokens )
        {
            tripple tr ;
            uint16_t * ptr = (uint16_ptr_t) ( &tr ) ;

            size_t idx = 0 ;
            size_t s = 0 ;
            size_t p = t.find_first_of( '/', s ) ;
            while ( p != std::string::npos )
            {
                motor::string_t const number = t.substr( s, p - s ) ;
                if ( number.empty() ) ++idx ;
                else ptr[ idx++ ] = std::stol( std::string( number ) ) ;
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

        return ret ;
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

        motor::vector< motor::string_t > lines ;
        motor::vector< motor::string_t > mtl_files ;

        // fill lines vector
        {
            size_t s = 0 ;
            size_t p = data_buffer.find_first_of( '\n' ) ;
            while( p != std::string::npos )
            {
                size_t const pb = p - 1 ;
                size_t const e = (pb < data_buffer.size() && data_buffer[pb] == '\r') ? pb : p ;
                
                auto line = data_buffer.substr( s, e - s ) ;
                lines.emplace_back( std::move( line ) ) ;

                s = p + 1 ;
                p = data_buffer.find_first_of( '\n', s ) ;
            }
        }

        // remove multiple white spaces
        {
            for( auto & l : lines )
            {
                motor::string_t tmp = std::move( l ) ;

                size_t s = 0 ;
                size_t p = tmp.find_first_of( ' ', s ) ;
                while( p != std::string::npos )
                {
                    l += tmp.substr( s, p - s ) + " ";
                    s = tmp.find_first_not_of( ' ', p ) ;
                    if( s == std::string::npos ) break ;
                    p = tmp.find_first_of( ' ', s ) ;
                    if( p == std::string::npos ) break ;
                }
                if( s != std::string::npos )
                    l += tmp.substr( s, tmp.size() - s ) ;

                // remove whitespace at the end of every line
                {
                    size_t const p = l.find_last_not_of( ' ' ) ;
                    if( p < l.size()-1 )
                    {
                        l = l.substr(0, p+1 ) ;
                    }
                }
            }
        }

        motor::vector< motor::math::vec3f_t > positions ;
        motor::vector< motor::math::vec3f_t > normals ;
        motor::vector< motor::math::vec3f_t > texcoords ;
        // only valid if texcoords actually have values.
        size_t num_texcoord_elems = 3 ;

        motor::vector< motor::string_t > coord_lines ;

        // filter lines
        {
            auto tmp_lines = std::move( lines ) ;
            lines.reserve( tmp_lines.size() ) ;
            
            coord_lines.reserve( tmp_lines.size() ) ;

            size_t num_positions = 0 ;
            size_t num_texcoords = 0 ;
            size_t num_normals = 0 ;

            // filter lines and count 
            for ( auto & line : tmp_lines )
            {
                if ( line.size() < 2 ) continue ;

                if ( line[ 0 ] == 'v' && line[ 1 ] == 'n' )
                {
                    ++num_normals ;
                    coord_lines.emplace_back( std::move( line ) ) ;
                }
                else if ( line[ 0 ] == 'v' && line[ 1 ] == 't' )
                {
                    ++num_texcoords ;
                    coord_lines.emplace_back( std::move( line ) ) ;
                }
                else if ( line[ 0 ] == 'v' )
                {
                    ++num_positions ;
                    coord_lines.emplace_back( std::move( line ) ) ;
                }
                else if( line[0] == '#' )
                {}
                else if ( line.empty() )
                {}
                else if( line.size() >= 6 && line[0] == 'm' && line[1] == 't' && line[2] == 'l' && 
                    line[3] == 'l' && line[4] == 'i' && line[5] == 'b' )
                {
                    mtl_files.emplace_back( line.substr( 7 ) ) ;
                }
                else
                {
                    lines.emplace_back( std::move( line ) ) ;
                }
            }
            
            positions.reserve( num_positions ) ;
            normals.reserve( num_normals ) ;
            texcoords.reserve( num_texcoords ) ;
        }

        // issue material file loading
        {
            if( mtl_files.size() > 0 )
            {
                motor::log::global_t::warning("[Wavefront obj] : mtl files not supported yet.") ;
            }
            // load the files here and generate msl shaders in the end.
            // for the shader, the vertex layout is required.
        }

        // read out coords
        {
            auto read_out_vec3 = [] ( size_t s, motor::string_in_t line, size_t & num_elems )
            {
                float_t coords[ 3 ] = { 0.0f, 0.0f, 0.0f };

                size_t i = 0 ;
                size_t p = line.find_first_of( ' ', s ) ;
                while ( p != std::string::npos && i < 3 )
                {
                    coords[ i++ ] = std::stof( std::string( line.substr( s, p - s ) ) ) ;
                    s = p + 1 ;
                    p = line.find_first_of( ' ', s ) ;
                }

                if ( s < line.size() && i < 3 )
                {
                    coords[ i++ ] = std::stof( std::string( line.substr( s, line.size() - s ) ) ) ;
                }

                num_elems = i ;

                return motor::math::vec3f_t( coords[ 0 ], coords[ 1 ], coords[ 2 ] ) ;
            } ;

            for ( auto const & line : coord_lines )
            {
                size_t num_elems = 0 ;
                // normal
                if ( line[ 0 ] == 'v' && line[ 1 ] == 'n' )
                {
                    normals.emplace_back( read_out_vec3( 3, line, num_elems ) ) ;
                }
                // tex-coord
                else if ( line[ 0 ] == 'v' && line[ 1 ] == 't' )
                {
                    texcoords.emplace_back( read_out_vec3( 3, line, num_elems ) ) ;
                    num_texcoord_elems = std::min( num_elems, num_texcoord_elems ) ;
                }
                // position
                else if ( line[ 0 ] == 'v' )
                {
                    positions.emplace_back( read_out_vec3( 2, line, num_elems ) ) ;
                }
            }

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
                uint16_t pos_idx ;
                uint16_t tx_idx ;
                uint16_t nrm_idx ;
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
        // analyse object properties
        // name
        // material name
        // faces
        {
            mesh_data cur_data ;
            for ( auto const & l : lines )
            {
                if ( l[ 0 ] == 'o' || l[ 0 ] == 'g' )
                {
                    if( !cur_data.name.empty() )
                        meshes.emplace_back( std::move( cur_data ) ) ;

                    cur_data.name = l.substr( 2 ) ;
                    cur_data.faces.clear() ;
                }
                else if( l.size() >= 6 && 
                    l[0] == 'u' && l[1] == 's' && l[2] == 'e' && 
                    l[3] == 'm' && l[4] == 't' && l[5] == 'l' )
                {
                    cur_data.material = l.substr( 7, l.size() - 7 ) ;
                }
                else if( l[ 0 ] == 'f' )
                {
                    // f usually looks like this:
                    // pos/tx/nrm
                    // example:
                    // 1//1 or 1/2/3

                    mesh_data::face f ;

                    {
                        auto tripples = this_file::dissect_tripples( l ) ;
                        
                        for( auto const & t : tripples )
                        {
                            f.indices.emplace_back( mesh_data::index_tripple
                            {
                                t.pid,
                                t.nid,
                                t.tid
                            } ) ;
                        }
                        f.num_vertices = tripples.size() ;
                        //assert( f.num_vertices <= 8 ) ;
                    }
                    
                    cur_data.faces.emplace_back( std::move( f ) ) ;
                }
            }
            meshes.push_back( std::move( cur_data ) ) ;
        }

        // 1. test negative indices
        // 2. convert to zero-based index
        // @note negative indices would require to 
        // split up the whole importing process. 
        // Negative indices require to read the vertex
        // data from the file position the face index is 
        // negative. So this is not supported.
        {
            bool negative_index_detected = false ;

            for( auto & m : meshes ) 
            {
                for( auto & f : m.faces )
                {
                    for( auto & idx : f.indices )
                    {
                        if( idx.pos_idx > positions.size() ||
                            idx.tx_idx > texcoords.size() ||
                            idx.nrm_idx > normals.size() ) 
                        {
                            negative_index_detected = true ;
                            idx.pos_idx = 0 ;
                            idx.tx_idx = 0  ;
                            idx.nrm_idx = 0 ;
                        }

                        idx.pos_idx -= 1 ;
                        idx.tx_idx -= 1 ;
                        idx.nrm_idx -= 1 ;
                    }
                }
            }

            if( negative_index_detected )
            {
                motor::log::global_t::error( "[Wavefront obj] : negative"
                    " indices are not supported." ) ;
            }
        }

        {
            ret.geos.resize( meshes.size() ) ;
        }

        // make the polygon mesh
        {
            size_t midx = 0 ;
            for( auto & m : meshes )
            {
                motor::geometry::polygon_mesh pm ;
                pm.position_format = motor::geometry::vector_component_format::xyz ;
                pm.normal_format = pm.normals.size() == 0 ?  motor::geometry::vector_component_format::invalid : motor::geometry::vector_component_format::xyz ;
                pm.texcoord_format = motor::geometry::from_num_components( num_texcoord_elems ) ;

                pm.polygons.resize( m.faces.size() ) ;

                // count some numbers
                {
                    size_t i = 0;
                    size_t num_indices = 0 ;
                    for ( auto & f : m.faces )
                    {
                        pm.polygons[ i++ ] = f.num_vertices ;
                        num_indices += f.num_vertices ;
                    }

                    pm.indices.resize( num_indices ) ;
                    pm.positions.resize( positions.size() * 3 ) ;

                    if( normals.size() != 0 )
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

                // copy indices
                {
                    size_t poly_idx = 0 ;
                    size_t idx = 0 ;
                    for ( auto const & f : m.faces )
                    {
                        pm.polygons[poly_idx++] = f.num_vertices ;
                        for( auto & fidx : f.indices )
                        {
                            pm.indices[idx] = fidx.pos_idx ;
                            if ( normals.size() > 0 )
                                pm.normals_indices[ 0 ][ idx ] = fidx.nrm_idx ;
                            if ( texcoords.size() > 0 )
                                pm.texcoords_indices[ 0 ][ idx ] = fidx.tx_idx ;
                            ++idx ;
                        }
                    }
                }

                // copy positions
                {
                    {
                        size_t j = 0 ;
                        for ( auto const & v : positions )
                        {
                            pm.positions[j++] = v.x() ;
                            pm.positions[j++] = v.y() ;
                            pm.positions[j++] = v.z() ;
                        }
                    }

                    if( normals.size() != 0 )
                    {
                        size_t j = 0 ;
                        for ( auto const & v : normals )
                        {
                            pm.normals[0][ j++ ] = v.x() ;
                            pm.normals[0][ j++ ] = v.y() ;
                            pm.normals[0][ j++ ] = v.z() ;
                        }
                    }

                    if( texcoords.size() != 0 )
                    {
                        size_t j = 0 ;
                        for ( auto const & tx : texcoords )
                        {
                            pm.texcoords[0][j++] = tx.x() ;
                            pm.texcoords[0][j++] = tx.y() ;
                            if( num_texcoord_elems > 2 )
                                pm.texcoords[0][j++] = tx.z() ;
                        }
                    }
                }

                ret.geos[midx++].poly = std::move( pm ) ;
            }
        }

        ret.name = loc.as_string() ;

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
                motor::string_t name = ret.name + "." + m.name;
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
