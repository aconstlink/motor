#pragma once

#include "default_visitor.h"

#include "../node/group.h"
#include "../node/leaf.h"

#include "../component/name_component.hpp"

namespace motor
{
namespace scene
{
// search a node by a name component. It will break the traversal
// if the first occurance is found.
class MOTOR_SCENE_API search_by_name_visitor : public default_visitor
{
    motor_this_typedefs( search_by_name_visitor );

    motor::string_t _name;
    motor::scene::node_mtr_t _found_node; // managed

  public:

    search_by_name_visitor( motor::string_cref_t name ) noexcept : _name( name ) {}

    search_by_name_visitor( this_rref_t rhv ) noexcept
        : _name( std::move( rhv._name ) ), _found_node( motor::move( rhv._found_node ) )
    {
    }
    search_by_name_visitor( this_cref_t ) = delete;
    virtual ~search_by_name_visitor( void_t ) noexcept
    {
        motor::release( motor::move( _found_node ) );
    }

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        {
            motor::scene::name_component_mtr_t cptr;
            if( nptr->has_component_and_borrow< motor::scene::name_component_t >( cptr ) )
            {
                if( cptr->get_name() == _name )
                {
                    motor::release( motor::move( _found_node ) );
                    _found_node = motor::share( nptr );
                }
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

  public:

    motor::scene::node_mtr_t move_found_node( void_t ) noexcept
    {
        return motor::move( _found_node );
    }
};
motor_typedef( search_by_name_visitor );
} // namespace scene
} // namespace motor