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

class debug_graphics_visitor : public motor::scene::default_visitor
{
    motor_this_typedefs( debug_graphics_visitor );

  public:

    motor::gfx::generic_camera_mtr_t _cam = nullptr;
    motor::gfx::primitive_render_3d_mtr_t _prim_render = nullptr;

  public:

    debug_graphics_visitor(
        motor::gfx::generic_camera_mtr_t cam, motor::gfx::primitive_render_3d_mtr_t pr ) noexcept
        : _prim_render( pr ), _cam( cam )
    {
        assert( _prim_render != nullptr );
        assert( _cam != nullptr );
    }

    debug_graphics_visitor( this_cref_t ) = delete;

    debug_graphics_visitor( this_rref_t rhv ) noexcept
        : _prim_render( motor::move( rhv._prim_render ) ), _cam( motor::move( rhv._cam ) )
    {
    }

    virtual ~debug_graphics_visitor( void_t ) noexcept {}

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
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

    virtual motor::scene::result post_visit(
        motor::scene::node_ptr_t, motor::scene::result const res ) noexcept
    {
        return res;
    }
};
motor_typedef( debug_graphics_visitor );
} // namespace scene
} // namespace motor