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
class add_msl_to_set_visitor : public default_visitor
{
    motor_this_typedefs( add_msl_to_set_visitor );

    motor::scene::msl_set_component_t::id_t _id = motor::scene::msl_set_component_t::invalid_id();

    motor::graphics::msl_object_mtr_t _msl;
    motor::graphics::command_status_mtr_t _status;

  public:

    add_msl_to_set_visitor( motor::scene::msl_set_component_t::id_t const id,
        motor::graphics::msl_object_mtr_t msl ) noexcept
        : _id( id ), _msl( motor::move( msl ) )
    {
    }

    add_msl_to_set_visitor( motor::scene::msl_set_component_t::id_t const id,
        motor::graphics::msl_object_mtr_safe_t msl,
        motor::graphics::command_status_mtr_safe_t status ) noexcept
        : _id( id ), _msl( motor::move( msl ) ), _status( motor::move( status ) )
    {
    }

    add_msl_to_set_visitor( this_rref_t rhv ) noexcept
        : _id( rhv._id ), _msl( motor::move( rhv._msl ) )
    {
    }

    add_msl_to_set_visitor( this_cref_t ) = delete;

    virtual ~add_msl_to_set_visitor( void_t ) noexcept {}

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        motor::scene::msl_set_component_mtr_t comp;
        if( nptr->has_component_and_borrow< motor::scene::msl_set_component_t >( comp ) )
        {

            motor::scene::msl_component_mtr_t msl_comp;
            if( comp->borrow_msl_component( _id, msl_comp ) )
            {
                // there is already a msl component.
            }
            else
            {
                size_t geo_idx = size_t( -1 );

                motor::scene::geometry_name_component_mtr_t geo_comp;
                if( nptr->has_component_and_borrow( geo_comp ) )
                {
                    geo_idx = _msl->link_geometry( geo_comp->get_name() );
                }
                else
                {
                    // do not have any geometry, so we can not
                    // link the shader to any geoemtry.
                }

                size_t const vs =
                    _msl->add_variable_set( motor::shared( motor::graphics::variable_set_t() ) );

                motor::scene::msl_component_t new_comp(
                    motor::share( _msl ), motor::share( _status ), vs, geo_idx );

                comp->add_component( _id, motor::shared( std::move( new_comp ) ) );
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
motor_typedef( add_msl_to_set_visitor );
} // namespace scene
} // namespace motor