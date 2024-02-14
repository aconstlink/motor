
#pragma once

#include "imesh.h"
#include "../enums/component_formats.h"

#include <vector>

namespace motor
{
    namespace geometry
    {
        class MOTOR_GEOMETRY_API flat_line_mesh : public imesh
        {
            motor_this_typedefs( flat_line_mesh ) ;

        public:

            motor::geometry::vector_component_format position_format ;

            uints_t indices ;
            floats_t positions ;

        public:

            flat_line_mesh( void_t ) ;
            flat_line_mesh( this_rref_t ) ;
            virtual ~flat_line_mesh( void_t ) ;

            this_ref_t operator = ( this_rref_t ) ;

        public:

            this_t repeat_full( size_t const times ) const ;

        };
        motor_typedef( flat_line_mesh ) ;
    }
}

