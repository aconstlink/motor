
#include "trafo_visitor.h"
#include "../node/leaf.h"

#include "../component/trafo3d_component.h"
#include "../component/camera_component.h"
#include "../component/msl_component.h"

using namespace motor::scene;

//******************************************************************************
trafo_visitor::trafo_visitor( void_t ) noexcept {}

//******************************************************************************
trafo_visitor::~trafo_visitor( void_t ) noexcept {}

//******************************************************************************
motor::scene::result trafo_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
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
    // trafo3d
    {
        auto * tcomp = nptr->borrow_component< motor::scene::trafo3d_component_t >();
        if( tcomp != nullptr )
        {
            auto const local = tcomp->get_trafo();
            auto const trafo = _trafos.top() * local;
            motor::scene::trafo3d_component_t::visitor_interface( tcomp ).set_computed( trafo );

            // push in any case, so the post_visit function
            // can just pop the trafo from the stack
            _trafos.push( trafo );
        }
    }

    // camera
    {
        auto * comp = nptr->borrow_component< motor::scene::camera_component_t >();
        if( comp != nullptr )
        {
            if( auto * ptr = comp->borrow_camera(); ptr != nullptr )
            {
                ptr->transform_by( _trafos.top() );
            }
        }
    }

    //
    {
        auto * comp = nptr->borrow_component< motor::scene::msl_component_t >();
        if( comp != nullptr )
        {
            comp->set_world( _trafos.top() );
        }
    }
}

//******************************************************************************
void_t trafo_visitor::handle_post_visit( motor::scene::node_ptr_t nptr )
{
    auto * tcomp = nptr->borrow_component< motor::scene::trafo3d_component_t >();
    if( tcomp == nullptr ) return;

    // pop transformation
    _trafos.pop();
}

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