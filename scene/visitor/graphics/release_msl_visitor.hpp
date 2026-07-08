#pragma once

#include "../default_visitor.h"

#include "../../node/group.h"
#include "../../node/leaf.h"

#include "../../component/graphics/msl_set_component.hpp"

#include <motor/application/typedefs.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

namespace motor
{
namespace scene
{
// search a node by a name component. It will break the traversal
// if the first occurance is found.
class release_msl_visitor : public default_visitor
{
    motor_this_typedefs( release_msl_visitor );

    motor::application::window_id_t _wid = motor::application::invalid_window_id();
    motor::scene::msl_set_component_t::id_t _id = motor::scene::msl_set_component_t::invalid_id();
    motor::graphics::gen4::frontend_ptr_t _fe = nullptr;

  public:

    release_msl_visitor( motor::application::window_id_t const wid,
        motor::scene::msl_set_component_t::id_t const id,
        motor::graphics::gen4::frontend_ptr_t fe ) noexcept
        : _wid( wid ), _id( id ), _fe( fe )
    {
    }

    release_msl_visitor( this_rref_t rhv ) noexcept
        : _wid( rhv._wid ), _id( rhv._id ), _fe( motor::move( rhv._fe ) )
    {
    }
    release_msl_visitor( this_cref_t ) = delete;
    virtual ~release_msl_visitor( void_t ) noexcept {}

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        {
            motor::scene::msl_set_component_mtr_t cptr;
            if( nptr->has_component_and_borrow< motor::scene::msl_set_component_t >( cptr ) )
            {
                cptr->release_msl( _wid, _id, _fe );
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
motor_typedef( release_msl_visitor );
} // namespace scene
} // namespace motor