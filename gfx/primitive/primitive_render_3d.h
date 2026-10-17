// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "line_render_3d.h"
#include "tri_render_3d.h"

#include <motor/gfx/camera/generic_camera.h>

#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/math/spline/linear_bezier_spline.hpp>
#include <motor/math/spline/cubic_bezier_spline.hpp>
#include <motor/math/spline/cubic_hermit_spline.hpp>
#include <motor/math/primitive/3d/frustum.hpp>

namespace motor
{
namespace gfx
{
class MOTOR_GFX_API primitive_render_3d
{
    motor_this_typedefs( primitive_render_3d );

  private: // other render

    motor::string_t _name;

    motor::gfx::line_render_3d_t _lr;
    motor::gfx::tri_render_3d_t _tr;

  public:

    primitive_render_3d( void_t );
    primitive_render_3d( this_cref_t ) = delete;
    primitive_render_3d( this_rref_t );
    ~primitive_render_3d( void_t );

    void_t init( motor::string_cref_t ) noexcept;
    void_t release( void_t ) noexcept;

  public:

    void_t draw_line( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1,
        motor::math::vec4f_cref_t color ) noexcept;

    void_t draw_lines(
        size_t const num_lines, motor::gfx::line_render_3d_t::draw_lines_funk_t f ) noexcept;

    void_t draw_tri( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1,
        motor::math::vec3f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept;

#if 0
            void_t draw_rect( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, 
                motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, 
                motor::math::vec4f_cref_t color, motor::math::vec4f_cref_t border_color ) noexcept ;
#endif
    void_t draw_circle( motor::math::mat3f_cref_t, motor::math::vec3f_cref_t p0, float_t const r,
        motor::math::vec4f_cref_t color, motor::math::vec4f_cref_t border_color,
        size_t const ) noexcept;

    // right/up must form an orthonormal basis of the circle's plane.
    // Non-positive radii or fewer than three segments emit no lines.
    void_t draw_wire_circle( motor::math::vec3f_cref_t center,
        motor::math::vec3f_cref_t right, motor::math::vec3f_cref_t up, float_t const radius,
        motor::math::vec4f_cref_t color, size_t const num_segments = 32 ) noexcept;

    // The orientation transforms a circle in the local XY plane.
    void_t draw_wire_circle( motor::math::mat3f_cref_t orientation,
        motor::math::vec3f_cref_t center, float_t const radius,
        motor::math::vec4f_cref_t color, size_t const num_segments = 32 ) noexcept;

    // Three world-axis-aligned great circles, not a tessellated sphere.
    void_t draw_wire_sphere( motor::math::vec3f_cref_t center, float_t const radius,
        motor::math::vec4f_cref_t color, size_t const num_segments = 32 ) noexcept;

    // Direction is normalized internally. Zero direction or non-positive length emits nothing.
    // Head dimensions are in world units; head_length is clamped to length.
    void_t draw_arrow( motor::math::vec3f_cref_t origin, motor::math::vec3f_cref_t direction,
        float_t const length, motor::math::vec4f_cref_t color,
        float_t const head_length = 0.35f, float_t const head_radius = 0.18f ) noexcept;

    void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept;

  public: // splines

    // draws from control point to control point.
    void_t draw_spline( motor::math::linear_bezier_spline< motor::math::vec3f_t > const &,
        motor::math::vec4f_cref_t spline_color ) noexcept;

    // draws the spline as lines.
    // @param num_samples how many times to sample the spline for line creation.
    void_t draw_spline( size_t const num_samples,
        motor::math::cubic_hermit_spline< motor::math::vec3f_t > const &,
        motor::math::vec4f_cref_t color ) noexcept;

    template < typename spline_t >
    void_t draw_control_points( spline_t const & spline, float_t const radius,
        size_t const num_segments, motor::math::vec4f_cref_t control_point_color )
    {
        auto const points = spline.control_points();
        for( size_t i = 0; i < points.size(); ++i )
        {
            auto const pos = points[ i ];
            this_t::draw_circle( motor::math::mat3f_t::make_identity(), pos, radius,
                control_point_color, control_point_color, num_segments );
        }
    }

  public: // frustum

      void_t draw_frustum( motor::gfx::generic_camera_cref_t cam, motor::math::vec4f_cref_t color ) ;

  public:

    void_t configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept;
    // copy all data to the gpu buffer and transmit the data
    void_t prepare_for_rendering( void_t ) noexcept;
    void_t prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept;
    void_t render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept;
};
motor_typedef( primitive_render_3d );
} // namespace gfx
} // namespace motor