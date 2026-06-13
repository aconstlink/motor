

#include "render_visitor.h"

#include "../component/msl_component.h"
#include "../component/render_settings_component.h"
#include "../component/config_graphics_component.h"

using namespace motor::scene;

//*****************************************************************************************
render_visitor::render_visitor( size_t const wid, motor::graphics::gen4::frontend_ptr_t fe,
                                motor::gfx::generic_camera_ptr_t cam ) noexcept
    : _wid( wid ), _fe( fe ), _cam( cam )
{
}

//*****************************************************************************************
render_visitor::render_visitor( this_rref_t rhv ) noexcept
    : _wid( rhv._wid ), _fe( motor::move( rhv._fe ) ), _cam( motor::move( rhv._cam ) )
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
    bool_t can_render = true;

    // configurations
    {
        auto * comp = nptr->borrow_component< motor::scene::config_graphics_component_t >();
        if( comp != nullptr )
        {
            can_render = false;

            if( comp->init_and_cleanup( this_t::wid(), _fe ) )
            {
                // cleaned up
                can_render = true;
            }
        }
    }

    // render settings
    {
        auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
        if( comp != nullptr )
        {
            _fe->push( comp->borrow_state() );
        }
    }

    // do the primary purpose of this visitor
    // => handle the renderable msl component.
    if( can_render )
    {
        auto * comp = nptr->borrow_component< motor::scene::msl_component_t >();
        if( comp != nullptr )
        {
            comp->render_update( _cam );

            auto msl = comp->borrow_msl();

            {
                motor::graphics::gen4::backend_t::render_detail_t detail;
                detail.start = 0;
                // detail.num_elems = 3 ;
                detail.geo = comp->get_geo_idx() ;
                detail.varset = comp->get_variable_set_idx();
                _fe->render( msl, detail );
            }
        }
    }
}

//*****************************************************************************************
void_t render_visitor::handle_post_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
    if( comp == nullptr ) return;

    _fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
}

//*****************************************************************************************
void_t render_visitor::on_start( void_t ) noexcept {}

//*****************************************************************************************
void_t render_visitor::on_finish( void_t ) noexcept {}