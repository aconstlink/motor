#pragma once

#include "../../vector/vector2.hpp"
#include "../../vector/vector3.hpp"

#include "../../typedefs.h"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            /// represents the hesse form normal equation of the line.
            /// the class stores a vector with (2d normal, -distance).
            /// every point can be tested in hesse space, that is, 
            /// a 2d point with a 1 in the z component. The test is
            /// a scalar product of the point in question and the 
            /// hesse line equation in vector form.
            template<typename type_t>
            class line
            {
                typedef line< type_t > this_t ;
                typedef this_t const& this_cref_t ;

                typedef motor::math::vector2< type_t > vec2_t ;
                typedef vec2_t const& vec2_cref_t ;

                typedef motor::math::vector3< type_t > vec3_t ;
                typedef vec3_t const& vec3_cref_t ;

            public:

                /// ctor flag
                struct normalize {} ;

                /// positive plane space.
                /// negative plane space.
                enum side { positive, negative } ;

            private:

                /// hesse plane
                /// x,y: normal
                /// z: -distance
                vec3_t _plane ;

            private:

                type_t hesse_distance( vec2_cref_t origin, vec2_cref_t normal ) const
                {
                    return -( normal.dot( origin ) ) ;
                }

            public:

                /// constucts a hesse plane with distance 0 and pointing in (1,0) direction.
                line( void_t ) { _plane = vec3_t( type_t( 1 ), type_t( 0 ), type_t( 0 ) ) ; }

                /// construct a hesse plane from an origin vector and
                /// a plane normal vector.
                line( vec2_cref_t origin, vec2_cref_t normal )
                {
                    _plane = vec3_t( normal, hesse_distance( origin, normal ) ) ;
                }

                /// construct a hesse plane from an origin vector and
                /// a plane normal vector. This ctor will normalize the normal.
                line( vec2_cref_t origin, vec2_cref_t normal, normalize flag )
                {
                    vec2_t n = normal.normalized() ;
                    _plane = vec3_t( n, hesse_distance( origin, n ) ) ;
                }

                /// @param hesse_form xy contains normal, z contains hesse distance.
                line( vec3_cref_t hesse_form )
                {
                    _plane = hesse_form ;
                }

            public:

                /// returns the hesse form plane equation.
                vec3_cref_t get_line( void_t ) const { return _plane ; }

                /// the plane's normal vector.
                vec2_t get_normal( void_t ) const { return _plane.xy() ; }

                /// the plane's distance in space.
                type_t distance( void_t ) const { return _plane.z() ; }

                /// returns the signed distance of point to plane.
                /// if the return value < 0, the point is behind the plane.
                /// if the return value >= 0, the point is in front of the plane.
                type_t distance_signed( vec2_cref_t point ) const
                {
                    return _plane.dot( vec3_t( point, type_t( 1 ) ) ) ;
                }

                /// returns the signed distance of point to plane.
                /// if the return value < 0, the point is behind the plane.
                /// if the return value >= 0, the point is in front of the plane.
                type_t distance_abs( vec2_cref_t point ) const
                {
                    return motor::math::fast<type_t>::abs( _plane.dot( vec3_t( point, type_t( 1 ) ) ) ) ;
                }

                /// determines the side on which the point is due to the plane.
                /// @return negative the point is behind the plane.
                /// @return positive the point is in front of the plane.
                side which_side( vec2_cref_t point ) const
                {
                    return this_t::distance_signed( point ) < type_t( 0 ) ? negative : positive ;
                }

            public:

                /// returns the projection of direction d to the line, or
                /// returns the projected distance on normal.
                /// @note vector d is assumed to be a direction vector.
                type_t onto_normal_project( vec2_cref_t d ) const
                {
                    return this_t::get_normal().dot( d ) ;
                }

                /// returns the projected vector part of direction d onto the 
                /// line's normal vector.
                /// so the returned vector is parallel to the normal.
                /// @return part of point p parallel to the line's normal.
                /// @note vector d is assumed to be a direction vector.
                vec2_t onto_normal_project_direction( vec2_cref_t d ) const
                {
                    return this_t::get_normal() * this_t::onto_normal_project( d ) ;
                }

                /// returns the parallel vector part of direction d onto the 
                /// line itself. The returned vector is orthogonal to the 
                /// normal vector of the line.
                /// so the returned vector is orthogonal to the normal.
                /// @note vector d is assumed to be a direction vector.
                vec2_t onto_line_project_direction( vec2_cref_t d ) const
                {
                    return d - this_t::onto_normal_project_direction( d ) ;
                }

            public:

                /// reflect on plane.
                /// useful, if v is pointing in the opposite direction of the normal.
                /// if v is point, v must be in plane space.
                vec2_t reflect( vec2_cref_t v ) const
                {
                    const vec2_t n = _plane.xy() ;
                    return v - n * v.dot( n ) * type_t( 2 ) ;
                }

                /// reflect on normal.
                /// useful, if v is pointing in the direction of the normal.
                /// if v is point, v must be in plane space.
                vec2_t reflect_on_normal( vec2_cref_t v ) const
                {
                    const vec2_t n = _plane.xy() ;
                    return v.negated() + n * v.dot( n ) * type_t( 2 ) ;
                }
            } ;
        }
    }
}

