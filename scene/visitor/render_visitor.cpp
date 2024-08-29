

#include "render_visitor.h"

using namespace motor::scene ;


//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::node_ptr_t ) noexcept 
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::leaf_ptr_t ) noexcept
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::group_ptr_t ) noexcept
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
void_t render_visitor::on_start( void_t ) noexcept 
{
}

//*****************************************************************************************
void_t render_visitor::on_finish( void_t ) noexcept 
{
}