

#include "render_visitor.h"

#include "../node/render_node.h"
#include "../node/render_settings.h"
#include "../component/msl_component.h"

using namespace motor::scene ;

motor_core_dd_id_init( render_visitor ) ;

//*****************************************************************************************
render_visitor::render_visitor( motor::graphics::gen4::frontend_ptr_t fe ) noexcept : _fe( fe )
{
}

//*****************************************************************************************
render_visitor::render_visitor( this_rref_t rhv ) noexcept : _fe( motor::move( rhv._fe ) )
{
}

//*****************************************************************************************
render_visitor::~render_visitor( void_t ) noexcept
{
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::render_node_ptr_t nptr ) noexcept  
{
    auto msl = nptr->borrow_msl() ;

    {
        motor::graphics::gen4::backend_t::render_detail_t detail ;
        detail.start = 0 ;
        //detail.num_elems = 3 ;
        detail.varset = nptr->get_variable_set_idx() ;
        _fe->render( msl, detail ) ;
    }

    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::render_settings_ptr_t nptr ) noexcept
{
    _fe->push( nptr->borrow_state() ) ;
    return motor::scene::result::ok ;
}

//*****************************************************************************************
motor::scene::result render_visitor::post_visit( motor::scene::render_settings_ptr_t, motor::scene::result const ) noexcept
{
    _fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
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