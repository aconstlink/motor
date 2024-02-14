
#pragma once

#include "../typedefs.h"
#include "../api.h"
#include "../result.h"

#include "imesh.h"

#include "../enums/component_formats.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>

namespace motor
{
    namespace geometry
    {
        /// this mesh type only has one indirection table.
        /// that indirection table(index buffer) connects
        /// the mesh's vertices to triangles.
        struct MOTOR_GEOMETRY_API flat_tri_mesh : public imesh
        {
            motor_this_typedefs( flat_tri_mesh ) ;

        public:

            /// defines triangles => 
            /// every 3 indices represent a triangle
            uints_t indices ;
            floats_t positions ;
            floats_t normals ;
            more_floats_t texcoords ;

            vector_component_format position_format ;
            vector_component_format normal_format ;
            texcoord_component_format texcoord_format ;

        public:

            flat_tri_mesh( void_t ) ;
            flat_tri_mesh( this_cref_t ) ;
            flat_tri_mesh( this_rref_t ) ;
            virtual ~flat_tri_mesh( void_t ) ;

        public:

            this_ref_t operator = ( this_rref_t ) ;
            this_ref_t operator = ( this_cref_t ) ;

        public:

            bool_t are_array_numbers_valid( void_t ) const ;

            size_t get_num_vertices( void_t ) const ;

            size_t get_num_positions( void_t ) const ;
            size_t get_num_normals( void_t ) const ;
            size_t get_num_texcoords( size_t const ) const ;

        public: // if you know the type is a 3d mesh, use these shortcut functions

            motor::math::vec3f_t get_vertex_position_3d( size_t const i ) const noexcept ;
            motor::math::vec3f_t get_vertex_normal_3d( size_t const i ) const noexcept ;
            motor::math::vec2f_t get_vertex_texcoord( size_t const l, size_t const i ) const noexcept ;


        };
        motor_typedef( flat_tri_mesh ) ;
    }
}
