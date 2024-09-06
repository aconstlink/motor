
#include "trafo_visitor.h"
#include "../node/group.h"
#include "../node/leaf.h"

#include "../component/trafo_components.hpp"

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
motor::scene::result trafo_visitor::visit( motor::scene::node_ptr_t node_ptr ) noexcept 
{
    this_t::check_component_and_compute_global( node_ptr ) ;
    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::leaf_ptr_t node_ptr ) noexcept
{
    this_t::check_component_and_compute_global( node_ptr ) ;
    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::group_ptr_t g ) noexcept
{
    // push in any case, so the post_visit function 
    // can just pop the trafo from the stack
    _trafos.push( this_t::check_component_and_compute_global( g ) ) ;

    return motor::scene::result::ok ;
}

//******************************************************************************
motor::scene::result trafo_visitor::post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept
{
    // pop transformation
    _trafos.pop() ;

    return motor::scene::result::ok ;
}

//******************************************************************************
void_t trafo_visitor::on_start( void_t ) noexcept 
{
    _trafos.push( motor::math::m3d::trafof_t() ) ;
}

//******************************************************************************
void_t trafo_visitor::on_finish( void_t ) noexcept 
{    
    _trafos.pop() ;
    assert( !_trafos.has_item() ) ;
}

//******************************************************************************
motor::math::m3d::trafof_t trafo_visitor::check_component_and_compute_global( motor::scene::node_ptr_t node_ptr ) noexcept 
{
    motor::math::m3d::trafof_t trafo ;

    // check transformation component.
    // if there is one, do:
    // #1 compute new global transformation
    // #2 push to stack for children
    // #3 store global trafo in component for other visitors
    {
        motor::scene::trafo_3d_component_ptr_t comp_ptr  ;
        if ( node_ptr->borrow_component<motor::scene::trafo_3d_component>( comp_ptr ) )
        {
            auto const local = comp_ptr->get_trafo() ;
            trafo = _trafos.top() * local ;
            motor::scene::trafo_3d_component_t::visitor_interface( comp_ptr ).set_computed( trafo ) ;
        }
    }

    return trafo ;
}