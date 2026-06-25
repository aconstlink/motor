
#include "primitive_render_3d.h"

using namespace motor::gfx;

//*****************************************************************
primitive_render_3d::primitive_render_3d( void_t ) {}

//*****************************************************************
primitive_render_3d::primitive_render_3d( this_rref_t rhv )
{
    _lr = std::move( rhv._lr );
    _tr = std::move( rhv._tr );
}

//*****************************************************************
primitive_render_3d::~primitive_render_3d( void_t ) {}

//*****************************************************************
void_t primitive_render_3d::init( motor::string_cref_t name ) noexcept
{
    _name = name;

    _lr = motor::gfx::line_render_3d_t();
    _lr.init( name + ".line_render" );

    _tr = motor::gfx::tri_render_3d_t();
    _tr.init( name + ".tri_render" );
}

//*****************************************************************
void_t primitive_render_3d::release( void_t ) noexcept
{
    _lr.release();
    _tr.release();
}

//*****************************************************************
void_t primitive_render_3d::draw_line( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1,
    motor::math::vec4f_cref_t color ) noexcept
{
    _lr.draw( p0, p1, color );
}

//*****************************************************************
void_t primitive_render_3d::draw_lines(
    size_t const num_lines, motor::gfx::line_render_3d_t::draw_lines_funk_t f ) noexcept
{
    _lr.draw_lines( num_lines, f );
}

//*****************************************************************
void_t primitive_render_3d::draw_tri( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1,
    motor::math::vec3f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept
{
    _tr.draw( p0, p1, p2, color );
}

#if 0
//*****************************************************************
void_t primitive_render_3d::draw_rect(  motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, motor::math::vec4f_cref_t color, 
    motor::math::vec4f_cref_t border_color ) noexcept 
{
    _tr.draw_rect( p0, p1, p2, p3, color ) ;
    _lr.draw_rect( p0, p1, p2, p3, border_color ) ;
}
#endif

//*****************************************************************
void_t primitive_render_3d::draw_circle( motor::math::mat3f_cref_t o, motor::math::vec3f_cref_t p0,
    float_t const r, motor::math::vec4f_cref_t color, motor::math::vec4f_cref_t /*border_color*/,
    size_t const s ) noexcept
{
    _tr.draw_circle( o, p0, r, color, s );
    //_lr.draw_circle( s, p0, r, border_color ) ;
}

//*****************************************************************
void_t primitive_render_3d::draw_spline(
    motor::math::linear_bezier_spline< motor::math::vec3f_t > const & spline,
    motor::math::vec4f_cref_t color ) noexcept
{
    if( spline.num_control_points() <= 1 ) return;

    size_t const num_steps = spline.num_control_points() - 1;
    this_t::draw_lines( num_steps, [ & ]( size_t const i )
    {
        auto const v0 = spline.control_points()[ i + 0 ];
        auto const v1 = spline.control_points()[ i + 1 ];

        return motor::gfx::line_render_3d::draw_line_data{ v0, v1, color };
    } );
}

//*****************************************************************
void_t primitive_render_3d::draw_spline( size_t const num_samples,
    motor::math::cubic_hermit_spline< motor::math::vec3f_t > const & spline,
    motor::math::vec4f_cref_t color ) noexcept
{
    if( spline.ncp() <= 1 ) return;

    size_t const num_steps = num_samples;
    this_t::draw_lines( num_steps, [ & ]( size_t const i )
    {
        float_t const t0 = float_t( i + 0 ) / float_t( num_steps - 1 );
        float_t const t1 = float_t( i + 1 ) / float_t( num_steps - 1 );

        auto const v0 = spline( t0 );
        auto const v1 = spline( t1 );

        return motor::gfx::line_render_3d::draw_line_data{ v0, v1, color };
    } );
}

//*****************************************************************
void_t primitive_render_3d::draw_frustum(
    motor::gfx::generic_camera_cref_t cam, motor::math::vec4f_cref_t color )
{
    // 0-3 : front plane
    // 4-7 : back plane
    motor::math::vec3f_t points[ 8 ];

    // auto const nf = cam->get_near_far() ;
    auto const nf = motor::math::vec2f_t( 50.0f, 1000.0f );

    auto const cs = cam.near_far_plane_half_dims( nf );
    {
        motor::math::vec3f_t const scale( cs.x(), cs.y(), nf.x() );
        points[ 0 ] = motor::math::vec3f_t( -1.0f, -1.0f, 1.0f ) * scale;
        points[ 1 ] = motor::math::vec3f_t( -1.0f, +1.0f, 1.0f ) * scale;
        points[ 2 ] = motor::math::vec3f_t( +1.0f, +1.0f, 1.0f ) * scale;
        points[ 3 ] = motor::math::vec3f_t( +1.0f, -1.0f, 1.0f ) * scale;
    }

    {
        motor::math::vec3f_t const scale( cs.z(), cs.w(), nf.y() );
        points[ 4 ] = motor::math::vec3f_t( -1.0f, -1.0f, 1.0f ) * scale;
        points[ 5 ] = motor::math::vec3f_t( -1.0f, +1.0f, 1.0f ) * scale;
        points[ 6 ] = motor::math::vec3f_t( +1.0f, +1.0f, 1.0f ) * scale;
        points[ 7 ] = motor::math::vec3f_t( +1.0f, -1.0f, 1.0f ) * scale;
    }

    for( size_t i = 0; i < 8; ++i )
    {
        points[ i ] = ( cam.get_transformation().get_transformation() *
                        motor::math::vec4f_t( points[ i ], 1.0f ) )
                          .xyz();
    }

    // front
    for( size_t i = 0; i < 4; ++i )
    {
        size_t const i0 = i + 0;
        size_t const i1 = ( i + 1 ) % 4;
        this_t::draw_line( points[ i0 ], points[ i1 ], color );
    }

    // back
    for( size_t i = 0; i < 4; ++i )
    {
        size_t const i0 = ( i + 0 ) + 4;
        size_t const i1 = ( ( i + 1 ) % 4 ) + 4;
        this_t::draw_line( points[ i0 ], points[ i1 ], color );
    }

    // sides
    for( size_t i = 0; i < 4; ++i )
    {
        size_t const i0 = ( i + 0 );
        size_t const i1 = ( i + 4 ) % 8;
        this_t::draw_line( points[ i0 ], points[ i1 ], color );
    }
}

//*****************************************************************
void_t primitive_render_3d::configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept
{
    _lr.configure( fe );
    _tr.configure( fe );
}

//*****************************************************************
void_t primitive_render_3d::prepare_for_rendering( void_t ) noexcept
{
    _lr.prepare_for_rendering();
    _tr.prepare_for_rendering();
}

//*****************************************************************
void_t primitive_render_3d::prepare_for_rendering(
    motor::graphics::gen4::frontend_mtr_t fe ) noexcept
{
    _lr.prepare_for_rendering( fe );
    _tr.prepare_for_rendering( fe );
}

//*****************************************************************
void_t primitive_render_3d::render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept
{
    _lr.render( fe );
    _tr.render( fe );
}

//*****************************************************************
void_t primitive_render_3d::set_view_proj(
    motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept
{
    _tr.set_view_proj( view, proj );
    _lr.set_view_proj( view, proj );
}