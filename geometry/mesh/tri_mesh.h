



#pragma once

#include "imesh.h"

#include "flat_tri_mesh.h"

namespace motor
{
    namespace geometry
    {
        /// a mesh with triangles. Every three indices 
        /// form a triangle.
        /// @note this mesh type does not have a polygon
        /// indirection table. The polygon's(tri's) vertices
        /// are indirected by the index tables. Every three
        /// indices make a triangle.
        struct MOTOR_GEOMETRY_API tri_mesh : public imesh
        {
            motor_this_typedefs( tri_mesh ) ;

        public:

            /// defines triangles => 
            /// every 3 indices represent a triangle
            uints_t indices ;
            more_uints_t texcoords_indices ;
            more_uints_t normals_indices ;

            /// x,y,z, x,y,z, ...
            floats_t positions ;
            more_floats_t texcoords ;
            more_floats_t normals ;

            vector_component_format position_format ;
            vector_component_format normal_format ;
            texcoord_component_format texcoord_format ;

        public:

            tri_mesh( void_t ) ;
            tri_mesh( this_cref_t ) ;
            tri_mesh( this_rref_t ) ;
            virtual ~tri_mesh( void_t ) ;

            this_ref_t operator = ( this_cref_t ) ;
            this_ref_t operator = ( this_rref_t ) ;

        public:

            motor::geometry::result flatten( flat_tri_mesh_ref_t mesh_out ) const ;

        };
        motor_typedef( tri_mesh ) ;
    }
}