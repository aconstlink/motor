
#include "variable_update_visitor.h"

#include "../component/variables_component.hpp"
#include "../node/node.h"

using namespace motor::scene;

//*************************************************************************************************
variable_update_visitor::variable_update_visitor( void_t ) noexcept {}

//*************************************************************************************************
variable_update_visitor::~variable_update_visitor( void_t ) noexcept {}

//*************************************************************************************************
motor::scene::result variable_update_visitor::visit( motor::scene::node_ptr_t nptr ) noexcept
{
    nptr->for_each_component( [ & ]( motor::scene::icomponent_ptr_t cptr )
    { motor::scene::icomponent::visitor_sync_accessor::sync_inputs( cptr ); } );
    
    return motor::scene::result::ok;
}

//*************************************************************************************************
motor::scene::result variable_update_visitor::post_visit( motor::scene::node_ptr_t,
                                                          motor::scene::result const ) noexcept
{
    return motor::scene::result::ok;
}