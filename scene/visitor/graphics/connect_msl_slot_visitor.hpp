#pragma once

#include "../default_visitor.h"

#include "../../node/group.h"
#include "../../node/leaf.h"

#include "../../component/graphics/msl_set_component.hpp"
#include "../../component/graphics/geometry_name_component.hpp"

#include <motor/application/typedefs.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>
#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
// search a node by a name component. It will break the traversal
// if the first occurance is found.
class connect_msl_slot_visitor : public default_visitor
{
    motor_this_typedefs( connect_msl_slot_visitor );

    motor::scene::msl_set_component_t::id_t _id = motor::scene::msl_set_component_t::invalid_id();

    using connection_funk_t = std::function< void_t( motor::wire::inputs_ref_t ) >;

    // called when msl is added and new variable
    // set is created for poluting the set with variables.
    connection_funk_t _cf = []( motor::wire::inputs_ref_t ) {
    };

  public:

    connect_msl_slot_visitor(
        motor::scene::msl_set_component_t::id_t const id, connection_funk_t cf ) noexcept
        : _id( id ), _cf( std::move( cf ) )
    {
    }

    connect_msl_slot_visitor( this_rref_t rhv ) noexcept
        : _id( rhv._id ), _cf( std::move( rhv._cf ) )
    {
    }

    connect_msl_slot_visitor( this_cref_t ) = delete;

    virtual ~connect_msl_slot_visitor( void_t ) noexcept {}

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        motor::scene::msl_set_component_mtr_t comp;
        if( nptr->has_component_and_borrow< motor::scene::msl_set_component_t >( comp ) )
        {
            motor::scene::msl_component_mtr_t msl_comp;
            if( comp->borrow_msl_component( _id, msl_comp ) )
            {
                auto * inp = msl_comp->borrow_shader_inputs();
                _cf( *inp );
            }
        }

        return motor::scene::result::ok;
    }

    virtual motor::scene::result post_visit(
        motor::scene::node_ptr_t, motor::scene::result const res ) noexcept
    {
        return res;
    }

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
motor_typedef( connect_msl_slot_visitor );
} // namespace scene
} // namespace motor