

#include "geometry_object.h"

#include <motor/log/global.h>

using namespace motor::graphics ;

//**********************************************************************************************
geometry_object::this_t geometry_object::create( motor::string_in_t name, motor::geometry::polygon_mesh_in_t mesh ) noexcept 
{
    motor::geometry::flat_tri_mesh_t tm ;
    mesh.flatten( tm ) ;

    motor::graphics::vertex_buffer_t vb ;
    motor::graphics::index_buffer_t ib ;

    // create vertex layout
    {
        // determine position layout
        {
            motor::graphics::type_struct ts = motor::graphics::type_struct::undefined ;
            switch ( tm.position_format )
            {
            case motor::geometry::vector_component_format::xy: ts = motor::graphics::type_struct::vec2 ; break ;
            case motor::geometry::vector_component_format::xyz: ts = motor::graphics::type_struct::vec3 ; break ;
            case motor::geometry::vector_component_format::xyzw: ts = motor::graphics::type_struct::vec4 ; break ;
            default: break ;
            }

            if ( ts == motor::graphics::type_struct::undefined ) 
            {
                motor::log::global::error( "[motor::graphics::geometry_object] : mesh needs at least position data" ) ;
                return this_t() ;
            }

            vb.add_layout_element( motor::graphics::vertex_attribute::position, motor::graphics::type::tfloat, ts ) ;
        }

        // determine normal layout
        {
            motor::graphics::type_struct ts = motor::graphics::type_struct::undefined ;
            switch ( tm.normal_format )
            {
            case motor::geometry::vector_component_format::xy: ts = motor::graphics::type_struct::vec2 ; break ;
            case motor::geometry::vector_component_format::xyz: ts = motor::graphics::type_struct::vec3 ; break ;
            case motor::geometry::vector_component_format::xyzw: ts = motor::graphics::type_struct::vec4 ; break ;
            default: break ;
            }

            if ( ts != motor::graphics::type_struct::undefined && tm.get_num_normals() != 0 ) 
                vb.add_layout_element( motor::graphics::vertex_attribute::normal, motor::graphics::type::tfloat, ts ) ;
        }

        // determine texture coordinate layout
        {
            motor::graphics::type_struct ts = motor::graphics::type_struct::undefined ;
            switch ( tm.texcoord_format )
            {
            case motor::geometry::texcoord_component_format::uv: ts = motor::graphics::type_struct::vec2 ; break ;
            case motor::geometry::texcoord_component_format::uvw: ts = motor::graphics::type_struct::vec3 ; break ;
            default: break ;
            }

            if ( ts != motor::graphics::type_struct::undefined )
            {
                for( size_t i=0; i<tm.texcoords.size(); ++i )
                {
                    if( tm.get_num_texcoords( i ) == 0 ) break ;

                    auto const va = motor::graphics::vertex_attribute( size_t( motor::graphics::vertex_attribute::texcoord0 ) + i ) ;
                    vb.add_layout_element( va, motor::graphics::type::tfloat, ts ) ;
                }
            }
        }
    }

    // create vertex buffer and fill data
    {
        vb.resize( tm.get_num_vertices() ) ;

        {
            // size in float per vertex
            //size_t const sif = vb.get_layout_sib() / sizeof( float_t ) ;

            vb.update<float_t>( [&] ( float_t * array, size_t const ne )
            {
                for( size_t v=0; v<tm.get_num_vertices(); ++v )
                {
                    // positions
                    {
                        size_t const comps = motor::geometry::vector_component_format_to_number( tm.position_format ) ;
                        size_t const idx = v * comps ;
                        for( size_t i=0; i<comps; ++i )
                        {
                            array[i] = tm.positions[idx + i] ;
                        }
                        array += comps ;
                    }

                    // normals
                    {
                        size_t const comps = motor::geometry::vector_component_format_to_number( tm.normal_format ) ;
                        size_t const idx = v * comps ;
                        for ( size_t i = 0; i < comps; ++i )
                        {
                            array[ i ] = tm.normals[ idx + i ] ;
                        }
                        array += comps ;
                    }

                    // texcoords
                    {
                        size_t const comps = motor::geometry::texcoords_component_format_to_number( tm.texcoord_format ) ;
                        size_t const idx = v * comps ;
                        for( size_t t=0; t<2; ++t )
                        {
                            if( tm.get_num_texcoords( t ) == 0 ) break ;

                            for ( size_t i = 0; i < comps; ++i )
                            {
                                array[ i ] = tm.texcoords[t][ idx + i ] ;
                            }
                            array += comps ;
                        }
                    }
                }
            } );
        }
    }

    // create index buffer and fill data
    {
        ib.set_layout_element( motor::graphics::type::tuint ) ;
        ib.resize( tm.indices.size() ).update<uint_t>( [&] ( uint_t * array, size_t const ne )
        {
            for( size_t i=0; i<tm.indices.size(); ++i )
            {
                array[i] = tm.indices[i] ;
            }
        } ) ;
    }


    return this_t( name, motor::graphics::primitive_type::triangles, std::move( vb ), std::move( ib ) ) ;
}