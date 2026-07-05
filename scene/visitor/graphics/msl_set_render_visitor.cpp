

#include "msl_set_render_visitor.h"

#include "../../component/graphics/msl_component.h"
#include "../../component/graphics/msl_set_component.hpp"
#include "../../component/graphics/render_settings_component.h"
#include "../../component/graphics/config_graphics_component.h"

using namespace motor::scene;

//*****************************************************************************************
msl_set_render_visitor::msl_set_render_visitor( motor::application::window_id_t const wid,
    motor::scene::msl_set_component_t::id_t const msl_set_id,
    motor::graphics::gen4::frontend_ptr_t fe, motor::gfx::generic_camera_ptr_t cam ) noexcept
    : _wid( wid ), _msl_set_id( msl_set_id ), _fe( fe ), _cam( cam )
{
}

//*****************************************************************************************
msl_set_render_visitor::msl_set_render_visitor( this_rref_t rhv ) noexcept
    : _wid( rhv._wid ), _msl_set_id( rhv._msl_set_id ), _fe( motor::move( rhv._fe ) ),
      _cam( motor::move( rhv._cam ) )
{
}

//*****************************************************************************************
msl_set_render_visitor::~msl_set_render_visitor( void_t ) noexcept {}

//*****************************************************************************************
motor::scene::result msl_set_render_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    this_t::handle_post_visit( nptr );

    return motor::scene::result::ok;
}

//*****************************************************************************************
motor::scene::result msl_set_render_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
motor::scene::result msl_set_render_visitor::post_visit(
    motor::scene::group_ptr_t nptr, motor::scene::result const ) noexcept
{
    this_t::handle_post_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
void_t msl_set_render_visitor::handle_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    // configurations
    {
        auto * comp = nptr->borrow_component< motor::scene::config_graphics_component_t >();
        if( comp != nullptr )
        {
            if( comp->init_and_cleanup( this_t::wid(), _fe ) )
            {
                // nothing to do right now
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
    {
        auto * set_comp = nptr->borrow_component< motor::scene::msl_set_component_t >();
        if( set_comp != nullptr && set_comp->init_msl( this_t::wid(), _msl_set_id, _fe ) )
        {
            motor::scene::msl_component_mtr_t comp;
            if( set_comp->borrow_msl_component( _msl_set_id, comp ) )
            {
                comp->render_update( _cam );

                auto msl = comp->borrow_msl();

                {
                    motor::graphics::gen4::backend_t::render_detail_t detail;
                    detail.start = 0;
                    // detail.num_elems = 3 ;
                    detail.geo = comp->get_geo_idx() == size_t( -1 ) ? 0 : comp->get_geo_idx();
                    detail.varset = comp->get_variable_set_idx();
                    _fe->render( msl, detail );
                }
            }
        }
    }
}

//*****************************************************************************************
void_t msl_set_render_visitor::handle_post_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
    if( comp == nullptr ) return;

    _fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
}

//*****************************************************************************************
void_t msl_set_render_visitor::on_start( void_t ) noexcept {}

//*****************************************************************************************
void_t msl_set_render_visitor::on_finish( void_t ) noexcept {}