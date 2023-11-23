#pragma once

#include <motor/math/matrix/matrix4.hpp>
#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

namespace motor
{
    namespace math
    {
        namespace m3d
        {
            namespace detail
            {
                /// 
                template< typename T >
                struct perspective_projection
                {
                    motor_this_typedefs( perspective_projection<T> ) ;
                    motor_typedefs( T, type ) ;

                public:

                    /// get the half length for a horizontal or vertical field of view, in order
                    /// to retrieve the half width/height for the plane at z
                    static type_t get_half_length_for( type_t const fov, type_t const z )
                    {
                        return tanf( fov * type_t( 0.5 ) ) * z ;
                    }

                public:

                    static type_t get_length_for( type_t const fov, type_t const z )
                    {
                        return type_t( 2.0 ) * this_t::get_half_length_for( fov, z ) ;
                    }
                };
            }

            ///
            template< typename T >
            struct perspective
            {
                motor_this_typedefs( perspective<T> ) ;

                motor_typedefs( T, type ) ;
                motor_typedefs( motor::math::vector2<type_t>, vec2 ) ;
                motor_typedefs( motor::math::vector4<type_t>, vec4 ) ;
                motor_typedefs( motor::math::matrix4<type_t>, mat4 ) ;

                motor_typedefs( motor::math::m3d::detail::perspective_projection<type_t>, proj ) ;

            private:

                static void_t __create( type_t const w, type_t const h, type_t const n,
                    type_t const f, mat4_out_t m )
                {
                    // make column vectors
                    // but they are the row vectors of
                    // the dx projection matrix.
                    vec4_t const vcX( type_t( 2 ) * n / w, type_t( 0 ), type_t( 0 ), type_t( 0 ) ) ;
                    vec4_t const vcY( type_t( 0 ), type_t( 2 ) * n / h, type_t( 0 ), type_t( 0 ) ) ;
                    vec4_t const vcZ( type_t( 0 ), type_t( 0 ), ( f ) / ( f - n ), -( 2 * n * f ) / ( f - n ) ) ;
                    vec4_t const vcW( type_t( 0 ), type_t( 0 ), type_t( 1 ), type_t( 0 ) ) ;

                    m.set_row( 0, vcX ) ;
                    m.set_row( 1, vcY ) ;
                    m.set_row( 2, vcZ ) ;
                    m.set_row( 3, vcW ) ;
                }

                /// not tested yet - just an idea
                /// optimized version if angles are present
                /// tan_fovw : tan( angle_w/2 )
                /// tan_fovh : tan( angle_h/2 )
                /// 2n/w = 2n/(2n*tan_fovw) = 1/tan_fovw
                /// 2n/h = 2n/(2n*tan_fovh) = 1/tan_fovw
                static void_t __create_by_fov( type_t const fov_a, type_t const fov_h, type_t const n,
                    type_t const f, mat4_out_t m )
                {
                    type_t const tan_w = tanf( fov_a * type_t( 0.5 ) ) ;
                    type_t const tan_h = tanf( fov_h * type_t( 0.5 ) ) ;

                    // 1/tan = ctan
                    vec4_t const vcX( -type_t( 1 ) / tan_w, type_t( 0 ), type_t( 0 ), type_t( 0 ) ) ;
                    vec4_t const vcY( type_t( 0 ), type_t( 1 ) / tan_h, type_t( 0 ), type_t( 0 ) ) ;
                    vec4_t const vcZ( type_t( 0 ), type_t( 0 ), ( f ) / ( f - n ), -( 2 * n * f ) / ( f - n ) ) ;
                    vec4_t const vcW( type_t( 0 ), type_t( 0 ), type_t( 1 ), type_t( 0 ) ) ;

                    m.set_row( 0, vcX ) ;
                    m.set_row( 1, vcY ) ;
                    m.set_row( 2, vcZ ) ;
                    m.set_row( 3, vcW ) ;
                }

            public:

                /// @param fov horizontal and vertical fov in rad
                static void_t create_by_fov( vec2_cref_t const fov, type_t const n, type_t const f, mat4_out_t m )
                {
                    type_t const w = proj_t::get_length_for( fov.x(), n ) ;
                    type_t const h = proj_t::get_length_for( fov.y(), n ) ;

                    this_t::__create( w, h, n, f, m ) ;
                }

                /// by horizontal fov
                static void_t create_by_fovh_aspect( type_t const fov, type_t const aspect,
                    type_t const n, type_t const f, mat4_out_t m )
                {
                    type_t const w = proj_t::get_length_for( fov, n ) ;
                    type_t const h = aspect * w ;

                    this_t::__create( w, h, n, f, m ) ;
                }

                /// by vertical fov
                static void_t create_by_fovv_aspect( type_t const fov, type_t const aspect,
                    type_t const n, type_t const f, mat4_out_t m )
                {
                    type_t const h = proj_t::get_length_for( fov, n ) ;
                    type_t const w = aspect * h ;

                    this_t::__create( w, h, n, f, m ) ;
                }

            public:

                /// @todo this can be optimized by resolving the used functions
                static type_t aspect_for_fov( vec2_cref_t fov )
                {
                    // 1. since the ratio does not change along z, we use z=1
                    // 2. since the factor 2 cancels out, we could completely use
                    // the half length of width and height
                    // we could do:
                    //type_t const w = tanf( fov.x() * type_t(0.5) ) ;
                    //type_t const h = tanf( fov.y() * type_t(0.5) ) ;

                    type_t const w = proj_t::get_length_for( fov.x(), type_t( 1 ) ) ;
                    type_t const h = proj_t::get_length_for( fov.y(), type_t( 1 ) ) ;

                    return w / h ;
                }

                static type_t aspect_for_size( vec2_cref_t wh )
                {
                    type_t const w = wh.x() ;
                    type_t const h = wh.y() ;

                    return w / h ;
                }

                /// if fov vertical is known, fov horizontal can be calculated with the aspect
                static type_t fovh_for_fovv_aspect( type_t const fovv, type_t const aspect )
                {
                    return atanf( aspect * tanf( fovv * type_t( 0.5 ) ) ) * type_t( 2 ) ;
                }

                /// if fov horizontal is known, fov vertical can be calculated with the aspect
                static type_t fovv_for_fovh_aspect( type_t const fovh, type_t const aspect )
                {
                    return atanf( ( type_t( 1 ) / aspect ) * tanf( fovh * type_t( 0.5 ) ) ) * type_t( 2 ) ;
                }
            };
        }

        /**
            Allows to get the plane width at a specific z value.
            Just pass the aspect ratio to be preserved and a 
            field of view.

            @deprecated
        */
        template< typename real >
        real get_width_at( real fov, real ar, real z )
        {
            return 2.0f * ar * tanf(fov/2.0f) * z ;
        }

        /**
            Allows to get the plane height at a specific z value.
            Here just the field of view is needed.

            @deprecated
        */
        template< typename real >
        real get_height_at( real fov, real z )
        {
            return 2.0f * tanf(fov/2.0f) * z ;
        }

        /**
                Creates a prespective viewing projection matrix. 
                Here the width and height of the projection plane must be passed.
                This depends on the fov of the observer.
                If the near plane's distance to the observer is preserved, so the
                width or height is changing. This must be considered.

                @param width [in] The width at the projection plane.
                @param height [in] The height at the projection plane.
                @param n The [in] near plane distance.
                @param f The [in] far plane distance.
                @param inout [out] The projection matrix. 

                @deprecated

        */
        template< typename type_t >
        void create_perspective_projection_matrix( type_t width, type_t height, type_t n, type_t f, 
                motor::math::matrix4< type_t > & inout ) 
        {
        
            typedef motor::math::vector4< type_t > vec4_t ;

            // make column vectors
            // but they are the row vectors of
            // the dx projection matrix.
            vec4_t vcX(-type_t(2)*n/width,	type_t(0),			type_t(0),	    type_t(0)) ;
            vec4_t vcY(type_t(0),			type_t(2)*n/height,	type_t(0),	    type_t(0)) ;
            vec4_t vcZ(type_t(0),			type_t(0),			(f)/(f-n),	-(2*n*f)/(f-n)) ;
            vec4_t vcW(type_t(0),			type_t(0),			type_t(1),	    type_t(0) ) ;

            inout.set_row( 0,vcX ) ;
            inout.set_row( 1,vcY ) ;
            inout.set_row( 2,vcZ ) ;
            inout.set_row( 3,vcW ) ;
        }

        /**
            A convenient function which allows to construct a perspective
            projection matrix from the field of view.

            @param fov [in] The field of view.
            @param ar [in] The aspect ratio.
            @param n [in] The near plane distance. This is, where to project.
            @param f [in] The far plane distance.
            @param inout [out] The projection matrix.

            @deprecated

        */
        template< typename type_t >
        void create_perspective_projection_matrix_fov( type_t fov, type_t ar, type_t n, type_t f, 
            motor::math::matrix4< type_t > & inout )
        {            
            //type_t w = get_width_at<type_t>( fov, ar, n ) ;
            type_t h = get_height_at<type_t>( fov, n ) ;
            type_t w = ar * h ;

            create_perspective_projection_matrix<type_t>( w, h, n, f, inout ) ;
        }
    }
}