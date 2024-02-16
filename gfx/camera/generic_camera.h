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
            float_t _aspect ;

            // in pixels
            motor::math::vec2f_t _sensor_dims ;
            motor::math::vec2f_t _near_far ;
            frustum_t _frustum ;

        private:

            motor::math::m3d::trafof_t _trafo ;

        public: // 

            generic_camera( void_t ) noexcept ;

            generic_camera( this_rref_t rhv ) noexcept ;

            generic_camera( this_cref_t rhv ) noexcept ;
            
            virtual ~generic_camera( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            this_ref_t make_orthographic( float_t const w, float_t const h,
                float_t const n, float_t const f ) noexcept ;

            this_ref_t make_perspective_fov( float_t const w, float_t const h,
                float_t const fov, float_t const aspect,
                float_t const n, float_t const f ) noexcept  ;

            static this_t create_orthographic( float_t const w, float_t const h,
                float_t const n, float_t const f ) noexcept ;

            static this_t create_perspective_fov( float_t const w, float_t const h, 
                float_t const fov, float_t const aspect,
                float_t const n, float_t const f ) noexcept  ;

            bool_t is_perspective( void_t ) const noexcept ;

            bool_t is_orthographic( void_t ) const noexcept ;

            void_t update_view_matrix( motor::math::mat4f_cref_t frame ) noexcept ;

            motor::math::mat4f_cref_t get_view_matrix( void_t ) const noexcept ;

            motor::math::mat4f_cref_t get_proj_matrix( void_t ) const noexcept ;

            float_t get_fov( void_t ) const noexcept { return _fov ; }
            
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

        public: // ray

            /// creates a ray in view center direction.
            /// @param norm_pos [in] [-1,1] screen position
            ray3_t create_ray_norm( vec2_cref_t norm_pos ) const noexcept ;

            /// creates a ray in view center direction.
            ray3_t create_center_ray( void_t ) const ;

            /// creates a ray in view center direction.
            /// @param screen_pos [in] [(0,0),(width,height)] screen position.
            ray3_t create_ray( vec2_cref_t screen_pos ) const ;

        public: // interface

            
            size_t get_num_lenses( void_t ) const noexcept ;

            void_t transform_by( motor::math::m3d::trafof_cref_t trafo ) noexcept  ;
            void_t set_transformaion( motor::math::m3d::trafof_cref_t trafo ) noexcept ;

            motor::math::m3d::trafof_cref_t get_transformation( void_t ) const noexcept ;

        private:

            
            void_t reconstruct_frustum_planes( void_t ) noexcept ;

        };
        motor_typedef( generic_camera ) ;
    }
}