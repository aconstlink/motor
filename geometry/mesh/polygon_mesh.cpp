



#include "polygon_mesh.h"

#include <motor/log/global.h>

using namespace motor::geometry ;

//*******************************************************************
polygon_mesh::polygon_mesh( void_t ) 
{

}

//*******************************************************************
polygon_mesh::polygon_mesh( this_cref_t rhv ) 
{
    *this = rhv ;
}

//*******************************************************************
polygon_mesh::polygon_mesh( this_rref_t rhv )
{
    *this = std::move( rhv ) ;
}

//*******************************************************************
polygon_mesh::~polygon_mesh( void_t )
{

}

//*******************************************************************
polygon_mesh::this_ref_t polygon_mesh::operator = ( this_cref_t rhv )
{
    polygons = rhv.polygons ;
    indices = rhv.indices ;
    positions = rhv.positions ;
    normals_indices = rhv.normals_indices ;
    normals = rhv.normals ;
    texcoords_indices = rhv.texcoords_indices ;
    texcoords = rhv.texcoords ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;

    return *this ;
}

//*******************************************************************
polygon_mesh::this_ref_t polygon_mesh::operator = ( this_rref_t rhv )
{
    polygons = std::move( rhv.polygons ) ;
    indices = std::move( rhv.indices ) ;
    positions = std::move( rhv.positions ) ;
    normals_indices = std::move( rhv.normals_indices ) ;
    normals = std::move( rhv.normals ) ;
    texcoords_indices = std::move( rhv.texcoords_indices ) ;
    texcoords = std::move( rhv.texcoords ) ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;

    return *this ;
}

//*******************************************************************
void_t polygon_mesh::triangulate_index_buffer( size_t num_triangles, 
            uints_cref_t indices_in, uints_ref_t indices_out ) const
{
    indices_out.resize( num_triangles * 3 ) ;

    size_t start_index = 0 ;
    size_t other_index = 0 ;

        
    for( size_t i=0; i<polygons.size(); ++i )
    {
        uint_t const vi_0 = indices_in[start_index + 0] ;

        size_t const num_tris = polygons[i]-2 ;

        // for each triangle in the current polygon
        for( size_t tri=0; tri<num_tris; ++tri )
        {            
            uint_t const vi_1 = indices_in[start_index + tri + 1] ;
            uint_t const vi_2 = indices_in[start_index + tri + 2] ;

            indices_out[other_index+0] = vi_0 ;
            indices_out[other_index+1] = vi_1 ;
            indices_out[other_index+2] = vi_2 ;
            other_index += 3 ;
        }        

        start_index += polygons[i] ;
    }
}

//*******************************************************************
motor::geometry::result polygon_mesh::flatten( tri_mesh_ref_t mesh_out ) const
{
    tri_mesh the_mesh ;

    the_mesh.position_format = this_t::position_format ;
    the_mesh.normal_format = this_t::normals.size() != 0 ? this_t::normal_format : vector_component_format::invalid ;
    the_mesh.texcoord_format = texcoords.size() != 0 ? this_t::texcoord_format : texcoord_component_format::invalid ;
    
    // check some values
    {
    }   

    size_t num_triangles = 0 ;

    // count required indices for the triangle mesh
    // num_tris = poly_vertices - 2
    {
        for( size_t i=0; i<polygons.size(); ++i )
        {
            num_triangles += polygons[i] - 2 ;
        }
    }

    // do the position index
    {
        this_t::triangulate_index_buffer( num_triangles, 
            this_t::indices, the_mesh.indices ) ;
    }

    // copy positions
    {
        the_mesh.positions.resize( this_t::positions.size() ) ;
    
        for( size_t i=0; i<this_t::positions.size(); ++i )
        {
            the_mesh.positions[i] = this_t::positions[i] ;
        }
    }

    // copy normals
    if( this_t::normals.size() > 0 )
    {
        the_mesh.normals.resize( this_t::normals.size() ) ;
        for( size_t l=0; l<the_mesh.normals.size(); ++l )
        {
            the_mesh.normals[l].resize( this_t::normals[l].size() ) ;

            for( size_t i=0; i<the_mesh.normals[l].size(); ++i )
            {
                the_mesh.normals[l][i] = this_t::normals[l][i] ;
            }
        }
    }

    // do normal indices
    if( this_t::normals_indices.size() )
    {
        the_mesh.normals_indices.resize( this_t::normals_indices.size() ) ;
        for( size_t l=0; l<the_mesh.normals_indices.size(); ++l )
        {
            this_t::triangulate_index_buffer( num_triangles, 
                this_t::normals_indices[l], the_mesh.normals_indices[l] ) ;
        }

    }

    // copy texcoords
    if(this_t::texcoords.size() > 0)
    {
        the_mesh.texcoords.resize( this_t::texcoords.size() ) ;
        for(size_t l = 0; l < the_mesh.texcoords.size(); ++l)
        {
            the_mesh.texcoords[l].resize( this_t::texcoords[l].size() ) ;

            for(size_t i = 0; i < the_mesh.texcoords[l].size(); ++i)
            {
                the_mesh.texcoords[l][i] = this_t::texcoords[l][i] ;
            }
        }
    }

    // do texcoord indices
    if(this_t::texcoords_indices.size())
    {
        the_mesh.texcoords_indices.resize( this_t::texcoords_indices.size() ) ;
        for(size_t l = 0; l < the_mesh.texcoords_indices.size(); ++l)
        {
            this_t::triangulate_index_buffer( num_triangles,
                this_t::texcoords_indices[l], the_mesh.texcoords_indices[l] ) ;
        }

    }

    mesh_out = std::move( the_mesh ) ;

    return motor::geometry::ok ;
}

//*******************************************************************
motor::geometry::result polygon_mesh::flatten( flat_tri_mesh_ref_t mesh_out ) const
{
    tri_mesh_t mesh_a ;

    this_t::flatten( mesh_a ) ;
    mesh_a.flatten( mesh_out ) ;

    return motor::geometry::ok ;
}

//*******************************************************************
polygon_mesh::this_t polygon_mesh::repeat( size_t const times ) const 
{
    this_t m = *this ;
        
    // repeat polygons
    {
        motor::geometry::uints_t old_polys = std::move( m.polygons ) ;
        motor::geometry::uints_t new_polys( times * old_polys.size() ) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_polys.size() ;
            for( size_t j=0; j<old_polys.size(); ++j )
            {
                new_polys[base_index+j] = old_polys[j] ;
            }
        }
        m.polygons = std::move( new_polys ) ;
    }
    
    // repeat position indices
    {
        motor::geometry::uints_t old_indices = std::move( m.indices ) ;
        motor::geometry::uints_t new_indices( times * old_indices.size() ) ;

        for( size_t i=0; i<times; ++i )
        {
            size_t const base_index = i * old_indices.size() ;

            for( size_t j=0; j<old_indices.size(); ++j )
            {
                new_indices[base_index+j] = old_indices[j] ;
            }
        }
        m.indices = std::move( new_indices ) ;
    }

    // repeat normal indices
    for( size_t k=0; k<m.normals_indices.size(); ++k )
    {
        motor::geometry::uints_t old_indices = std::move( m.normals_indices[k] ) ;
        motor::geometry::uints_t new_indices( times * old_indices.size() ) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_indices.size() ;

            for( size_t j = 0; j < old_indices.size(); ++j )
            {
                new_indices[base_index + j] = old_indices[j] ;
            }
        }
        m.normals_indices[k] = std::move( new_indices ) ;
    }

    // repeat texcoord indices
    for( size_t k = 0; k < m.texcoords_indices.size(); ++k )
    {
        motor::geometry::uints_t old_indices = std::move( m.texcoords_indices[k] ) ;
        motor::geometry::uints_t new_indices( times * old_indices.size() ) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_indices.size() ;

            for( size_t j = 0; j < old_indices.size(); ++j )
            {
                new_indices[base_index + j] = old_indices[j] ;
            }
        }
        m.texcoords_indices[k] = std::move( new_indices ) ;
    }

    return std::move(m) ;
}

//*******************************************************************
polygon_mesh::this_t polygon_mesh::repeat_full( size_t const times ) const 
{
    this_t m = *this ;

    if( motor::log::global::error(m.position_format == motor::geometry::vector_component_format::invalid, 
        "[polygon_mesh::repeat_full] : invalid position formal") )
    {
        return std::move( m ) ;
    }

    if( m.normals_indices.size() > 0 && 
        motor::log::global::error(m.normal_format == motor::geometry::vector_component_format::invalid, 
        "[polygon_mesh::repeat_full] : invalid normal formal") )
    {
        return std::move( m ) ;
    }

    if( m.texcoords_indices.size() > 0 &&
        motor::log::global::error( m.texcoord_format == motor::geometry::texcoord_component_format::invalid, 
        "[polygon_mesh::repeat_full] : invalid texcoord formal") )
    {
        return std::move( m ) ;
    }

    // repeat polygons
    {
        motor::geometry::uints_t old_polys = std::move( m.polygons ) ;
        motor::geometry::uints_t new_polys( times * old_polys.size() ) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_polys.size() ;
            for( size_t j = 0; j < old_polys.size(); ++j )
            {
                new_polys[base_index + j] = old_polys[j] ;
            }
        }
        m.polygons = std::move( new_polys ) ;
    }

    // repeat position
    {
        motor::geometry::uints_t old_indices = std::move( m.indices ) ;
        motor::geometry::floats_t old_pos = std::move( m.positions ) ;

        motor::geometry::uints_t new_indices( times * old_indices.size() ) ;
        motor::geometry::floats_t new_pos( times * old_pos.size() ) ;

        size_t const div = motor::geometry::vector_component_format_to_number(m.position_format) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_indices.size() ;
            size_t const base_pos = i * old_pos.size() ;

            // do positions
            for( size_t j=0; j<old_pos.size(); ++j )
            {
                new_pos[base_pos+j] = old_pos[j] ;
            }

            size_t const per_elem_adv = base_pos/div ;

            // do indices
            for( size_t j = 0; j < old_indices.size(); ++j )
            {
                new_indices[base_index + j] = old_indices[j] + uint_t(per_elem_adv) ;
            }
        }
        m.positions = std::move( new_pos ) ;
        m.indices = std::move( new_indices ) ;
    }

    // repeat normals
    for( size_t k = 0; k < m.normals_indices.size(); ++k )
    {
        motor::geometry::uints_t old_indices = std::move( m.normals_indices[k] ) ;
        motor::geometry::floats_t old_normals = std::move( m.normals[k] ) ;

        motor::geometry::uints_t new_indices( times * old_indices.size() ) ;
        motor::geometry::floats_t new_normals( times * old_normals.size() ) ;

        size_t const div = motor::geometry::vector_component_format_to_number(m.normal_format) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_indices.size() ;
            size_t const base_normal = i * old_normals.size() ;

            // do normals
            for( size_t j = 0; j < old_normals.size(); ++j )
            {
                new_normals[base_normal+j] = old_normals[j] ;
            }

            size_t const per_elem_adv = base_normal/div ;

            // do indices
            for( size_t j = 0; j < old_indices.size(); ++j )
            {
                new_indices[base_index + j] = old_indices[j] + uint_t(per_elem_adv) ;
            }
        }

        m.normals[k] = std::move( new_normals ) ;
        m.normals_indices[k] = std::move( new_indices ) ;
    }

    // repeat texcoords
    for( size_t k = 0; k < m.texcoords_indices.size(); ++k )
    {
        motor::geometry::uints_t old_indices = std::move( m.texcoords_indices[k] ) ;
        motor::geometry::floats_t old_texcoords = std::move( m.texcoords[k] ) ;

        motor::geometry::uints_t new_indices( times * old_indices.size() ) ;
        motor::geometry::floats_t new_texcoords( times * old_texcoords.size() ) ;

        size_t const div = motor::geometry::texcoords_component_format_to_number(m.texcoord_format) ;

        for( size_t i = 0; i < times; ++i )
        {
            size_t const base_index = i * old_indices.size() ;
            size_t const base_texcoord = i * old_texcoords.size() ;

            // do texcoords
            for( size_t j = 0; j < old_texcoords.size(); ++j )
            {
                new_texcoords[base_texcoord+j] = old_texcoords[j] ;
            }

            size_t const per_elem_adv = base_texcoord/div ;

            // do indices
            for( size_t j = 0; j < old_indices.size(); ++j )
            {
                new_indices[base_index + j] = old_indices[j] + uint_t(per_elem_adv) ;
            }
        }
        m.texcoords[k] = std::move( new_texcoords ) ;
        m.texcoords_indices[k] = std::move( new_indices ) ;
    }

    return std::move(m) ;
}


