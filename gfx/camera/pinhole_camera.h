
#pragma once

#include "generic_camera.h"

#include <motor/math/camera/3d/camera_util.hpp>

namespace motor
{
    namespace gfx
    {
        // this class simplyfies the use of the pinhole lens and 
        // the generic camera. It has one pinhole lens and a generic camera.
        class pinhole_camera
        {
            motor_this_typedefs( pinhole_camera ) ;

            motor::gfx::generic_camera_t _camera ;

            motor::math::vec4f_t _wh_nf = motor::math::vec4f_t( 1.0f, 1.0f, 1.0f, 1000.0f ) ;

        public:

            pinhole_camera( void_t ) noexcept
            {
            }

            pinhole_camera( this_rref_t rhv ) noexcept
            {
                *this = std::move( rhv ) ;
            }

            pinhole_camera( this_cref_t rhv ) = delete ;

            ~pinhole_camera( void_t ) noexcept
            {}

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _camera = std::move( rhv._camera ) ;
                _wh_nf = rhv._wh_nf ;

                return *this ;
            }

        public:

            this_ref_t set_sensor_dims( float_t const w, float_t const h ) noexcept
            {
                _wh_nf = motor::math::vec4f_t( w, h, _wh_nf.zw() ) ;
                return *this ;
            }

            this_ref_t set_near_far( float_t const n, float_t const f ) noexcept
            {
                _wh_nf = motor::math::vec4f_t( _wh_nf.xy(), n, f ) ;
                return *this ;
            }

            this_ref_t set_dims( float_t const w, float_t const h, float_t const n, float_t const f ) noexcept
            {
                _wh_nf = motor::math::vec4f_t( w, h, n, f ) ;
                return *this ;
            }

            this_ref_t orthographic( void_t  ) noexcept
            {
                _camera.make_orthographic( _wh_nf.x(), _wh_nf.y(), _wh_nf.z(), _wh_nf.w() ) ;
                return *this ;
            }

            this_ref_t perspective_fov( float_t const fov ) noexcept
            {
                _camera.make_perspective_fov( _wh_nf.x(),_wh_nf.y(), fov, _wh_nf.x() / _wh_nf.y(), _wh_nf.z(), _wh_nf.w() ) ;
                return *this ;
            }

            this_ref_t perspective_fov( void_t ) noexcept
            {
                _camera.make_perspective_fov( _wh_nf.x(),_wh_nf.y(), this_t::fov(), _wh_nf.x() / _wh_nf.y(), _wh_nf.z(), _wh_nf.w() ) ;
                return *this ;
            }

            this_ref_t translate_by( motor::math::vec3f_cref_t d ) noexcept
            {
                motor::math::m3d::trafof_t t ;
                t.translate_fr( d ) ;
                _camera.transform_by( t ) ;
                return *this ;
            }

            this_ref_t translate_to( motor::math::vec3f_cref_t d ) noexcept
            {
                auto t = _camera.get_transformation() ;
                t.set_translation( d ) ;
                _camera.set_transformaion( t ) ;
                return *this ;
            }

            motor::math::vec3f_t get_position( void_t ) const noexcept
            {
                return _camera.get_transformation().get_translation() ;
            }

            float_t fov( void_t ) const noexcept
            {
                return _camera.get_fov() ;
            }

            this_ref_t look_at( motor::math::vec3f_cref_t pos,
                motor::math::vec3f_cref_t up, motor::math::vec3f_cref_t at ) noexcept
            {
                motor::math::mat4f_t m ;
                motor::math::m3d::create_lookat_rh( pos, up, at, m ) ;

                _camera.set_transformaion( m ) ;

                return *this ;
            }

            bool_t is_perspective( void_t ) const noexcept
            {
                return _camera.is_perspective() ;
            }

            bool_t is_orthographic( void_t ) const noexcept
            {
                return _camera.is_orthographic() ;
            }

            #if 0
            motor::gfx::generic_camera_res_t get_camera( void_t ) noexcept
            {
                return _camera ;
            }
            #endif
        public:

            motor::math::mat4f_t mat_proj( void_t ) const noexcept
            {
                return _camera.get_proj_matrix() ;
            }

            motor::math::mat4f_t mat_view( void_t ) const noexcept
            {
                return _camera.get_view_matrix() ;
            }

            motor::math::m3d::trafof_cref_t get_transformation( void_t ) const noexcept
            {
                return _camera.get_transformation() ;
            }

            void_t set_transformation( motor::math::m3d::trafof_cref_t t ) noexcept
            {
                _camera.set_transformaion( t ) ;
            }

            float_t aspect( void_t ) const noexcept
            {
                return _wh_nf.y() / _wh_nf.x() ;
            }
        };
        motor_typedef( pinhole_camera ) ;
    }
}