

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

        struct mesh_data
        {
            struct face_indices
            {
                uint16_t pos_idx[ 4 ] = { 0, 0, 0, 0 };
                uint16_t tx_idx[ 4 ] = { 0, 0, 0, 0 };
                uint16_t nrm_idx[ 4 ] = { 0, 0, 0, 0 };
            };

            struct face
            {
                byte_t num_vertices = 0 ;
                face_indices indices ;
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
                if ( l[ 0 ] == 'o' )
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

                    byte_t vtx_idx =  0 ;
                    
                    size_t s = 2 ;
                    size_t p = l.find_first_of( ' ', s ) ;

                    while( true )
                    {
                        uint16_t * dst = reinterpret_cast<uint16_t*>( &f.indices ) + (vtx_idx++) ;

                        motor::string_t tripple = l.substr( s, p - s ) ;

                        size_t s0 = 0 ;
                        size_t p0 = tripple.find_first_of( '/', s0 ) ;
                        while( p0 != std::string::npos )
                        {
                            auto const number = std::string( tripple.substr( s0, p0 - s0 ) ) ;
                            if( !number.empty() )
                            {
                                uint16_t idx = std::stol( number ) ;
                                *dst = idx ;
                            }
                            
                            dst += 4 ;
                            s0 = p0 + 1 ;
                            p0 = tripple.find_first_of( '/', s0 ) ;
                        }

                        {
                            auto const number = std::string( tripple.substr( s0, p0 - s0 ) ) ;
                            if( !number.empty() )
                            {
                                uint16_t const idx = std::stol( number ) ;
                                
                                *dst = idx ;
                            }
                        }
                        //dst = reinterpret_cast<uint16_t*>( &f.indices ) + (vtx_idx++) ;
                        
                        s = p + 1 ;
                        p = l.find_first_of( ' ', s ) ;

                        if( p == std::string::npos )
                        {
                            if( s == l.size() + 1 ) break ;
                            p = l.size() ;
                        }

                        if( vtx_idx == 5 ) 
                        {
                            motor::log::global_t::error("[Wavefront obj] : can not import "
                                "more than 4 vertices per primitive.") ;
                            break ;
                        }
                    }
                    f.num_vertices = vtx_idx ;
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
                    for( size_t i=0 ;i<4; ++i )
                    {
                        if( f.indices.pos_idx[i] > positions.size() ||
                            f.indices.tx_idx[i] > texcoords.size() ||
                            f.indices.nrm_idx[i] > normals.size() ) 
                        {
                            negative_index_detected = true ;
                            f.indices.pos_idx[ i ] = 0 ;
                            f.indices.tx_idx[ i ] = 0  ;
                            f.indices.nrm_idx[ i ] = 0 ;
                        }

                        f.indices.pos_idx[ i ] -= 1 ;
                        f.indices.tx_idx[ i ] -= 1 ;
                        f.indices.nrm_idx[ i ] -= 1 ;
                    }
                }
            }

            if( negative_index_detected )
            {
                motor::log::global_t::error( "[Wavefront obj] : negative"
                    " indices are not supported." ) ;
            }
        }

        // make the polygon mesh
        {
            #if 1
            for( auto & m : meshes )
            {
                motor::geometry::polygon_mesh pm ;
                pm.position_format = motor::geometry::vector_component_format::xyz ;
                pm.normal_format = motor::geometry::vector_component_format::xyz ;
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
                        for( size_t i = 0 ; i<f.num_vertices; ++i )
                        {
                            pm.indices[idx] = f.indices.pos_idx[i] ;
                            if( normals.size() > 0 )
                                pm.normals_indices[0][idx] = f.indices.nrm_idx[i] ;
                            if( texcoords.size() > 0 )
                                pm.texcoords_indices[0][idx] = f.indices.tx_idx[i] ;
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

                ret.poly = std::move( pm ) ;
                break ;
            }
            #if 0
            for ( auto & m : meshes )
            {
                // copy vertices
                {

                }

                // copy indices
                {
                    size_t j = 0 ;
                    for ( auto & f : m.faces )
                    {
                        for ( size_t i = 0 ; i < f.num_vertices; ++i )
                        {
                            pm.indices[j++] = f.indices.[i] ;
                        }
                    }
                }
                
            }
            #endif
            #else
            {
                motor::geometry::polygon_mesh_t poly ;
                motor::geometry::cube_t::input_params const ip = 
                {

                } ;
                motor::geometry::cube_t::make( &poly, std::move( ip ) )  ;
                ret.poly = std::move( poly ) ;
            }

            #endif
            
        }

        // put together a shader per material
        // or maybe just some shading infos?
        {
            bool_t const has_pos = positions.size() != 0 ;
            bool_t const has_nrm = normals.size() != 0 ;
            bool_t const has_tx = texcoords.size() != 0 ;

            if( has_tx )
            {
                //texcoords[0] ;
            }

            auto print_ln = []( size_t const indent, motor::string_in_t ln, motor::string_ref_t shader )
            {
                for( size_t i=0; i<indent<<2; ++i ) shader+= " " ;
                shader += ln + "\n" ;
            } ;

            size_t indent = 0 ;
            motor::string_t shader ;
            
            print_ln( indent, "config " + loc.as_string() , shader ) ;
            print_ln( indent, "{" , shader ) ;
            ++indent ;

            // vertex shader
            {
                print_ln( indent, "vertex_shader", shader ) ;
                print_ln( indent, "{" , shader ) ;
                ++indent ;
                
                
                print_ln( indent, "in vec3_t pos : position ;" , shader ) ;
                print_ln( indent, "in vec3_t nrm : normal ;" , shader ) ;

                if ( has_tx )
                    print_ln( indent, "in vec3_t tx : texcoord0 ;" , shader ) ;

                print_ln( indent, "" , shader ) ;
                print_ln( indent, "out vec4_t pos : position ;" , shader ) ;
                print_ln( indent, "out vec3_t nrm : normal ;" , shader ) ;

                if ( has_tx )
                    print_ln( indent, "out vec3_t tx : texcoord0 ;" , shader ) ;

                print_ln( indent, "" , shader ) ;
                print_ln( indent, "mat4_t world : world ;", shader ) ;
                print_ln( indent, "mat4_t view : view ;" , shader ) ;
                print_ln( indent, "mat4_t proj : proj ;" , shader ) ;
                print_ln( indent, "" , shader ) ;
                print_ln( indent, "void main()" , shader ) ;
                print_ln( indent, "{" , shader ) ;
                ++indent ;
                {
                    print_ln( indent, "out.nrm = in.nrm ;" , shader ) ;
                    print_ln( indent, "out.pos = proj * view * world * vec4_t( in.pos, 1.0 ) ;" , shader ) ;
                }
                --indent ;
                print_ln( indent, "}" , shader ) ;
                

                --indent ;
                print_ln( indent, "}" , shader ) ;
            }

            {
                print_ln( indent, "pixel_shader", shader ) ;
                print_ln( indent, "{", shader ) ;
                ++indent ;

                print_ln( indent, "in vec3_t nrm : normal ;", shader ) ;

                if ( has_tx )
                    print_ln( indent, "in vec3_t tx : texcoord0 ;", shader ) ;

                print_ln( indent, "vec3_t light_dir ;", shader ) ;
                print_ln( indent, "out vec4_t color : color ;", shader ) ;

                print_ln( indent, "void main()", shader ) ;
                print_ln( indent, "{", shader ) ;
                ++indent ;
                {
                    print_ln( indent, "float_t ndl = dot( in.nrm, light_dir ) ;", shader ) ;
                    print_ln( indent, "out.color = vec4_t(ndl, ndl, ndl, 1.0 ) ;", shader ) ;
                }
                --indent ;
                print_ln( indent, "}", shader ) ;
                --indent ;
                print_ln( indent, "}" , shader ) ;
            }

            --indent ;
            print_ln( indent, "}" , shader ) ;

            ret.shader = std::move( shader ) ;
        }

        ret.name = loc.as_string() ;

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