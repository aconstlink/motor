
#include "cube.h"

#include "../mesh/polygon_mesh.h"
#include "../mesh/tri_mesh.h"
#include "../mesh/flat_line_mesh.h"

#include <motor/math/vector/vector2.hpp>

using namespace motor::geometry ;

/// one vertex position for each corner
motor::math::vec3f_t const positions[8] =
{
    // front
    motor::math::vec3f_t( -0.5f, -0.5f, 0.5f ),   // 0
    motor::math::vec3f_t( -0.5f, 0.5f, 0.5f ),    // 1
    motor::math::vec3f_t( 0.5f, 0.5f, 0.5f ),     // 2
    motor::math::vec3f_t( 0.5f, -0.5f, 0.5f ),    // 3
    // back
    motor::math::vec3f_t( -0.5f, -0.5f, -0.5f ),    // 4
    motor::math::vec3f_t( -0.5f, 0.5f, -0.5f ),     // 5
    motor::math::vec3f_t( 0.5f, 0.5f, -0.5f ),      // 6
    motor::math::vec3f_t( 0.5f, -0.5f, -0.5f )      // 7
} ;
size_t const g_num_positions = 8 ;

/// one normal for each face
motor::math::vec3f_t normals[6] =
{
    motor::math::vec3f_t( 0.0f, 0.0f, 1.0f ),
    motor::math::vec3f_t( 0.0f, 1.0f, 0.0f ),
    motor::math::vec3f_t( 0.0f, 0.0f, -1.0f ),
    motor::math::vec3f_t( 0.0f, -1.0f, 0.0f ),
    motor::math::vec3f_t( -1.0f, 0.0f, 0.0f ),
    motor::math::vec3f_t( 1.0f, 0.0f, 0.0f )
} ;
size_t const g_num_normals = 6 ;

motor::math::vec2f_t tex_0[4] =
{
    motor::math::vec2f_t( 0.0f, 0.0f ), // bottom left
    motor::math::vec2f_t( 0.0f, 1.0f ), // top left
    motor::math::vec2f_t( 1.0f, 1.0f ), // top right
    motor::math::vec2f_t( 1.0f, 0.0f ) // bottom right
} ;

size_t const g_num_texcoords_0 = 4 ;

size_t const g_num_tri_indices = 36 ;
size_t const g_num_quad_indices = 24 ;
size_t const g_num_line_indices = 48 ;

/// triangles
uint_t const position_tri_indices[36] = 
{
    0,1,2, 0,2,3,   // front
    1,5,6, 1,6,2,   // top
    5,4,7, 5,7,6,   // back
    4,0,3, 4,3,7,   // bottom
    4,5,1, 4,1,0,   // left
    3,2,6, 3,6,7    // right
} ;

/// triangles
uint_t normal_tri_indices[36] =
{
    0,0,0, 0,0,0,   // front
    1,1,1, 1,1,1,   // top
    2,2,2, 2,2,2,   // back
    3,3,3, 3,3,3,   // bottom
    4,4,4, 4,4,4,   // left
    5,5,5, 5,5,5    // right
} ;

/// triangles
uint_t const texcoord0_tri_indices[36] =
{
    0,1,2, 0,2,3,   // front
    1,0,3, 1,3,2,   // top
    0,1,2, 0,2,3,   // back
    1,0,3, 1,3,2,   // bottom
    1,0,1, 1,1,0,   // left
    3,2,3, 3,3,2    // right
} ;

/// quads
uint_t const position_quad_indices[24] = 
{
    0,1,2,3, // front
    1,5,6,2, // top
    7,6,5,4, // back    
    4,0,3,7, // bottom
    4,5,1,0, // left    
    3,2,6,7 // right
} ;

/// quads
uint_t const normal_quad_indices[24] =
{
    0,0,0,0,   // front
    1,1,1,1,   // top
    2,2,2,2,   // back
    3,3,3,3,   // bottom
    4,4,4,4,   // left
    5,5,5,5    // right
} ;

/// triangles
uint_t const texcoord0_quads_indices[24] =
{
    0,1,2,3,   // front
    0,1,2,3,   // top
    0,1,2,3,   // back
    0,1,2,3,   // bottom
    0,1,2,3,   // left
    0,1,2,3    // right
} ;

/// line
uint_t const position_line_indices[ 48 ] =
{
    0, 1, 1, 2, 2, 3, 3, 0, // front
    1, 5, 5, 6, 6, 2, 2, 1, // top
    7, 6, 6, 5, 5, 4, 4, 7, // back    
    4, 0, 0, 3, 3, 7, 7, 4, // bottom
    4, 5, 5, 1, 1, 0, 0, 4, // left    
    3, 2, 2, 6, 6, 7, 7, 3 // right
} ;

//*************************************************************************************
motor::geometry::result cube::make( polygon_mesh_ptr_t mesh_ptr, input_params_cref_t params_in ) 
{
    if( mesh_ptr == nullptr )
        return motor::geometry::invalid_argument ;

    polygon_mesh_ref_t amesh = *mesh_ptr ;

    // polygons
    {
        amesh.polygons.resize(6) ;

        for( size_t i=0; i<6; ++i )
            amesh.polygons[i] = 4 ;
    }

    // positions
    {
        amesh.position_format = motor::geometry::vector_component_format::xyz ;
        amesh.positions.resize(g_num_positions*3) ;
    
        for( size_t i=0; i<g_num_positions; ++i )
        {
            amesh.positions[i*3+0] = positions[i].x() * params_in.scale.x() ;
            amesh.positions[i*3+1] = positions[i].y() * params_in.scale.y() ;
            amesh.positions[i*3+2] = positions[i].z() * params_in.scale.z() ;
        }

        amesh.indices.resize( g_num_quad_indices ) ;
        for(size_t i = 0; i < g_num_quad_indices; ++i)
        {
            amesh.indices[i] = position_quad_indices[i] ;
        }
    }

    // normals
    {
        amesh.normal_format = motor::geometry::vector_component_format::xyz ;
        amesh.normals.resize(1) ;
        amesh.normals[0].resize(g_num_normals*3) ;

        for(size_t i = 0; i < g_num_normals; ++i)
        {
            amesh.normals[0][i * 3 + 0] = normals[i].x() ;
            amesh.normals[0][i * 3 + 1] = normals[i].y() ;
            amesh.normals[0][i * 3 + 2] = normals[i].z() ;
        }

        amesh.normals_indices.resize(1) ;
        amesh.normals_indices[0].resize( g_num_quad_indices ) ;

        for(size_t i = 0; i < g_num_quad_indices; ++i)
        {
            amesh.normals_indices[0][i] = normal_quad_indices[i] ;
        }
    }
    
    amesh.texcoord_format = motor::geometry::texcoord_component_format::uv ;
    amesh.texcoords.resize( 2 ) ;
    amesh.texcoords_indices.resize( 2 ) ;

    // texcoords 0
    {
        amesh.texcoords[0].resize( g_num_texcoords_0 * 2 ) ;        

        for(size_t i = 0; i < g_num_texcoords_0; ++i)
        {
            amesh.texcoords[0][i * 2 + 0] = tex_0[i].x() ;
            amesh.texcoords[0][i * 2 + 1] = tex_0[i].y() ;
        }
        
        amesh.texcoords_indices[0].resize( g_num_quad_indices ) ;
        for(size_t i = 0; i < g_num_quad_indices; ++i)
        {
            amesh.texcoords_indices[0][i] = texcoord0_quads_indices[i] ;
        }
    }

    // texcoords 1
    {
        amesh.texcoords[ 1 ].resize( g_num_texcoords_0 * 2 ) ;

        for( size_t i = 0; i < g_num_texcoords_0; ++i )
        {
            amesh.texcoords[ 1 ][ i * 2 + 0 ] = tex_0[ i ].y() ;
            amesh.texcoords[ 1 ][ i * 2 + 1 ] = tex_0[ i ].x() ;
        }

        amesh.texcoords_indices[ 1 ].resize( g_num_quad_indices ) ;
        for( size_t i = 0; i < g_num_quad_indices; ++i )
        {
            amesh.texcoords_indices[ 1 ][ i ] = texcoord0_quads_indices[ i ] ;
        }
    }

    return motor::geometry::ok ;
}

//*************************************************************************************
motor::geometry::result cube::make( tri_mesh_ptr_t mesh_ptr, input_params_cref_t params_in ) 
{
    if( mesh_ptr == nullptr )
        return motor::geometry::invalid_argument ;

    motor::geometry::tri_mesh_ref_t amesh = *mesh_ptr ;

    // positions
    {
        amesh.position_format = motor::geometry::vector_component_format::xyz ;
        amesh.positions.resize(g_num_positions*3) ;
    
        for( size_t i=0; i<g_num_positions; ++i )
        {
            amesh.positions[i*3+0] = positions[i].x() * params_in.scale.x() ;
            amesh.positions[i*3+1] = positions[i].y() * params_in.scale.y() ;
            amesh.positions[i*3+2] = positions[i].z() * params_in.scale.z() ;
        }

        amesh.indices.resize( g_num_tri_indices ) ;
        for(size_t i = 0; i < g_num_tri_indices; ++i)
        {
            amesh.indices[i] = position_tri_indices[i] ;
        }
    }

    // normals
    {
        amesh.normal_format = motor::geometry::vector_component_format::xyz ;
        amesh.normals.resize(1) ;
        amesh.normals[0].resize(g_num_normals*3) ;

        for(size_t i = 0; i < g_num_normals; ++i)
        {
            amesh.normals[0][i * 3 + 0] = normals[i].x() ;
            amesh.normals[0][i * 3 + 1] = normals[i].y() ;
            amesh.normals[0][i * 3 + 2] = normals[i].z() ;
        }

        amesh.normals_indices.resize(1) ;
        amesh.normals_indices[0].resize( g_num_tri_indices ) ;

        for(size_t i = 0; i < g_num_tri_indices; ++i)
        {
            amesh.normals_indices[0][i] = normal_tri_indices[i] ;
        }
    }

    amesh.texcoord_format = motor::geometry::texcoord_component_format::uv ;
    amesh.texcoords.resize( 2 ) ;
    amesh.texcoords_indices.resize( 2 ) ;

    // texcoords 0
    {
        amesh.texcoords[0].resize( g_num_texcoords_0 * 2 ) ;        

        for(size_t i = 0; i < g_num_texcoords_0; ++i)
        {
            amesh.texcoords[0][i * 2 + 0] = tex_0[i].x() ;
            amesh.texcoords[0][i * 2 + 1] = tex_0[i].y() ;
        }
        
        amesh.texcoords_indices[0].resize( g_num_tri_indices ) ;
        for(size_t i = 0; i < g_num_tri_indices; ++i)
        {
            amesh.texcoords_indices[0][i] = texcoord0_tri_indices[i] ;
        }
    }

    // texcoords 1
    {
        amesh.texcoords[ 1 ].resize( g_num_texcoords_0 * 2 ) ;

        for( size_t i = 0; i < g_num_texcoords_0; ++i )
        {
            amesh.texcoords[ 1 ][ i * 2 + 0 ] = tex_0[ i ].y() ;
            amesh.texcoords[ 1 ][ i * 2 + 1 ] = tex_0[ i ].x() ;
        }

        amesh.texcoords_indices[ 1 ].resize( g_num_tri_indices ) ;
        for( size_t i = 0; i < g_num_tri_indices; ++i )
        {
            amesh.texcoords_indices[ 1 ][ i ] = texcoord0_tri_indices[ i ] ;
        }
    }

    return motor::geometry::ok ;
}

//*************************************************************************************
motor::geometry::result cube::make( flat_line_mesh_ptr_t mesh_ptr, input_params_cref_t params_in )
{
    if( mesh_ptr == nullptr )
        return motor::geometry::invalid_argument ;

    motor::geometry::flat_line_mesh_ref_t amesh = *mesh_ptr ;

    // positions
    {
        amesh.position_format = motor::geometry::vector_component_format::xyz ;
        amesh.positions.resize( g_num_positions * 3 ) ;

        for( size_t i = 0; i < g_num_positions; ++i )
        {
            amesh.positions[ i * 3 + 0 ] = positions[ i ].x() * params_in.scale.x() ;
            amesh.positions[ i * 3 + 1 ] = positions[ i ].y() * params_in.scale.y() ;
            amesh.positions[ i * 3 + 2 ] = positions[ i ].z() * params_in.scale.z() ;
        }

        amesh.indices.resize( g_num_line_indices ) ;
        for( size_t i = 0; i < g_num_line_indices; ++i )
        {
            amesh.indices[ i ] = position_line_indices[ i ] ;
        }
    }

    return motor::geometry::ok ;
}
