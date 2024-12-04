
#include "generic_camera.h"

#include <motor/math/camera/3d/camera_util.hpp>
#include <motor/math/camera/3d/perspective_fov.hpp>
#include <motor/math/camera/3d/orthographic_projection.hpp>
#include <motor/math/utility/3d/transformation.hpp>

#include <motor/math/primitive/3d/frustum.hpp>

using namespace motor ;
using namespace motor::gfx ;

//*********************************************
generic_camera::generic_camera( void_t ) noexcept 
{
    _cam_matrix.identity() ;
    _view_matrix.identity() ;
    _proj_matrix.identity() ;
}

//*********************************************
generic_camera::generic_camera( float_t const w, float_t const h, float_t const n, float_t const f ) noexcept 
{
    this_t::set_dims( w, h, n, f ) ;
}

//*********************************************
generic_camera::generic_camera( this_cref_t rhv ) noexcept 
{
    _trafo = ( rhv._trafo ) ;
    _cam_matrix = rhv._cam_matrix ;
    _view_matrix = rhv._view_matrix ;
    _proj_matrix = rhv._proj_matrix ;
    _projection_mode = rhv._projection_mode ;

    _fov = rhv._fov ;
    _near_far = rhv._near_far ;
    _sensor_dims = rhv._sensor_dims ;

    _frustum = rhv._frustum ;
}

//*********************************************
generic_camera::generic_camera( this_rref_t rhv ) noexcept
{
    _trafo = std::move( rhv._trafo ) ;
    _cam_matrix = rhv._cam_matrix ;
    _view_matrix = rhv._view_matrix ;
    _proj_matrix = rhv._proj_matrix ;
    _projection_mode = rhv._projection_mode ;

    _fov = rhv._fov ;
    _near_far = rhv._near_far ;
    _sensor_dims = rhv._sensor_dims ;

    _frustum = rhv._frustum ;
}

//*********************************************
generic_camera::~generic_camera( void_t ) noexcept 
{
}

//*********************************************
generic_camera::this_ref_t generic_camera::operator = ( this_rref_t rhv ) noexcept 
{
    _trafo = std::move( rhv._trafo ) ;
    _cam_matrix = rhv._cam_matrix ;
    _view_matrix = rhv._view_matrix ;
    _proj_matrix = rhv._proj_matrix ;
    _projection_mode = rhv._projection_mode ;

    _fov = rhv._fov ;
    _near_far = rhv._near_far ;
    _sensor_dims = rhv._sensor_dims ;

    _frustum = rhv._frustum ;

    return *this ;
}

//*********************************************
generic_camera::this_ref_t generic_camera::operator = ( this_cref_t rhv ) noexcept 
{
    _trafo =  rhv._trafo ;
    _cam_matrix = rhv._cam_matrix ;
    _view_matrix = rhv._view_matrix ;
    _proj_matrix = rhv._proj_matrix ;
    _projection_mode = rhv._projection_mode ;

    _fov = rhv._fov ;
    _near_far = rhv._near_far ;
    _sensor_dims = rhv._sensor_dims ;

    _frustum = rhv._frustum ;

    return *this ;
}

//*********************************************
void_t generic_camera::transform_by( motor::math::m3d::trafof_cref_t trafo ) noexcept
{
    _trafo = _trafo * trafo ;
    this->update_view_matrix( _trafo.get_transformation() ) ;
}

//*********************************************
void_t generic_camera::set_transformation( motor::math::m3d::trafof_cref_t trafo ) noexcept
{
    _trafo = trafo ;
    this->update_view_matrix( trafo.get_transformation() ) ;
}

//*********************************************
motor::math::m3d::trafof_cref_t generic_camera::get_transformation( void_t ) const noexcept
{
    return _trafo ;
}

//*********************************************
generic_camera::this_ref_t generic_camera::make_orthographic( float_t const w, float_t const h,
    float_t const n, float_t const f ) noexcept 
{
    _proj_matrix = motor::math::m3d::orthographic<float_t>::create(
        w, h, n, f ) ;

    _projection_mode = projection_type::orthographic ;
    _near_far = motor::math::vec2f_t( n, f ) ;
    _sensor_dims = motor::math::vec2f_t( w, h ) ;
    return *this ;
}

//*********************************************
generic_camera::this_ref_t generic_camera::make_perspective_fov( float_t const w, float_t const h, float_t const fov,
    float_t const n, float_t const f ) noexcept 
{
    motor::math::m3d::perspective<float_t>::create_by_fovv_aspect(
        fov, w/h, n, f, _proj_matrix ) ;

    _projection_mode = projection_type::perspective ;
    _fov = fov ;
    _near_far = motor::math::vec2f_t( n, f ) ;
    _sensor_dims = motor::math::vec2f_t( w, h ) ;

    return *this ;
}

//*********************************************
generic_camera::this_t generic_camera::create_orthographic( float_t const w, float_t const h,
    float_t const n, float_t const f ) noexcept 
{
    return std::move( this_t().make_orthographic( w, h, n, f ) ) ;
}

//*********************************************
generic_camera::this_t generic_camera::create_perspective_fov( float_t const w, float_t const h, float_t const fov,
    float_t const n, float_t const f ) noexcept 
{
    return this_t().make_perspective_fov( w, h, fov, n, f ) ;
}

//*********************************************
bool_t generic_camera::is_perspective( void_t ) const noexcept
{
    return _projection_mode == projection_type::perspective ;
}

//*********************************************
bool_t generic_camera::is_orthographic( void_t ) const noexcept
{
    return _projection_mode == projection_type::orthographic ;
}

//*********************************************
void_t generic_camera::update_view_matrix( motor::math::mat4f_cref_t frame ) noexcept 
{
    _cam_matrix = frame ;
    motor::math::m3d::create_view_matrix( frame, _view_matrix ) ;
    this_t::reconstruct_frustum_planes() ;
}

//*********************************************
generic_camera::this_ref_t generic_camera::look_at( motor::math::vec3f_cref_t pos,
    motor::math::vec3f_cref_t up, motor::math::vec3f_cref_t at ) noexcept
{
    motor::math::mat4f_t m ;
    motor::math::m3d::create_lookat_rh( pos, up, at, m ) ;

    this_t::set_transformation( m ) ;

    return *this ;
}

//*********************************************
motor::math::mat4f_cref_t generic_camera::get_view_matrix( void_t ) const noexcept 
{
    return _view_matrix ;
}

//*********************************************
motor::math::mat4f_cref_t generic_camera::get_proj_matrix( void_t ) const noexcept 
{
    return _proj_matrix ;
}

//*********************************************
motor::math::mat4f_cref_t generic_camera::get_camera_matrix( void_t ) const noexcept 
{
    return _cam_matrix ;
}

generic_camera::vec3_t generic_camera::get_position( void_t ) const noexcept
{
    vec3_t pos ;
    motor::math::m3d::get_pos_from_camera_matrix( pos, _cam_matrix ) ;
    return pos ;
}

generic_camera::this_cref_t generic_camera::get_position( vec3_ref_t pos ) const noexcept
{
    motor::math::m3d::get_pos_from_camera_matrix( pos, _cam_matrix ) ;
    return *this ;
}

generic_camera::vec3_t generic_camera::get_direction( void_t ) const noexcept
{
    vec3_t dir ;
    motor::math::m3d::get_dir_from_camera_matrix<float_t>( dir, _cam_matrix ) ;
    return dir ;
}

generic_camera::this_cref_t generic_camera::get_direction( vec3_ref_t dir ) const noexcept
{
    motor::math::m3d::get_dir_from_camera_matrix<float_t>( dir, _cam_matrix ) ;
    return *this ;
}

generic_camera::vec3_t generic_camera::get_up( void_t ) const noexcept
{
    vec3_t up ;
    motor::math::m3d::get_up_from_camera_matrix<float_t>( up, _cam_matrix ) ;
    return up ;
}

generic_camera::this_cref_t generic_camera::get_up( vec3_ref_t up ) const noexcept
{
    motor::math::m3d::get_up_from_camera_matrix<float_t>( up, _cam_matrix ) ;
    return *this ;
}

generic_camera::vec3_t generic_camera::get_right( void_t ) const noexcept
{
    vec3_t right ;
    motor::math::m3d::get_right_from_camera_matrix<float_t>( right, _cam_matrix ) ;
    return right ;
}

generic_camera::this_cref_t generic_camera::get_right( vec3_ref_t right ) const noexcept
{
    motor::math::m3d::get_right_from_camera_matrix<float_t>( right, _cam_matrix ) ;
    return *this ;
}

generic_camera::frustum_cref_t generic_camera::get_frustum( void_t ) const noexcept 
{
    return _frustum ;
}

//******************************************************************************************
motor::math::vec4f_t generic_camera::near_far_plane_half_dims( void_t ) const noexcept 
{
    return this_t::near_far_plane_half_dims( _near_far ) ;
}

//******************************************************************************************
motor::math::vec4f_t generic_camera::near_far_plane_half_dims( motor::math::vec2f_in_t nf ) const noexcept 
{
    auto const nl = motor::math::m3d::detail::perspective_projection<float_t>::
        get_half_length_for( _fov, nf.x() ) ;
    auto const fl = motor::math::m3d::detail::perspective_projection<float_t>::
        get_half_length_for( _fov, nf.y() ) ;

    auto const aspect = this_t::aspect_h_w() ;
    return motor::math::vec4f_t( nl, nl * aspect, fl, fl * aspect ) ;
}

//******************************************************************************************
generic_camera::ray3_t generic_camera::create_ray_norm( vec2_cref_t norm_pos ) const noexcept
{
    if( _projection_mode == projection_type::perspective )
    {
        float_t const w = motor::math::get_width_at<float_t>( _fov, this_t::aspect_w_h(), 1.0f ) ;
        float_t const h = motor::math::get_height_at<float_t>( _fov, 1.0f ) ;
        return ray3_t( this_t::get_position(), _cam_matrix * vec3_t( norm_pos * vec2_t( w, h ) * 0.5f, 1.0f ).normalize() ) ;
    }

    auto const po = _cam_matrix * motor::math::vec3f_t( norm_pos * _sensor_dims * 0.5f, 0.0f ) ;
    return ray3_t( this_t::get_position() + po, _cam_matrix * motor::math::vec3f_t(0.0f,0.0f,1.0f) ) ;
}

generic_camera::ray3_t generic_camera::create_center_ray( void_t ) const
{
    return this_t::create_ray_norm( vec2_t( 0.0f, 0.0f ) ) ;
    return ray3_t( this_t::get_position(), get_direction() ) ;
}

generic_camera::ray3_t generic_camera::create_ray( vec2_cref_t screen_pos ) const
{
    vec2_t norm_coord = screen_pos / vec2_t( _sensor_dims.x(), _sensor_dims.y() ) ;
    norm_coord = norm_coord * 2.0f - 1.0f ;
    return create_ray_norm( norm_coord ) ;
}

void_t generic_camera::reconstruct_frustum_planes( void_t ) noexcept
{
    motor::math::mat4f_t comb = _proj_matrix * _view_matrix ;

    motor::math::vec4f_t r0, r1, r2, r3 ;

    comb.get_row( 0, r0 ) ;
    comb.get_row( 1, r1 ) ;
    comb.get_row( 2, r2 ) ;
    comb.get_row( 3, r3 ) ;

    motor::math::vec4f_t const p0 = ( r3 + r0 ).normalized() ;
    motor::math::vec4f_t const p1 = ( r3 - r0 ).normalized() ;
    motor::math::vec4f_t const p2 = ( r3 + r1 ).normalized() ;
    motor::math::vec4f_t const p3 = ( r3 - r1 ).normalized() ;
    motor::math::vec4f_t const p4 = ( r3 + r2 ).normalized() ;
    motor::math::vec4f_t const p5 = ( r3 - r2 ).normalized() ;

    _frustum.set_plane( frustum_t::frustum_plane::left_plane, plane_t( p0.negated() ) ) ;
    _frustum.set_plane( frustum_t::frustum_plane::right_plane, plane_t( p1.negated() ) ) ;
    _frustum.set_plane( frustum_t::frustum_plane::bottom_plane, plane_t( p2.negated() ) ) ;
    _frustum.set_plane( frustum_t::frustum_plane::top_plane, plane_t( p3.negated() ) ) ;
    _frustum.set_plane( frustum_t::frustum_plane::near_plane, plane_t( p4.negated() ) ) ;
    _frustum.set_plane( frustum_t::frustum_plane::far_plane, plane_t( p5.negated() ) ) ;
}