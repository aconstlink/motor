

#include "render_visitor.h"

#include "../component/msl_component.h"
#include "../component/render_settings_component.h"

using namespace motor::scene;

//*****************************************************************************************
render_visitor::render_visitor( motor::graphics::gen4::frontend_ptr_t fe,
                                motor::gfx::generic_camera_ptr_t cam ) noexcept
    : _fe( fe ), _cam( cam )
{
}

//*****************************************************************************************
render_visitor::render_visitor( this_rref_t rhv ) noexcept
    : _fe( motor::move( rhv._fe ) ), _cam( motor::move( rhv._cam ) )
{
}

//*****************************************************************************************
render_visitor::~render_visitor( void_t ) noexcept {}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    this_t::handle_post_visit( nptr );

    return motor::scene::result::ok;
}

//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
motor::scene::result render_visitor::post_visit( motor::scene::group_ptr_t nptr, motor::scene::result const ) noexcept
{
    this_t::handle_post_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
void_t render_visitor::handle_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    {
        auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
        if( comp != nullptr )
        {
            _fe->push( comp->borrow_state() );
        }
    }

    // do the primary purpose of this visitor
    // => handle the renderable msl component.
    {
        auto * comp = nptr->borrow_component< motor::scene::msl_component_t >() ;
        if( comp != nullptr )
        {
            comp->render_update( _cam );

            auto msl = comp->borrow_msl();

            {
                motor::graphics::gen4::backend_t::render_detail_t detail;
                detail.start = 0;
                // detail.num_elems = 3 ;
                detail.varset = comp->get_variable_set_idx();
                _fe->render( msl, detail );
            }
        }
    }

    #if 0
    if( auto * rptr = dynamic_cast< motor::scene::render_node_ptr_t >( nptr ); rptr != nullptr )
    {
        rptr->render_update( _cam );

        auto msl = rptr->borrow_msl();

        {
            motor::graphics::gen4::backend_t::render_detail_t detail;
            detail.start = 0;
            // detail.num_elems = 3 ;
            detail.varset = rptr->get_variable_set_idx();
            _fe->render( msl, detail );
        }
    }
    #endif
}

//*****************************************************************************************
void_t render_visitor::handle_post_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
    if( comp == nullptr ) return;

    _fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
}

#if 0
//*****************************************************************************************
motor::scene::result render_visitor::visit( motor::scene::render_node_ptr_t nptr ) noexcept  
{
    nptr->render_update(  _cam ) ;

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
#endif
//*****************************************************************************************
void_t render_visitor::on_start( void_t ) noexcept {}

//*****************************************************************************************
void_t render_visitor::on_finish( void_t ) noexcept {}