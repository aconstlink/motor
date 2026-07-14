

#include "render_visitor.h"

#include "../../component/graphics/msl_component.h"
#include "../../component/graphics/msl_set_component.hpp"
#include "../../component/graphics/render_settings_component.hpp"
#include "../../component/graphics/config_graphics_component.h"

using namespace motor::scene;

//*****************************************************************************************
render_visitor::render_visitor( size_t const wid, motor::scene::msl_set_component_t::id_t const id,
    motor::graphics::gen4::frontend_ptr_t fe, motor::gfx::generic_camera_ptr_t cam ) noexcept
    : _wid( wid ), _msl_set_id( id ), _fe( fe ), _cam( cam )
{
}

//*****************************************************************************************
render_visitor::render_visitor( size_t const wid, motor::graphics::gen4::frontend_ptr_t fe,
    motor::gfx::generic_camera_ptr_t cam ) noexcept
    : _wid( wid ), _msl_set_id( 0 ), _fe( fe ), _cam( cam )
{
}

//*****************************************************************************************
render_visitor::render_visitor( this_rref_t rhv ) noexcept
    : _wid( rhv._wid ), _msl_set_id( rhv._msl_set_id ), _fe( motor::move( rhv._fe ) ),
      _cam( motor::move( rhv._cam ) )
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
motor::scene::result render_visitor::post_visit(
    motor::scene::group_ptr_t nptr, motor::scene::result const ) noexcept
{
    this_t::handle_post_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
void_t render_visitor::handle_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    // configurations
    {
        auto * comp = nptr->borrow_component< motor::scene::config_graphics_component_t >();
        if( comp != nullptr )
        {
            if( comp->init_and_cleanup( this_t::wid(), _fe ) )
            {
            }
        }
    }

    // render settings
    {
        auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
        if( comp != nullptr )
        {
            // only called if render state for id exists.            
            comp->borrow_state( 0, [ & ]( motor::graphics::command_status_mtr_t status,
                                           motor::graphics::state_object_mtr_t state ) //
            {
                motor::graphics::command_status::status const s = _fe->decode( *status );
                if( s != motor::graphics::command_status::status::configured )
                {
                    _fe->configure< motor::graphics::state_object_t >( state, status );
                }
                else if( s == motor::graphics::command_status::status::configured )
                {
                    _fe->push( state );
                }
            } );
        }
    }

    {
        auto * set_comp = nptr->borrow_component< motor::scene::msl_set_component_t >();
        if( set_comp != nullptr && set_comp->init_msl( this_t::wid(), this_t::msl_set_id(), _fe ) )
        {
            motor::scene::msl_component_mtr_t comp;
            if( set_comp->borrow_msl_component( this_t::msl_set_id(), comp ) )
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
void_t render_visitor::handle_post_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
    if( comp == nullptr ) return;

    auto const res = comp->borrow_state( 0, [ & ]( motor::graphics::command_status_mtr_t status,
                                                motor::graphics::state_object_mtr_t state ) //
    {
        motor::graphics::command_status::status s;
        if( _fe->decode( *status, s ) && s == motor::graphics::command_status::status::configured )
        {
            _fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
        }
    } );
}

//*****************************************************************************************
void_t render_visitor::on_start( void_t ) noexcept {}

//*****************************************************************************************
void_t render_visitor::on_finish( void_t ) noexcept {}