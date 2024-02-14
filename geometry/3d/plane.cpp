
#include "plane.h"
#include "helper.h"
#include "../2d/helper.h"

#include "../mesh/polygon_mesh.h"
#include "../mesh/tri_mesh.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>

using namespace motor::geometry ;

//*************************************************************************************
motor::geometry::result plane::make( polygon_mesh_ptr_t mesh_ptr, input_params_cref_t ip ) 
{
    if( mesh_ptr == nullptr )
        return motor::geometry::invalid_argument ;

    size_t const per_dim_quads = ip.tess == 0 ? 1 : ip.tess ;

    size_t const num_quads = per_dim_quads * per_dim_quads ;

    size_t const per_dim_verts = per_dim_quads + 1 ;
    size_t const num_verts = per_dim_verts * per_dim_verts ;

    motor::geometry::polygon_mesh_ref_t m = *mesh_ptr ;

    // vertices
    {    
        this_t::data_t d ;
        d.qpd = per_dim_quads ;
        d.nv = num_verts ;
        d.vpd = per_dim_verts ; 
    
        this_t::construct_vertices( d, ip ) ;

        m.normals.resize(1) ;

        m.positions = std::move(d.positions) ;
        m.normals[0] = std::move(d.normals) ;
        m.texcoords = std::move(d.texcoords) ;

        m.position_format = motor::geometry::vector_component_format::xyz ;
        m.normal_format = motor::geometry::vector_component_format::xyz ;
        m.texcoord_format = motor::geometry::texcoord_component_format::uv ;
    }

    // polygons
    {
        m.polygons.resize( num_quads ) ;

        for(size_t i = 0; i < num_quads; ++i)
            m.polygons[i] = 4 ;
    }

    // position indices
    {
        size_t const num_indices = num_quads * 4 ;
        m.indices.resize( num_indices ) ;
        
        m.texcoords_indices.resize(1) ;
        m.texcoords_indices[0].resize( num_indices ) ;

        for(size_t q = 0; q < num_quads; ++q)
        {
            size_t const a = q / per_dim_quads ;
            size_t const start_i = q + a ;

            size_t const i = q * 4 ;

            m.indices[i + 0] = uint_t( start_i + 0 ) ;
            m.indices[i + 1] = uint_t( start_i + 0 + per_dim_verts ) ;
            m.indices[i + 2] = uint_t( start_i + 1 + per_dim_verts ) ;
            m.indices[i + 3] = uint_t( start_i + 1 ) ;

            m.texcoords_indices[0][i+0] = uint_t( start_i + 0 ) ;
            m.texcoords_indices[0][i+1] = uint_t( start_i + 0 + per_dim_verts ) ;
            m.texcoords_indices[0][i+2] = uint_t( start_i + 1 + per_dim_verts ) ;
            m.texcoords_indices[0][i+3] = uint_t( start_i + 1 ) ;
        }
    }

    // normal indices
    {
        size_t const num_indices = num_quads * 4 ;
        m.normals_indices.resize(1) ;
        m.normals_indices[0].resize( num_indices ) ;

        for(size_t q = 0; q < num_quads; ++q)
        {
            size_t const i = q * 4 ;

            m.normals_indices[0][i + 0] = uint_t( 0 ) ;
            m.normals_indices[0][i + 1] = uint_t( 0 ) ;
            m.normals_indices[0][i + 2] = uint_t( 0 ) ;
            m.normals_indices[0][i + 3] = uint_t( 0 ) ;
        }
    }

    return motor::geometry::ok ;
}

//*************************************************************************************
void_t plane::construct_vertices( data_ref_t d, input_params_cref_t ip ) 
{
    size_t const tess = d.qpd ;
    size_t const per_dim_verts = d.vpd ;
    size_t const num_verts = d.nv ;

    // vertices
    {
        d.positions.resize( num_verts * 3 ) ;                

        d.texcoords.resize( 1 ) ;
        d.texcoords[0].resize( num_verts * 2 ) ;

        float const start_x = -0.5f ;
        float const start_y = -0.5f ;

        float const step = 1.0f / float( tess ) ;

        size_t index = 0 ;

        for(size_t iy = 0; iy < per_dim_verts; ++iy)
        {
            float const y = start_y + float( iy ) * step ;
            float const ty = float( iy ) * step ;
            for(size_t ix = 0; ix < per_dim_verts; ++ix)
            {
                float const x = start_x + float( ix ) * step ;
                float const tx = float( ix ) * step ;

                motor::math::vec3f_t v{ x*ip.scale.x(), y*ip.scale.y(), 0.0f } ;                
                motor::math::vec2f_t t{ tx, ty } ;

                size_t const i3d = index * 3 ;
                size_t const i2d = index * 2 ;

                helper_3d::vector_to_array( i3d, v, d.positions ) ;                
                helper_2d::vector_to_array( i2d, t, d.texcoords[0] ) ;

                ++index ;
            }
        }
    }

    // normals
    {
        d.normals.resize( 1 * 3 ) ;

        motor::math::vec3f_t n{ 0.0f, 0.0f, -1.0f } ;
        helper_3d::vector_to_array( 0, n, d.normals ) ;
    }
}
