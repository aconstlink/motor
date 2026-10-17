// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "default_visitor.h"

#include "../node/node.h"
#include "../component/icomponent.h"

namespace motor
{
namespace scene
{
class variable_update_visitor : public motor::scene::default_visitor
{
    motor_this_typedefs( variable_update_visitor );

  public:

    variable_update_visitor( void_t ) noexcept {}
    virtual ~variable_update_visitor( void_t ) noexcept {}

  private:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept override
    {
        nptr->for_each_component( [ & ]( motor::scene::icomponent_ptr_t cptr )
        { motor::scene::icomponent::visitor_sync_accessor::sync_inputs( cptr ); } );

        return motor::scene::result::ok;
    }
    virtual motor::scene::result post_visit(
        motor::scene::node_ptr_t nptr, motor::scene::result const res ) noexcept override
    {
        return motor::scene::result::ok;
    }

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
motor_typedef( variable_update_visitor );
} // namespace scene
} // namespace motor