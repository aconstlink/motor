
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "default_visitor.h"

#include "../node/group.h"
#include "../node/leaf.h"

#include "../component/name_component.hpp"

namespace motor
{
namespace scene
{
// runs over the scene graph and calls the lambda for each
// node found. return motor::scene::result::break_traversal if
// you like to break the traversal
class for_each_node_visitor : public default_visitor
{
    motor_this_typedefs( for_each_node_visitor );

  public:

    struct for_each_data
    {
        std::string_view name;
        motor::scene::node_mtr_t node;
    };
    using for_each_node_funk_t =
        std::function< motor::scene::result( this_t::for_each_data const & ) >;

  private:

    for_each_node_funk_t _f = []( this_t::for_each_data const & )
    {
        return motor::scene::result::break_traversal;
    };

  public:

    for_each_node_visitor( for_each_node_funk_t && f ) noexcept : _f( std::move( f ) ) {}
    for_each_node_visitor( this_rref_t rhv ) noexcept : _f( std::move( rhv._f ) ) {}
    for_each_node_visitor( this_cref_t ) = delete;
    virtual ~for_each_node_visitor( void_t ) noexcept {}

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        motor::string_t name;
        {
            motor::scene::name_component_mtr_t cptr;
            if( nptr->has_component_and_borrow< motor::scene::name_component_t >( cptr ) )
            {
                name = cptr->get_name();
            }
        }

        return _f( this_t::for_each_data{ name.c_str(), nptr } );
    }
    virtual motor::scene::result post_visit(
        motor::scene::node_ptr_t, motor::scene::result const res ) noexcept
    {
        return res;
    }

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
motor_typedef( for_each_node_visitor );
} // namespace scene
} // namespace motor