
#include "tetra.h"

#include "../mesh/polygon_mesh.h"

using namespace motor::geometry ;

//*************************************************************************************
motor::geometry::result tetra::make( polygon_mesh_ptr_t m_out_ptr, input_params_cref_t ip ) 
{
    if( m_out_ptr == nullptr )
        return motor::geometry::invalid_argument ;

    motor::geometry::polygon_mesh m ;
    
    m.position_format = motor::geometry::vector_component_format::xyz ;
    m.normal_format = motor::geometry::vector_component_format::xyz ;
    m.texcoord_format = motor::geometry::texcoord_component_format::uv ;

    motor::math::vec3f_t const c_positions[] =
    {
        motor::math::vec3f_t{ -1.0f, -1.0f, -1.0f }, // bottom left
        motor::math::vec3f_t{ 1.0f, -1.0f, -1.0f }, // bottom right
        motor::math::vec3f_t{ 0.0f, -1.0f, 1.0f }, // bottom back        
        motor::math::vec3f_t{ 0.0f, 1.0f, 0.0f } // tip
    } ;

    motor::math::vec3f_t const c_normals[] =
    {
        motor::math::vec3f_t{ -0.5f, 0.5f, 0.5f }, // left
        motor::math::vec3f_t{ 0.5f, 0.5f, 0.5f }, // right
        motor::math::vec3f_t{ 0.0f, 0.5f, -0.5f }, // front
        motor::math::vec3f_t{ 0.0f, -1.0f, 0.0f } // bottom
    } ;

    motor::math::vec2f_t const c_tex[] =
    {
        motor::math::vec2f_t{ +0.0f, +0.0f }, // front 
        motor::math::vec2f_t{ +1.0f, +0.0f }, // front 
        motor::math::vec2f_t{ +0.0f, +1.0f } // front 

    } ;

    // vertices
    {
        m.positions.resize( 12 ) ;
        m.normals.resize( 1 ) ;
        m.normals[0].resize( 12 ) ;
        m.texcoords.resize( 1 ) ;
        m.texcoords[0].resize( 12 ) ;


        for( size_t i = 0; i < 4; ++i )
        {
            size_t const i3 = i * 3 ;
            size_t const i2 = i * 2 ;

            {
                m.positions[i3 + 0] = c_positions[i].x() ;
                m.positions[i3 + 1] = c_positions[i].y() ;
                m.positions[i3 + 2] = c_positions[i].z() ;
            }
            {
                m.normals[0][i3 + 0] = c_normals[i].x() ;
                m.normals[0][i3 + 1] = c_normals[i].y() ;
                m.normals[0][i3 + 2] = c_normals[i].z() ;
            }
            {
                m.texcoords[0][i2 + 0] = c_tex[i].x() ;
                m.texcoords[0][i2 + 1] = c_tex[i].y() ;
            }
        }
    }

    // indices
    {
        struct vertex
        {
            uint_t pos_i ;
            uint_t nrm_i ;
            uint_t tex_i ;
        };

        std::vector<vertex> vertices( 24 ) ;

        // bottom
        vertices[0] = vertex{ 0, 3, 0 } ;
        vertices[1] = vertex{ 1, 3, 1 } ;
        vertices[2] = vertex{ 2, 3, 2 } ;

        // front
        vertices[3] = vertex{ 0, 2, 0 } ;
        vertices[4] = vertex{ 1, 2, 1 } ;
        vertices[5] = vertex{ 3, 2, 2 } ;

        // left
        vertices[6] = vertex{ 2, 0, 0 } ;
        vertices[7] = vertex{ 0, 0, 1 } ;
        vertices[8] = vertex{ 3, 0, 2 } ;

        // right
        vertices[9] = vertex{ 1, 1, 0 } ;
        vertices[10] = vertex{ 2, 1, 1 } ;
        vertices[11] = vertex{ 3, 1, 2 } ;

        // polygons
        {
            m.polygons.resize( 4 ) ;
            m.polygons[0] = 3 ;
            m.polygons[1] = 3 ;
            m.polygons[2] = 3 ;
            m.polygons[3] = 3 ;
        }

        // positions indices
    {
        m.indices.resize( 12 ) ;

        for( size_t i = 0; i < 12; ++i )
        {
            m.indices[i] = vertices[i].pos_i ;
        }
    }

    // normal indices
    {
        m.normals_indices.resize( 1 ) ;
        m.normals_indices[0].resize( 12 ) ;

        for( size_t i = 0; i < 12; ++i )
        {
            m.normals_indices[0][i] = vertices[i].nrm_i ;
        }
    }

    // texcoords indices
    {
        m.texcoords_indices.resize( 1 ) ;
        m.texcoords_indices[0].resize( 12 ) ;

        for( size_t i = 0; i < 12; ++i )
        {
            m.texcoords_indices[0][i] = vertices[i].tex_i ;
        }
    }

    }

    *m_out_ptr = std::move( m ) ;

    return motor::geometry::ok ;
}
