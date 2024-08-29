
#include "trafo_visitor.h"

using namespace motor::scene ;

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::node_ptr_t ) noexcept 
{
    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::leaf_ptr_t ) noexcept
{
    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::group_ptr_t ) noexcept
{
    // push transformation
    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept
{
    // pop transformation
    return motor::scene::result::ok ;
}

//******************************************************************************