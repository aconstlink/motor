// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../default_visitor.h"
#include "../../component/light_component.hpp"
#include "../../component/trafo3d_component.h"
#include "../../component/bounds_component.hpp"

#include "../../node/node.h"

#include <motor/gfx/camera/generic_camera.h>
#include <motor/gfx/primitive/primitive_render_3d.h>

namespace motor
{
namespace scene
{

class debug_graphics_visitor : public motor::scene::ivisitor
{
    motor_this_typedefs( debug_graphics_visitor );

  private:

    motor::gfx::generic_camera_mtr_t _cam = nullptr;
    motor::gfx::primitive_render_3d_mtr_t _prim_render = nullptr;

    size_t _draw_level = 0;
    size_t _count_depth = 0;
    size_t _max_depth = 0;

    bool_t _draw_leafes = false;

  public:

    struct draw_context
    {
        motor::gfx::generic_camera_mtr_t cam = nullptr;
        motor::gfx::primitive_render_3d_mtr_t pr = nullptr;

        size_t draw_level = 0;
        bool_t draw_leafs = false;
    };
    motor_typedef( draw_context );

  public:

    debug_graphics_visitor( draw_context_cref_t dc ) noexcept
        : _prim_render( dc.pr ), _cam( dc.cam ), _draw_leafes( dc.draw_leafs ),
        _draw_level( dc.draw_level )
    {
        assert( _prim_render != nullptr );
        assert( _cam != nullptr );
    }

    debug_graphics_visitor( this_cref_t ) = delete;

    debug_graphics_visitor( this_rref_t rhv ) noexcept
        : _prim_render( motor::move( rhv._prim_render ) ), _cam( motor::move( rhv._cam ) ),
          _count_depth( rhv._count_depth ), _draw_leafes( rhv._draw_leafes ),
          _draw_level( rhv._draw_level )
    {
    }

    virtual ~debug_graphics_visitor( void_t ) noexcept {}

    // during traversal, the visitor counts the depth.
    // this function allows to get that max depth.
    size_t get_max_depth( void_t ) const noexcept
    {
        return _max_depth;
    }

  public:

    virtual motor::scene::result visit( motor::scene::group_ptr_t nptr ) noexcept
    {
        this_t::do_visit( nptr, _count_depth == _draw_level );
        _max_depth = std::max( ++_count_depth, _max_depth );
        return motor::scene::result::ok;
    }

    virtual motor::scene::result post_visit(
        motor::scene::group_ptr_t nptr, motor::scene::result const res ) noexcept
    {
        --_count_depth;
        return res;
    }

    virtual motor::scene::result visit( motor::scene::leaf_ptr_t nptr ) noexcept
    {
        this_t::do_visit( nptr, _count_depth == _draw_level || _draw_leafes );
        return motor::scene::result::ok;
    }

  private:

    virtual motor::scene::result do_visit(
        motor::scene::node_ptr_t nptr, bool_t const draw_bounds ) noexcept
    {
        motor::math::vec3f_t pos;

        // lets try to get a position for the light
        {
            if( auto * tcomp = nptr->borrow_component< motor::scene::trafo3d_component_t >();
                tcomp )
            {
                pos = tcomp->get_computed().get_translation();
            }
        }

        // light component
        {
            motor::scene::light_component_mtr_t cptr;
            if( nptr->has_component_and_borrow< motor::scene::light_component_t >( cptr ) )
            {
                cptr->for_each_light( [ & ]( motor::gfx::light_mtr_t l )
                {
                    if( l->get_light_type() == motor::gfx::light_type::directional )
                    {
                        auto * ll = static_cast< motor::gfx::directional_light * >( l );
                        auto const dir = ll->get_transformed_direction();

                        _prim_render->draw_arrow(
                            pos, dir, 10.0f, motor::math::vec4f_t( 0.0f, 1.0f, 0.0f, 1.0f ) );
                    }
                    else if( l->get_light_type() == motor::gfx::light_type::point )
                    {
                        auto * ll = static_cast< motor::gfx::point_light_t * >( l );
                        auto const lpos = ll->get_transformed_position();

                        float_t const r_0 = 0.5f;
                        float_t const r_1 =
                            ll->get_radius() <= 0.0f ? r_0 * 2.0f : ll->get_radius();

                        _prim_render->draw_circle( _cam->get_camera_matrix(), lpos, r_0,
                            motor::math::vec4f_t( 0.0f, 1.0f, 0.0f, 1.0f ),
                            motor::math::vec4f_t( 0.0f, 1.0f, 0.0f, 1.0f ),

                            10 );

                        _prim_render->draw_wire_circle( _cam->get_camera_matrix(), lpos, r_1,
                            motor::math::vec4f_t( 0.0f, 1.0f, 0.0f, 1.0f ), 20 );
                    }
                } );
            }
        }

        // bounds component
        if( draw_bounds )
        {
            motor::scene::bounds_component_ptr_t cptr;
            if( nptr->has_component_and_borrow< motor::scene::bounds_component_t >( cptr ) )
            {
                auto const & b = cptr->get_coarse_transformed();
                _prim_render->draw_wire_cube(
                    b.points(), motor::math::vec4f_t( 1.0f, 1.0f, 1.0f, 1.0f ) );
            }
        }

        // camera component
        {
        }

        return motor::scene::result::ok;
    }
};
motor_typedef( debug_graphics_visitor );
} // namespace scene
} // namespace motor