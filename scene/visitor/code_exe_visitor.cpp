#include "../node/node.h"
#include "../component/code_component.h"
#include "code_exe_visitor.h"

using namespace motor::scene ;

//*********************************************************************
motor::scene::result code_exe_visitor::visit( motor::scene::node_ptr_t nptr ) noexcept
{
    motor::scene::code_component_ptr_t comp ;
    if( nptr->borrow_component( comp ) )
    {
        comp->execute( nptr ) ;
    }

    return motor::scene::result::ok ;
}

//*********************************************************************
motor::scene::result code_exe_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    return this_t::visit( motor::scene::node_ptr_t(nptr) )  ;
}

//*********************************************************************
motor::scene::result code_exe_visitor::post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept
{
    return motor::scene::result::ok ;
}

//*********************************************************************
motor::scene::result code_exe_visitor::visit( motor::scene::decorator_ptr_t nptr ) noexcept
{
    return this_t::visit( motor::scene::node_ptr_t( nptr ) )  ;
}

//*********************************************************************
motor::scene::result code_exe_visitor::post_visit( motor::scene::decorator_ptr_t, motor::scene::result const ) noexcept
{
    return motor::scene::result::ok ;
}

//*********************************************************************
motor::scene::result code_exe_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    return this_t::visit( motor::scene::node_ptr_t( nptr ) )  ;
}

//*********************************************************************