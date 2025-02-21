

#include "tri_mesh.h"

#include <algorithm>

using namespace motor::geometry ;

struct per_vertex_index_vector
{
    motor_this_typedefs( per_vertex_index_vector ) ;

    uint_t pos_id = uint_t( -1 ) ;
    uint_t nrm_id = uint_t( -1 ) ;
    uint_t tx0_id = uint_t( -1 ) ;
    uint_t tx1_id = uint_t( -1 ) ;
    uint_t tx2_id = uint_t( -1 ) ;
    uint_t tx3_id = uint_t( -1 ) ;
    uint_t tx4_id = uint_t( -1 ) ;

    /// the referenced index ids.
    /// multiple tris could reference this vertex vector.
    /// we need to remember the position in the index array
    motor::geometry::uints_t ref_iids ;

    uint_t new_vertex_id = uint_t(-1) ;

    bool_t operator == ( this_cref_t rhv ) const
    {
        if( pos_id != rhv.pos_id ) return false ;
        if( nrm_id != rhv.nrm_id ) return false ;
        if( tx0_id != rhv.tx0_id ) return false ;
        if( tx1_id != rhv.tx1_id ) return false ;
        if( tx2_id != rhv.tx2_id ) return false ;
        if( tx3_id != rhv.tx3_id ) return false ;
        if( tx4_id != rhv.tx4_id ) return false ;
        return true ;
    }
};
typedef motor::vector< per_vertex_index_vector > per_vertex_index_vectors_t ;
typedef motor::vector< per_vertex_index_vectors_t > more_per_vertex_index_vectors_t ;

//*******************************************************************
tri_mesh::tri_mesh( void_t ) 
{
}

//*******************************************************************
tri_mesh::tri_mesh( this_cref_t rhv ) 
{
    *this = rhv ;
}

//*******************************************************************
tri_mesh::tri_mesh( this_rref_t rhv )
{
    *this = std::move( rhv );
}

//*******************************************************************
tri_mesh::~tri_mesh( void_t )
{
}

//*******************************************************************
tri_mesh::this_ref_t tri_mesh::operator = ( this_cref_t rhv )
{
    indices = rhv.indices ;
    texcoords_indices = rhv.texcoords_indices ;
    normals_indices = rhv.normals_indices ;

    positions = rhv.positions ;
    texcoords = rhv.texcoords ;
    normals = rhv.normals ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;

    return *this ;
}

//*******************************************************************
tri_mesh::this_ref_t tri_mesh::operator = ( this_rref_t rhv )
{
    indices = std::move( rhv.indices ) ;
    texcoords_indices = std::move( rhv.texcoords_indices ) ;
    normals_indices = std::move( rhv.normals_indices ) ;

    positions = std::move( rhv.positions ) ;
    texcoords = std::move( rhv.texcoords ) ;
    normals = std::move( rhv.normals ) ;

    position_format = rhv.position_format ;
    normal_format = rhv.normal_format ;
    texcoord_format = rhv.texcoord_format ;

    return *this ;
}

//*******************************************************************
motor::geometry::result tri_mesh::flatten( flat_tri_mesh_ref_t mesh_out ) const 
{    
    // do some checks
    {}

    size_t const num_position_components = 
        motor::geometry::vector_component_format_to_number( motor::geometry::vector_component_format::xyz ) ;

    size_t const num_vertices = positions.size() / num_position_components ;
    // if a position is referenced by multiple index vectors, that
    // position must be duplicated and a new vertex must be created.
    more_per_vertex_index_vectors_t per_position_references( num_vertices ) ;

    //
    // PASS #1 : Find references
    //
    size_t const num_indices = indices.size() ;
    for( uint_t iid=0; iid<num_indices; ++iid )
    {
        // this is the vertex_id but since this mesh
        // has multiple indirections, this is actually the
        // position_id.
        uint_t const pos_id = indices[iid] ;

        per_vertex_index_vector iv ;

        // 1. prepare the full index vector for equality check
        {
            iv.pos_id = pos_id ;
        }
        
        if( normals_indices.size() > 0 )
            iv.nrm_id = normals_indices[0][iid] ;
        
        if( texcoords_indices.size() > 0 )
            iv.tx0_id = texcoords_indices[0][iid] ;
        if( texcoords_indices.size() > 1 )
            iv.tx1_id = texcoords_indices[1][iid] ;
        if( texcoords_indices.size() > 2 )
            iv.tx2_id = texcoords_indices[2][iid] ;

        // 2. find if the index vector already exists for the vertex position in question(pos_id)
        // if yes - add the index id(iid) to the index vector's internal vid reference vector.
        // if not - add the whole index vector to the index vector array of the position vertex.
        {
            auto & cur_index_vectors = per_position_references[pos_id] ;
            

            #if 1
            {
                size_t i = 0 ;

                for ( i; i < cur_index_vectors.size(); ++i )
                {
                    auto & item = cur_index_vectors[ i ] ;
                    if ( item == iv )
                    {
                        item.ref_iids.push_back( iid ) ;
                        break ;
                    }
                }

                if ( i == cur_index_vectors.size() )
                {
                    iv.ref_iids.push_back( iid, 1000 ) ;
                    cur_index_vectors.push_back( iv ) ;
                }
            }
            #else
            auto found = std::find_if( cur_index_vectors.begin(), cur_index_vectors.end(),
                [&]( per_vertex_index_vector const & item )
            {
                return item == iv ;
            } ) ;

            // if found, that vertex is referenced by another polygon
            if(found != cur_index_vectors.end())
            {
                found->ref_iids.push_back( iid ) ;
            }
            else
            {
                iv.ref_iids.push_back( iid ) ;
                cur_index_vectors.push_back( iv ) ;
            }
            #endif
        }        
    }

    //
    // PASS #2 : Count required vertices
    //
    size_t num_new_vertices = 0 ;
    size_t num_new_normals = 0 ;
    size_t num_new_tex0 = 0 ;
    size_t num_new_tex1 = 0 ;
    size_t num_new_tex2 = 0 ;

    {
        for(auto & index_vectors : per_position_references)
        {
            num_new_vertices += index_vectors.size() ;
        }
    }

    if( normals_indices.size() > 0 )
        num_new_normals = num_new_vertices ;

    if( texcoords_indices.size() > 0 )
        num_new_tex0 = num_new_vertices ;
    if( texcoords_indices.size() > 1 )
        num_new_tex1 = num_new_vertices ;
    if( texcoords_indices.size() > 2 )
        num_new_tex2 = num_new_vertices ;
    //
    // PASS #3 : Do the expansion directly into the flat mesh
    //
    {
        motor::geometry::floats_t new_positions( num_new_vertices * 3 ) ;
        motor::geometry::floats_t new_normals( num_new_normals * 3 ) ;
        motor::geometry::floats_t new_uvs0( num_new_tex0 * 2 ) ;
        motor::geometry::floats_t new_uvs1( num_new_tex1 * 2 ) ;
        motor::geometry::floats_t new_uvs2( num_new_tex2 * 2 ) ;

        mesh_out.position_format = motor::geometry::vector_component_format::xyz ;
        mesh_out.normal_format = motor::geometry::vector_component_format::xyz ;
        mesh_out.texcoord_format = motor::geometry::texcoord_component_format::uv ;

        size_t const uvb = texcoord_format == motor::geometry::texcoord_component_format::uvw ? 3 : 2 ;

        uint_t vertex_id = 0 ;

        for( auto & index_vectors : per_position_references )
        {
            // create a new vertex per index vector
            for( size_t ivi = 0; ivi< index_vectors.size(); ++ivi )
            {
                auto & index_vector = index_vectors[ivi] ;

                // position
                {
                    new_positions[vertex_id*3+0] = positions[index_vector.pos_id*3+0] ;
                    new_positions[vertex_id*3+1] = positions[index_vector.pos_id*3+1] ;
                    new_positions[vertex_id*3+2] = positions[index_vector.pos_id*3+2] ;
                }

                // normals
                if( index_vector.nrm_id != -1 )
                {
                    new_normals[vertex_id * 3 + 0] = normals[0][index_vector.nrm_id * 3 + 0] ;
                    new_normals[vertex_id * 3 + 1] = normals[0][index_vector.nrm_id * 3 + 1] ;
                    new_normals[vertex_id * 3 + 2] = normals[0][index_vector.nrm_id * 3 + 2] ;
                }

                // texcoords0
                if( index_vector.tx0_id != -1 )
                {
                    new_uvs0[vertex_id * 2 + 0] = texcoords[0][index_vector.tx0_id * uvb + 0] ;
                    new_uvs0[vertex_id * 2 + 1] = texcoords[0][index_vector.tx0_id * uvb + 1] ;
                }

                // texcoords1
                if( index_vector.tx1_id != -1 )
                {
                    new_uvs1[vertex_id * 2 + 0] = texcoords[1][index_vector.tx1_id * uvb + 0] ;
                    new_uvs1[vertex_id * 2 + 1] = texcoords[1][index_vector.tx1_id * uvb + 1] ;
                }

                // texcoords2
                if( index_vector.tx2_id != -1 )
                {
                    new_uvs2[vertex_id * 2 + 0] = texcoords[2][index_vector.tx2_id * uvb + 0] ;
                    new_uvs2[vertex_id * 2 + 1] = texcoords[2][index_vector.tx2_id * uvb + 1] ;
                }

                // store the new vertex_id for remeshing
                index_vector.new_vertex_id = vertex_id ;

                vertex_id++ ;
            }
        }

        mesh_out.positions = std::move( new_positions ) ;

        if( normals_indices.size() > 0 )
            mesh_out.normals = std::move( new_normals ) ;
        if( texcoords_indices.size() > 0 )
            mesh_out.texcoords.push_back( std::move( new_uvs0 ) ) ;
        if( texcoords_indices.size() > 1 )
            mesh_out.texcoords.push_back( std::move( new_uvs1 ) ) ;
        if( texcoords_indices.size() > 2 )
            mesh_out.texcoords.push_back( std::move( new_uvs2 ) ) ;
    }

    //
    // PASS #4 : rearrange the index buffer for the expanded vertex buffer
    //
    {
        // the number of polygons do not change, so the current number
        // of indices can still be used.
        uints_t new_indices( indices.size() ) ;

        for(auto & index_vectors : per_position_references)
        {
            for(size_t ivi = 0; ivi< index_vectors.size(); ++ivi )
            {
                auto const & index_vector = index_vectors[ivi] ;

                for( size_t i=0; i<index_vector.ref_iids.size(); ++i )
                {
                    uint_t const iid = index_vector.ref_iids[i] ;
                    new_indices[iid] = index_vector.new_vertex_id ;
                }
            }
        }

        mesh_out.indices = std::move( new_indices ) ;
    }

    return motor::geometry::ok ;
}