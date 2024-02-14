
#include "flat_tri_mesh.h"

using namespace motor::geometry ;

//*******************************************************************
flat_tri_mesh::flat_tri_mesh( void_t ) 
{
}

//*******************************************************************
flat_tri_mesh::flat_tri_mesh( this_cref_t rhv ) 
{
    indices = rhv.indices ;

    positions = rhv.positions ;
    texcoords = rhv.texcoords ;
    normals = rhv.normals ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;
}

//*******************************************************************
flat_tri_mesh::flat_tri_mesh( this_rref_t rhv )
{
    indices = std::move( rhv.indices ) ;

    positions = std::move( rhv.positions ) ;
    texcoords = std::move( rhv.texcoords ) ;
    normals = std::move( rhv.normals ) ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;
}

//*******************************************************************
flat_tri_mesh::~flat_tri_mesh( void_t )
{
}

//*******************************************************************
flat_tri_mesh::this_ref_t flat_tri_mesh::operator = ( this_rref_t rhv ) 
{
    indices = rhv.indices ;

    positions = rhv.positions ;
    texcoords = rhv.texcoords ;
    normals = rhv.normals ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;

    return *this ;
}

//*******************************************************************
flat_tri_mesh::this_ref_t flat_tri_mesh::operator = ( this_cref_t rhv ) 
{
    indices = std::move( rhv.indices ) ;

    positions = std::move( rhv.positions ) ;
    texcoords = std::move( rhv.texcoords ) ;
    normals = std::move( rhv.normals ) ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;

    return *this ;
}

//*******************************************************************
bool_t flat_tri_mesh::are_array_numbers_valid( void_t ) const 
{
    size_t const num_components = motor::geometry::vector_component_format_to_number( position_format ) ;

    if( num_components == 0 ) 
        return false ;

    size_t const num_position_vertices = positions.size() / num_components ;
        
    if( normals.size() > 0 )
    {
        size_t const num_normal_components = motor::geometry::vector_component_format_to_number( normal_format ) ;
        if(num_components == 0)
            return false ;

        size_t const num_normal_vertices = normals.size() / num_normal_components ;
        if( num_position_vertices != num_normal_vertices ) 
            return false ;
    }

    if( texcoords.size() > 0 )
    {
        size_t texcoord_components = motor::geometry::texcoords_component_format_to_number( texcoord_format ) ;
        for( size_t i=0; i<texcoords.size(); ++i )
        {
            size_t const num_texcoords_vertices = texcoords[i].size()/texcoord_components ;
            if( num_texcoords_vertices != num_position_vertices )
                return false ;
        }
    }

    return true ;
}

//*******************************************************************
size_t flat_tri_mesh::get_num_vertices( void_t ) const 
{
    size_t const num_components = motor::geometry::vector_component_format_to_number( position_format ) ;
    if( num_components == 0 ) 
        return 0 ;

    return positions.size() / num_components ;
}

//*******************************************************************
size_t flat_tri_mesh::get_num_positions( void_t ) const
{
    size_t const num_components = motor::geometry::vector_component_format_to_number( position_format ) ;
    if( num_components == 0 )
        return 0 ;

    return positions.size() / num_components ;
}

//*******************************************************************
size_t flat_tri_mesh::get_num_normals( void_t ) const
{
    size_t const num_components = motor::geometry::vector_component_format_to_number( normal_format ) ;
    if( num_components == 0 )
        return 0 ;

    return normals.size() / num_components ;
}

//*******************************************************************
size_t flat_tri_mesh::get_num_texcoords( size_t const l ) const
{
    size_t const num_components = motor::geometry::texcoords_component_format_to_number( texcoord_format) ;
    if( num_components == 0 )
        return 0 ;

    return texcoords[l].size() / num_components ;
}

//*******************************************************************
motor::math::vec3f_t flat_tri_mesh::get_vertex_position_3d( size_t const i ) const noexcept 
{
    size_t const index = i * 3 ;

    if( index+2 >= positions.size() ) return motor::math::vec3f_t( 0.0f ) ;
    return motor::math::vec3f_t( positions[ index + 0 ], positions[ index + 1 ], positions[ index + 2 ] ) ;
}

//*******************************************************************
motor::math::vec3f_t flat_tri_mesh::get_vertex_normal_3d( size_t const i ) const noexcept 
{
    size_t const index = i * 3 ;
    
    if( i+2 >= normals.size() ) return motor::math::vec3f_t( 0.0f ) ;
    return motor::math::vec3f_t( normals[ index + 0 ], normals[ index + 1 ], normals[ index + 2 ] ) ;
}

//*******************************************************************
motor::math::vec2f_t flat_tri_mesh::get_vertex_texcoord( size_t const l, size_t const i ) const noexcept 
{
    size_t const index = i * 2 ;

    if( l >= texcoords.size() || index+2 >= texcoords[ l ].size() ) return motor::math::vec2f_t() ;
    return motor::math::vec2f_t( texcoords[ l ][ index + 0 ], texcoords[ l ][ index + 1 ] ) ;
}
