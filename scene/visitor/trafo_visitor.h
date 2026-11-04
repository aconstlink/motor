// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "ivisitor.h"
#include "../node/group.h"
#include "../node/leaf.h"

#include <motor/collide/3d/aabb.hpp>
#include <motor/math/utility/3d/transformation.hpp>
#include <motor/std/stack>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API trafo_visitor : public ivisitor
{
    motor_this_typedefs( trafo_visitor );

  private:

    motor::stack< motor::math::m3d::trafof_t, 10 > _trafos;
    motor::stack< motor::collide::aabb3f_t, 10 > _coarses;

    // will be created if the traversal is done
    motor::collide::aabb3f_t _world_box;

  public:

    trafo_visitor( void_t ) noexcept;
    virtual ~trafo_visitor( void_t ) noexcept;

    virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept;
    virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept;
    virtual motor::scene::result post_visit(
        motor::scene::group_ptr_t, motor::scene::result const ) noexcept;

    virtual void_t on_start( void_t ) noexcept final;
    virtual void_t on_finish( void_t ) noexcept final;

    motor::collide::aabb3f_t get_world_box( void_t ) const noexcept
    {
        return _world_box;
    }

  private:

    void_t handle_visit( motor::scene::node_ptr_t, bool_t const update_bounds ) noexcept;
    void_t handle_post_visit( motor::scene::node_ptr_t ) noexcept;
};
motor_typedef( trafo_visitor );
} // namespace scene
} // namespace motor