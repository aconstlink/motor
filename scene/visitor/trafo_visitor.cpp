
#include "trafo_visitor.h"
#include "../node/camera_node.h"
#include "../node/trafo3d_node.h"
#include "../node/leaf.h"

using namespace motor::scene ;

//******************************************************************************
trafo_visitor::trafo_visitor( void_t ) noexcept 
{
}

//******************************************************************************
trafo_visitor::~trafo_visitor( void_t ) noexcept 
{
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::trafo3d_node_ptr_t g ) noexcept
{
    auto const local = g->get_trafo() ;
    auto const trafo = _trafos.top() * local ;
    motor::scene::trafo3d_node_t::visitor_interface( g ).set_computed( trafo ) ;
    
    // push in any case, so the post_visit function 
    // can just pop the trafo from the stack
    _trafos.push( trafo ) ;

    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::post_visit( motor::scene::trafo3d_node_ptr_t, motor::scene::result const ) noexcept
{
    // pop transformation
    _trafos.pop() ;

    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::camera_node_ptr_t nptr ) noexcept 
{ 
    auto * ptr = nptr->borrow_camera() ;
    if( ptr != nullptr )
    {
        ptr->transform_by( _trafos.top() ) ;
    }
    return motor::scene::result::ok ; 
}

//******************************************************************************
void_t trafo_visitor::on_start( void_t ) noexcept 
{
    // ensure at least one transformation is on the stack
    _trafos.push( motor::math::m3d::trafof_t() ) ;
}

//******************************************************************************
void_t trafo_visitor::on_finish( void_t ) noexcept 
{    
    _trafos.pop() ;
    assert( !_trafos.has_item() ) ;
}