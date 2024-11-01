
#include "variable_update_visitor.h"

#include "../component/variables_component.hpp"
#include "../node/node.h"

using namespace motor::scene ;

motor_core_dd_id_init( variable_update_visitor ) ;

//*************************************************************************************************
variable_update_visitor::variable_update_visitor( void_t ) noexcept 
{
}

//*************************************************************************************************
variable_update_visitor::~variable_update_visitor( void_t ) noexcept 
{
}

//*************************************************************************************************
motor::scene::result variable_update_visitor::visit( motor::scene::ivisitable_ptr_t vable ) noexcept
{
    if( auto * n = dynamic_cast< motor::scene::node_ptr_t >( vable ); n != nullptr )
    {
        auto * comp = n->borrow_component< motor::scene::variables_component_t >() ;
        // make stuff with variables

        return motor::scene::result::ok ;
    }

    //variables_component_ptr_t
    return motor::scene::result::ok ;
}

//*************************************************************************************************
motor::scene::result variable_update_visitor::post_visit( motor::scene::ivisitable_ptr_t, motor::scene::result const ) noexcept
{
    return motor::scene::result::ok ;
}