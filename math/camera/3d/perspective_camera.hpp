#pragma once

#include "../../typedefs.h"

#include "../../vector/vector3.hpp"
#include "../../vector/vector4.hpp"

#include "../../matrix/matrix3.hpp"
#include "../../matrix/matrix4.hpp"

#include "../../quaternion/quaternion4.hpp"

#include "../../primitive/3d/ray.hpp"
#include "../../primitive/3d/frustum.hpp"

#include "../../utility/3d/look_at.hpp"
#include "../../utility/3d/perspective_fov.hpp"
#include "../../utility/3d/orthographic_projection.hpp"

namespace motor_math
{
    namespace math
    {
        template< typename type_t >
        class perspective_camera
        {
            typedef perspective_camera< type_t > this_t ;
            typedef this_t& this_ref_t ;

            typedef motor::math::m3d::ray< type_t > ray3_t ;
            typedef ray3_t const& ray3_cref_t ;
            typedef ray3_t& ray3_ref_t ;

            typedef motor::math::vector2< type_t > vec2_t ;
            typedef vec2_t& vec2_ref_t ;
            typedef vec2_t const& vec2_cref_t ;

            typedef motor::math::vector3< type_t > vec3_t ;
            typedef vec3_t& vec3_ref_t ;
            typedef vec3_t const& vec3_cref_t ;

            typedef motor::math::vector4< type_t > vec4_t ;
            typedef vec4_t& vec4_ref_t ;
            typedef vec4_t const& vec4_cref_t ;

            typedef motor::math::matrix4< type_t > mat4_t ;
            typedef mat4_t const& mat4_cref_t ;
            typedef mat4_t& mat4_ref_t ;
            typedef mat4_t* mat4_ptr_t ;

            typedef motor::math::matrix3< type_t > mat3_t ;
            typedef mat3_t const& mat3_cref_t ;

            typedef motor::math::quaternion4< type_t > quat4_t ;
            typedef quat4_t const& quat4_cref_t ;

            typedef motor::math::m3d::frustum< type_t > frustum_t ;
            typedef frustum_t const& frustum_cref_t ;
            typedef frustum_t* frustum_ptr_t ;

            typedef typename frustum_t::plane_t plane_t ;
            typedef plane_t& plane_ref_t ;

        private:

            mat4_t _proj ;
            mat4_t _view ;

            mat4_t _cam ;

            frustum_t _frustum ;

            /// perspective properties
            /// x: fov
            /// y: ar
            /// z: nz
            /// w: fz
            vec4_t _pp ;

            type_t _swidth ;
            type_t _sheight ;

        public:

            perspective_camera( void_t ) {}
            perspective_camera( vec2_cref_t sdim, type_t fov, type_t nz, type_t fz )
            {
                build_projection_matrix( sdim, fov, nz, fz ) ;
            }

            this_ref_t translate_only( vec3_cref_t delta )
            {
                motor::math::m3d::set_pos_for_camera_matrix( get_position() + delta, _cam ) ;
            }

            this_ref_t rotate_frame_only_from_right( quat4_cref_t delta )
            {
                _cam.right_multiply( mat3_t( delta ) ) ;
                return build_camera_matrix() ;
            }

            this_ref_t build_camera_matrix( void_t )
            {
                motor::math::m3d::create_view_matrix<type_t>( _cam, _view ) ;
                reconstruct_frustum_planes() ;
                return *this ;
            }

            this_ref_t build_camera_matrix( vec3_cref_t pos )
            {
                motor::math::m3d::set_pos_for_camera_matrix<type_t>( pos, _cam ) ;
                motor::math::m3d::create_view_matrix<type_t>( _cam, _view ) ;
                reconstruct_frustum_planes() ;
                return *this ;
            }

            this_ref_t build_camera_matrix( vec3_cref_t pos, vec3_cref_t target )
            {
                motor::math::m3d::create_lookat<type_t>( pos, target, _cam ) ;
                motor::math::m3d::create_view_matrix<type_t>( _cam, _view ) ;
                reconstruct_frustum_planes() ;
                return *this ;
            }

            this_ref_t build_camera_matrix( vec3_cref_t right, vec3_cref_t up, vec3_cref_t dir )
            {
                motor::math::m3d::create_lookat_lh_from_vec<type_t>( get_position(), dir, up, right, _cam ) ;
                motor::math::m3d::create_view_matrix<type_t>( _cam, _view ) ;
                reconstruct_frustum_planes() ;
                return *this ;
            }

            this_ref_t build_projection_matrix( type_t width, type_t height, type_t fov, type_t nz, type_t fz )
            {
                _swidth = width ;
                _sheight = height ;

                type_t aspect = width / height ;

                _pp = vec4_t( fov, aspect, nz, fz ) ;
                motor::math::m3d::create_perspective_fov<type_t>( fov, aspect, nz, fz, _proj ) ;
                reconstruct_frustum_planes() ;
                return *this ;
            }

            this_ref_t build_projection_matrix( vec2_cref_t sdim, type_t fov, type_t nz, type_t fz )
            {
                return build_projection_matrix( sdim.x(), sdim.y(), fov, nz, fz ) ;
            }

            /// rebuild from internal properties
            this_ref_t build_projection_matrix( void_t )
            {
                motor::math::m3d::create_perspective_fov<type_t>( _pp.x(), _pp.y(), _pp.z(), _pp.w(), _proj ) ;
                reconstruct_frustum_planes() ;
                return *this ;
            }

            /// rebuild from internal properties
            this_ref_t build_projection_matrix( vec2_cref_t sdim )
            {
                set_screen_dimension( sdim ) ;
                return build_projection_matrix() ;
            }


        public: // set/get

            /// no side effect. the user need to rebuild the matrices herself!
            this_ref_t set_screen_dimension( type_t width, type_t height ) {
                _swidth = width ; _sheight = height ; _pp.y() = width / height ; return *this ;
            }

            this_ref_t set_screen_dimension( vec2_cref_t sdim ) {
                return set_screen_dimension( sdim.x(), sdim.y() ) ;
            }

            /// no side effect. the user need to rebuild the matrices herself!
            this_ref_t set_field_of_view( type_t fov ) { _pp.x() = fov ; return *this ; }

            /// no side effect. the user need to rebuild the matrices herself!
            this_ref_t set_near_far_distance( type_t nz, type_t fz ) { _pp.z() = nz ; _pp.w() = fz ; return *this ; }

            type_t get_fov( void_t ) const { return _pp.x() ; }
            type_t get_aspect( void_t ) const { return _pp.y() ; }
            type_t get_near_z( void_t ) const { return _pp.z() ; }
            type_t get_far_z( void_t ) const { return _pp.w() ; }

            vec2_t get_near_far( void_t ) const { return vec2_t( this_t::get_near_z(), this_t::get_far_z() ) ; }

        public: // get 

            mat4_ptr_t get_projection_matrix_ptr( void_t ) { return &_proj ; }
            mat4_ptr_t get_camera_matrix_ptr( void_t ) { return &_cam ; }
            mat4_ptr_t get_view_matrix_ptr( void_t ) { return &_view ; }

            mat4_ref_t get_projection_matrix( void_t ) { return _proj ; }
            mat4_ref_t get_camera_matrix( void_t ) { return _cam ; }
            mat4_ref_t get_view_matrix( void_t ) { return _view ; }

            mat4_cref_t get_projection_matrix( void_t ) const { return _proj ; }
            mat4_cref_t get_camera_matrix( void_t ) const { return _cam ; }
            mat4_cref_t get_view_matrix( void_t ) const { return _view ; }

            frustum_cref_t get_frustum( void_t ) const { return _frustum ; }
            frustum_ptr_t get_frustum_ptr( void_t ) { return &_frustum ; }

        public: // get vec

            vec3_t get_position( void_t ) const
            {
                vec3_t pos ;
                motor::math::m3d::get_pos_from_camera_matrix( pos, _cam ) ;
                return pos ;
            }

            this_ref_t get_position( vec3_ref_t pos ) const
            {
                motor::math::m3d::get_pos_from_camera_matrix( pos, _cam ) ;
                return *this ;
            }

            vec3_t get_direction( void_t ) const
            {
                vec3_t dir ;
                motor::math::m3d::get_dir_from_camera_matrix<type_t>( dir, _cam ) ;
                return dir ;
            }

            this_ref_t get_direction( vec3_ref_t dir ) const
            {
                motor::math::m3d::get_dir_from_camera_matrix<type_t>( dir, _cam ) ;
                return *this ;
            }

            vec3_t get_up( void_t ) const
            {
                vec3_t up ;
                motor::math::m3d::get_up_from_camera_matrix<type_t>( up, _cam ) ;
                return up ;
            }

            this_ref_t get_up( vec3_ref_t up ) const
            {
                motor::math::m3d::get_up_from_camera_matrix<type_t>( up, _cam ) ;
                return *this ;
            }

            vec3_t get_right( void_t ) const
            {
                vec3_t right ;
                motor::math::m3d::get_right_from_camera_matrix<type_t>( right, _cam ) ;
                return right ;
            }

            this_ref_t get_right( vec3_ref_t right ) const
            {
                motor::math::m3d::get_right_from_camera_matrix<type_t>( right, _cam ) ;
                return *this ;
            }


        public: // ray

            /// creates a ray in view center direction.
            /// @param norm_pos [in] [-1,1] screen position
            ray3_t create_ray_norm( vec2_cref_t norm_pos ) const
            {
                type_t w = motor::math::m3d::get_width_at<float_t>( get_fov(), get_aspect(), type_t( 1 ) ) ;
                type_t h = motor::math::m3d::get_height_at<float_t>( get_fov(), type_t( 1 ) ) ;
                return ray3_t( get_position(), _cam * vec3_t( norm_pos * vec2_t( w, h ) * type_t( 0.5 ), type_t( 1 ) ).normalize() ) ;
            }

            /// creates a ray in view center direction.
            ray3_t create_center_ray( void_t ) const
            {
                //return create_ray_norm( vec2_t(type_t(0), type_t(0)) ) ;
                return ray3_t( get_position(), get_direction() ) ;
            }

            /// creates a ray in view center direction.
            /// @param screen_pos [in] [(0,0),(width,height)] screen position.
            ray3_t create_ray( vec2_cref_t screen_pos ) const
            {
                vec2_t norm_coord = screen_pos / vec2_t( _swidth, _sheight ) ;
                norm_coord = norm_coord * type_t( 2 ) - type_t( 1 ) ;
                return create_ray_norm( norm_coord ) ;
            }

        private:

            /// constructs this view frustum's planes from passed parameters.
            void_t reconstruct_frustum_planes( void_t )
            {
                mat4_t comb = _proj * _view ;

                vec4_t r0, r1, r2, r3 ;

                comb.get_row( 0, r0 ) ;
                comb.get_row( 1, r1 ) ;
                comb.get_row( 2, r2 ) ;
                comb.get_row( 3, r3 ) ;

                vec4_t p0 = ( r3 + r0 ).normalized() ;
                vec4_t p1 = ( r3 - r0 ).normalized() ;
                vec4_t p2 = ( r3 + r1 ).normalized() ;
                vec4_t p3 = ( r3 - r1 ).normalized() ;
                vec4_t p4 = ( r3 + r2 ).normalized() ;
                vec4_t p5 = ( r3 - r2 ).normalized() ;

                _frustum.set_plane( frustum_t::p_left, plane_t( p0.negate() ) ) ;
                _frustum.set_plane( frustum_t::p_right, plane_t( p1.negate() ) ) ;
                _frustum.set_plane( frustum_t::p_bottom, plane_t( p2.negate() ) ) ;
                _frustum.set_plane( frustum_t::p_top, plane_t( p3.negate() ) ) ;
                _frustum.set_plane( frustum_t::p_near, plane_t( p4.negate() ) ) ;
                _frustum.set_plane( frustum_t::p_far, plane_t( p5.negate() ) ) ;
            }
        } ;

        motor_typedefs( perspective_camera<float_t>, perspective_cameraf ) ;
    }
}