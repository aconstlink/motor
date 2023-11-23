#pragma once

#include "../../vector/vector2.hpp"
#include "../../vector/vector3.hpp"

namespace motor
{
    namespace math
    {
        namespace m2d
        {
            template< typename type_t >
            class ray
            {
                motor_this_typedefs( ray< type_t > ) ;
                motor_typedefs( motor::math::vector2< type_t >, vec2 ) ;
                motor_typedefs( motor::math::vector3< type_t >, vec3 ) ;

            private:

                vec2_t _origin ;
                vec2_t _dir ;

            public:

                ray( void_t ) noexcept {}

                ray( vec2_cref_t orig, vec2_cref_t dir ) noexcept 
                {
                    _origin = orig ;
                    _dir = dir ;
                }

                vec2_cref_t get_origin( void_t ) const noexcept { return _origin ; }
                vec2_cref_t get_direction( void_t ) const noexcept { return _dir ; }

                vec2_t point_at( type_t dist ) const noexcept { return _origin + _dir * dist ; }

                this_ref_t translate_to( this_t::vec2_cref_t p ) noexcept 
                {
                    _origin = p ;
                    return *this ;
                }

                void_t set_direction( vec2_cref_t d ) noexcept { _dir = d ; }

                // dot with both direction vectors
                type_t dot( this_cref_t other ) const noexcept
                {
                    return _dir.dot( other.get_direction() ) ;
                }

                // p is supposed to lay on this ray.
                type_t lambda_to( vec2_cref_t p ) const noexcept 
                {
                    return _dir.dot( p ) ;
                }

                type_t lambda_to( this_cref_t or ) const noexcept 
                {
                    // orhto of the other rays' direction
                    auto const n = vec3_t( 
                        or.get_direction().ortho(), 
                        -or.get_direction().ortho().dot( or.get_origin() ) ) ;
                    
                    return 
                        -n.dot( vec3_t( _origin, type_t(1) ) ) /
                        +n.xy().dot( _dir ) ;
                }

                vec2_t point_at( this_cref_t or ) const noexcept
                {
                    return this_t::point_at( this_t::lambda_to( or ) ) ;
                }
            };
        }
        typedef m2d::ray< float_t > ray2f_t ;
    }
}