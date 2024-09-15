

#include "render_visitor.h"

#include "../node/leaf.h"
#include "../component/msl_component.h"

using namespace motor::scene ;


//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::node_ptr_t ) noexcept 
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::leaf_ptr_t ptr ) noexcept
{
    motor::scene::msl_component_ptr_t comp ;
    if( ptr->borrow_component( comp ) )
    {
        
    }

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
motor::scene::result render_visitor::visit( motor::scene::camera_node_ptr_t ) noexcept 
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::trafo3d_node_ptr_t ) noexcept
{
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::post_visit( motor::scene::trafo3d_node_ptr_t, motor::scene::result const ) noexcept
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