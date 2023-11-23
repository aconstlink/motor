#pragma once

#include "ray.hpp"

#include "../../vector/vector2.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            template< typename type_t >
            class line_segment
            {
                typedef line_segment< type_t > this_t ;
                typedef this_t const& this_cref_t ;
                typedef this_t& this_ref_t ;

                typedef motor::math::vector2< type_t > vec2_t ;
                typedef vec2_t const& vec2_cref_t ;

                typedef ray< type_t > ray_t ;
                typedef ray_t const& ray_cref_t ;

            private:

                vec2_t _a ;
                vec2_t _b ;

            public:

                line_segment( void_t ) {}

                line_segment( this_cref_t rhv ) : _a( rhv._a ), _b( rhv._b ) {}

                line_segment( vec2_cref_t a, vec2_cref_t b ) : _a( a ), _b( b ) {}

                line_segment( ray_cref_t r, type_t b_at )
                {
                    _a = r.get_origin() ;
                    _b = r.point_at( b_at ) ;
                }

                vec2_cref_t get_point_a( void_t ) const { return _a ; }
                vec2_cref_t get_point_b( void_t ) const { return _b ; }
                vec2_t get_center( void_t ) const { return _a + get_distance() * type_t( 0.5 ) ; }

                vec2_t get_distance( void_t ) const { return vec2_t( _b - _a ) ; }
                vec2_t get_distance_half( void_t ) const { return get_distance() * type_t( 0.5 ) ; }

                vec2_t get_direction( void_t ) const { return get_distance().normalized() ; }
                type_t get_length( void_t ) const { return get_distance().length() ; }

                ray_t get_ray( void_t ) const { return ray_t( _a, get_direction() ) ; }

                this_ref_t scale( type_t s ) { _b = _a + this_t::get_distance() * s ; return *this ; }
                this_ref_t scale_norm( type_t s ) { _b = _a + this_t::get_direction() * s ; return *this ; }

            public:

                /// converts this line segment to a line segment that ends at
                /// the projected point.
                this_ref_t project_local( vec2_cref_t point )
                {
                    const vec2_t dir = this_t::get_direction() ;
                    const vec2_t pp = dir * vec2_t( point ).dot( dir ) ;
                    _b = _a + pp ;
                    return *this ;
                }

                /// converts this line segment to a line segment that ends at
                /// the projected point.
                this_ref_t project_global( vec2_cref_t point )
                {
                    return project_local( point - _a ) ;
                }

                this_t projected_global( vec2_cref_t point ) const
                {
                    return this_t( *this ).project_global( point ) ;
                }

                this_ref_t ortho( vec2_cref_t point )
                {
                    vec2_t b = this_t( *this ).project( point ).get_point_b() ;
                    _a = b ;
                    _b = point - b ;
                    return *this ;
                }

                this_t orthoed( vec2_cref_t point ) const
                {
                    return this_t( *this ).ortho( point ) ;
                }
            };
        }
        typedef m2d::line_segment< float_t > line_segment2f_t ;
    }
}