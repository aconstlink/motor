
#include "trafo_visitor.h"
#include "../node/camera_node.h"
#include "../node/trafo3d_node.h"
#include "../node/render_node.h"
#include "../node/leaf.h"

#include "../component/trafo3d_component.h"

using namespace motor::scene;

//******************************************************************************
trafo_visitor::trafo_visitor( void_t ) noexcept {}

//******************************************************************************
trafo_visitor::~trafo_visitor( void_t ) noexcept {}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );

    if( auto * rptr = dynamic_cast< motor::scene::render_node_ptr_t >( nptr ); rptr != nullptr )
    {
        rptr->set_world( _trafos.top() );
    }
    else if( auto * cptr = dynamic_cast< motor::scene::camera_node_ptr_t >( nptr ); cptr != nullptr )
    {
        auto * ptr = cptr->borrow_camera();
        if( ptr != nullptr )
        {
            ptr->transform_by( _trafos.top() );
        }
    }

    this_t::handle_post_visit( nptr );

    return motor::scene::result::ok;
}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    return motor::scene::result::ok;
}

//******************************************************************************
motor::scene::result trafo_visitor::post_visit( motor::scene::group_ptr_t nptr, motor::scene::result const ) noexcept
{
    this_t::handle_post_visit( nptr );
    return motor::scene::result::ok;
}

//******************************************************************************
void_t trafo_visitor::handle_visit( motor::scene::node_ptr_t nptr )
{
    auto * tcomp = nptr->borrow_component< motor::scene::trafo3d_component_t >();
    if( tcomp == nullptr ) return;

    auto const local = tcomp->get_trafo();
    auto const trafo = _trafos.top() * local;
    motor::scene::trafo3d_component_t::visitor_interface( tcomp ).set_computed( trafo );

    // push in any case, so the post_visit function
    // can just pop the trafo from the stack
    _trafos.push( trafo );
}

//******************************************************************************
void_t trafo_visitor::handle_post_visit( motor::scene::node_ptr_t nptr )
{
    auto * tcomp = nptr->borrow_component< motor::scene::trafo3d_component_t >();
    if( tcomp == nullptr ) return;

    // pop transformation
    _trafos.pop();
}

#if 0
//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::render_node_ptr_t nptr ) noexcept 
{
    nptr->set_world( _trafos.top() ) ;
    return motor::scene::result::ok ;
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
#endif
//******************************************************************************
void_t trafo_visitor::on_start( void_t ) noexcept
{
    // ensure at least one transformation is on the stack
    _trafos.push( motor::math::m3d::trafof_t() );
}

//******************************************************************************
void_t trafo_visitor::on_finish( void_t ) noexcept
{
    _trafos.pop();
    assert( !_trafos.has_item() );
}