#pragma once

#include "../api.h"

#include <motor/math/primitive/3d/ray.hpp>
#include <motor/math/primitive/3d/frustum.hpp>
#include <motor/math/utility/3d/transformation.hpp>
#include <motor/math/camera/3d/perspective_fov.hpp>
#include <motor/std/vector>

#include <array>

namespace motor
{
    namespace gfx
    {
        // A generic camera class storing lenses for rendering 
        // through those lenses. The camera has a frame for spacial 
        // positioning and the lenes have a transformation too.
        // @todo each lens needs an offset so a stereoscopic camera
        // can be created by adding two lenses and defining the offset.
        class MOTOR_GFX_API generic_camera
        {
            motor_this_typedefs( generic_camera ) ;

            motor_typedefs( motor::math::m3d::ray3f_t, ray3 ) ;
            motor_typedefs( motor::math::vec2f_t, vec2 ) ;
            motor_typedefs( motor::math::vec3f_t, vec3 ) ;
            motor_typedefs( motor::math::m3d::frustum<float_t>, frustum ) ;
            motor_typedefs( frustum_t::plane_t, plane ) ;

        private:
            
            enum class projection_type
            {
                undefined,
                orthographic,
                perspective
            };

            projection_type _projection_mode = projection_type::undefined ;

        private:

            motor::math::mat4f_t _cam_matrix ;
            motor::math::mat4f_t _view_matrix ;
            motor::math::mat4f_t _proj_matrix ;

            float_t _fov ;

            // in pixels
            motor::math::vec2f_t _sensor_dims ;
            motor::math::vec2f_t _near_far ;
            frustum_t _frustum ;

        private:

            motor::math::m3d::trafof_t _trafo ;

        public: // 

            generic_camera( void_t ) noexcept ;
            generic_camera( float_t const w, float_t const h, float_t const n, float_t const f ) noexcept ;

            generic_camera( this_rref_t rhv ) noexcept ;

            generic_camera( this_cref_t rhv ) noexcept ;
            
            virtual ~generic_camera( void_t ) noexcept ;

            this_ref_t operator = ( this_cref_t ) noexcept ;
            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            // @note does not recompute the projection matrix.
            void_t set_dims( float_t const w, float_t const h, float_t const n, float_t const f ) noexcept
            {
                _sensor_dims = motor::math::vec2f_t( w, h ) ;
                _near_far = motor::math::vec2f_t( n, f ) ;
            }

            void_t set_sensor_dims( float_t const w, float_t const h ) noexcept
            {
                _sensor_dims = motor::math::vec2f_t( w, h ) ;
            }

            // returns ( w, h, n, f )
            motor::math::vec4f_t get_dims( void_t ) const noexcept
            {
                return motor::math::vec4f_t( _sensor_dims, _near_far ) ;
            }

            // this will be a orthographic camera 
            // this function changes the projection matrix
            this_ref_t make_orthographic( float_t const w, float_t const h,
                float_t const n, float_t const f ) noexcept ;

            // this will be a perspective camera
            // this function changes the projection matrix
            this_ref_t make_perspective_fov( float_t const w, float_t const h,
                float_t const fov, float_t const n, float_t const f ) noexcept  ;

            // will create a orthographic camera and return it
            static this_t create_orthographic( float_t const w, float_t const h,
                float_t const n, float_t const f ) noexcept ;

            // will create a perspective camera and return it
            static this_t create_perspective_fov( float_t const w, float_t const h, 
                float_t const fov, float_t const n, float_t const f ) noexcept  ;

            // just changes the projection to orthographic
            this_ref_t orthographic( void_t ) noexcept
            {
                this_t::make_orthographic( _sensor_dims.x(), _sensor_dims.y(), 
                    _near_far.x(), _near_far.y() ) ;
                return *this ;
            }

            // just changes the projection to perspective 
            this_ref_t perspective_fov( void_t ) noexcept
            {
                this_t::make_perspective_fov( _sensor_dims.x(), _sensor_dims.y(), 
                    _fov, _near_far.x(), _near_far.y() ) ;
                return *this ;
            }


            this_ref_t perspective_fov( float_t const fov ) noexcept
            {
                this_t::make_perspective_fov( _sensor_dims.x(), _sensor_dims.y(), fov, 
                   _near_far.x(), _near_far.y() ) ;

                return *this ;
            }

            bool_t is_perspective( void_t ) const noexcept ;

            bool_t is_orthographic( void_t ) const noexcept ;

            // creates and changes the camera matrix and the view matrix
            this_ref_t look_at( motor::math::vec3f_cref_t pos,
                motor::math::vec3f_cref_t up, motor::math::vec3f_cref_t at ) noexcept ;

        public: // get

            vec3_t get_position( void_t ) const noexcept ;

            this_cref_t get_position( vec3_ref_t pos ) const noexcept ;

            vec3_t get_direction( void_t ) const noexcept ;

            this_cref_t get_direction( vec3_ref_t dir ) const noexcept ;

            vec3_t get_up( void_t ) const noexcept ;

            this_cref_t get_up( vec3_ref_t up ) const noexcept ;

            vec3_t get_right( void_t ) const noexcept ;

            this_cref_t get_right( vec3_ref_t right ) const noexcept ;

            frustum_cref_t get_frustum( void_t ) const noexcept ;

            float_t get_fov( void_t ) const noexcept
            {
                return _fov ;
            }

            // returns w/h aspect
            float_t aspect_w_h( void_t ) const noexcept
            {
                return _sensor_dims.x() / _sensor_dims.y() ;
            }

            // returns h/w aspect
            float_t aspect_h_w( void_t ) const noexcept
            {
                return _sensor_dims.y() / _sensor_dims.x() ;
            }

            motor::math::mat4f_cref_t get_view_matrix( void_t ) const noexcept ;
            motor::math::mat4f_t mat_view( void_t ) const noexcept
            {
                return this_t::get_view_matrix() ;
            }


            motor::math::mat4f_cref_t get_proj_matrix( void_t ) const noexcept ;
            motor::math::mat4f_t mat_proj( void_t ) const noexcept
            {
                return this_t::get_proj_matrix() ;
            }

            motor::math::mat4f_cref_t get_camera_matrix( void_t ) const noexcept ;
            motor::math::mat4f_t mat_cam( void_t ) const noexcept
            {
                return this_t::get_camera_matrix() ;
            }

            motor::math::vec4f_t near_far_plane_half_dims( void_t ) const noexcept ;
            motor::math::vec4f_t near_far_plane_half_dims( motor::math::vec2f_in_t ) const noexcept ;

            motor::math::vec2f_t get_near_far( void_t ) const noexcept
            {
                return _near_far ;
            }

        public: // ray

            /// creates a ray in view center direction.
            /// @param norm_pos [in] [-1,1] screen position
            ray3_t create_ray_norm( vec2_cref_t norm_pos ) const noexcept ;

            /// creates a ray in view center direction.
            ray3_t create_center_ray( void_t ) const ;

            /// creates a ray in view center direction.
            /// @param screen_pos [in] [(0,0),(width,height)] screen position.
            ray3_t create_ray( vec2_cref_t screen_pos ) const ;

        public: // some positioning

            this_ref_t translate_by( motor::math::vec3f_cref_t d ) noexcept
            {
                motor::math::m3d::trafof_t t ;
                t.translate_fr( d ) ;
                this_t::transform_by( t ) ;
                return *this ;
            }

            this_ref_t translate_to( motor::math::vec3f_cref_t d ) noexcept
            {
                auto t = this_t::get_transformation() ;
                t.set_translation( d ) ;
                this_t::set_transformation( t ) ;
                return *this ;
            }

        public: // interface

            
            void_t transform_by( motor::math::m3d::trafof_cref_t trafo ) noexcept  ;
            void_t set_transformation( motor::math::m3d::trafof_cref_t trafo ) noexcept ;

            motor::math::m3d::trafof_cref_t get_transformation( void_t ) const noexcept ;

        private:

            void_t update_view_matrix( motor::math::mat4f_cref_t frame ) noexcept ;
            void_t reconstruct_frustum_planes( void_t ) noexcept ;

        };
        motor_typedef( generic_camera ) ;
    }
}