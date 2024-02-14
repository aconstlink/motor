


#pragma once

#include "imesh.h"

#include "tri_mesh.h"
#include "flat_tri_mesh.h"

namespace motor
{
    namespace geometry
    {
        class MOTOR_GEOMETRY_API polygon_mesh : public imesh
        {
            motor_this_typedefs( polygon_mesh ) ;

        public:

            /// ith entry => ith polygon
            /// polygons[i] => how many indices for polygon i
            uints_t polygons ;

            uints_t indices ;
            floats_t positions ;

            more_uints_t normals_indices ;
            more_floats_t normals ;

            more_uints_t texcoords_indices ;
            more_floats_t texcoords ;

            vector_component_format position_format ;
            vector_component_format normal_format ;
            texcoord_component_format texcoord_format ;

        public:

            polygon_mesh( void_t ) ;
            polygon_mesh( this_cref_t ) ;
            polygon_mesh( this_rref_t ) ;
            virtual ~polygon_mesh( void_t ) ;

            this_ref_t operator = ( this_cref_t ) ;
            this_ref_t operator = ( this_rref_t ) ;

        public:

            motor::geometry::result flatten( tri_mesh_ref_t mesh_out ) const ;
            motor::geometry::result flatten( flat_tri_mesh_ref_t mesh_out ) const ;

            /// only repeats indices
            this_t repeat( size_t const times ) const ;

            /// repeats everything
            this_t repeat_full( size_t const times ) const ;

        private:

            void_t triangulate_index_buffer( size_t num_triangles,
                uints_cref_t indices_in, uints_ref_t indices_out ) const ;
        };
        motor_typedef( polygon_mesh ) ;
    }
}
