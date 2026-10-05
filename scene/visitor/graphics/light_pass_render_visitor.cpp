

#include "light_pass_render_visitor.h"

#include "../../component/graphics/msl_component.h"
#include "../../component/graphics/msl_set_component.hpp"
#include "../../component/graphics/render_settings_component.hpp"
#include "../../component/graphics/config_graphics_component.h"

using namespace motor::scene;

//*****************************************************************************************
light_pass_render_visitor::light_pass_render_visitor(
    motor::scene::msl_set_component_t::id_t const id, size_t const render_id,
    motor::graphics::gen4::frontend_ptr_t fe, motor::gfx::generic_camera_ptr_t cam,
    motor::gfx::light_mtr_t light, motor::gfx::ishadow_data_mtr_t sd ) noexcept
    : _msl_set_id( id ), _render_data_id( render_id ), _fe( fe ), _cam( cam ), _light( light )

{
    this_t::determine_and_set_shadow_data_cast_funk( sd );
}

//*****************************************************************************************
light_pass_render_visitor::light_pass_render_visitor( this_rref_t rhv ) noexcept
    : _msl_set_id( rhv._msl_set_id ), _render_data_id( rhv._render_data_id ),
      _fe( motor::move( rhv._fe ) ), _cam( motor::move( rhv._cam ) ),
      _light( motor::move( rhv._light ) ), _cast_shadow_data( std::move( _cast_shadow_data ) )
{
    
}

//*****************************************************************************************
light_pass_render_visitor::~light_pass_render_visitor( void_t ) noexcept {}

//*****************************************************************************************
motor::scene::result light_pass_render_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    this_t::handle_post_visit( nptr );

    return motor::scene::result::ok;
}

//*****************************************************************************************
motor::scene::result light_pass_render_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    this_t::handle_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
motor::scene::result light_pass_render_visitor::post_visit(
    motor::scene::group_ptr_t nptr, motor::scene::result const ) noexcept
{
    this_t::handle_post_visit( nptr );
    return motor::scene::result::ok;
}

//*****************************************************************************************
void_t light_pass_render_visitor::handle_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    // configurations
    {
        auto * comp = nptr->borrow_component< motor::scene::config_graphics_component_t >();
        if( comp != nullptr )
        {
            if( comp->init_and_cleanup( _fe ) )
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
            comp->borrow_state( 0, [ & ]( motor::graphics::state_object_mtr_t state ) //
            {
                auto res = _fe->decode( state );
                if( res.first == motor::graphics::object_state::ready )
                {
                    _fe->push( state );
                }
                else if( res.first == motor::graphics::object_state::raw ||
                         res.first == motor::graphics::object_state::invalid )
                {
                    _fe->configure< motor::graphics::state_object_t >( state );
                }
            } );
        }
    }

    // msl stuff
    {
        auto * set_comp = nptr->borrow_component< motor::scene::msl_set_component_t >();
        if( set_comp != nullptr && set_comp->init_msl( this_t::msl_set_id(), _fe ) )
        {
            motor::scene::msl_component_mtr_t comp;
            if( set_comp->borrow_msl_component( this_t::msl_set_id(), comp ) )
            {
                size_t const used_vs_idx = comp->render_update( _render_data_id, _cam );

                // @todo do the same callback as done for shadow data.
                if( _light != nullptr &&
                    _light->get_light_type() == motor::gfx::light_type::directional )
                {
                    auto * ll = dynamic_cast< motor::gfx::directional_light_ptr_t >( _light );
                    comp->set_light_direction( _render_data_id, ll->get_direction() );
                }

                // copy shadow data into the variable sets.
                _cast_shadow_data( _render_data_id, comp ) ;

                auto msl = comp->borrow_msl();

                {
                    motor::graphics::gen4::backend_t::render_detail_t detail;
                    detail.start = 0;
                    // detail.num_elems = 3 ;
                    detail.geo = comp->get_geo_idx() == size_t( -1 ) ? 0 : comp->get_geo_idx();
                    detail.varset = used_vs_idx;
                    _fe->render( msl, detail );
                }
            }
        }
    }
}

//*****************************************************************************************
void_t light_pass_render_visitor::handle_post_visit( motor::scene::node_ptr_t nptr ) noexcept
{
    auto * comp = nptr->borrow_component< motor::scene::render_settings_component_t >();
    if( comp == nullptr ) return;

    auto const res = comp->borrow_state( 0, [ & ]( motor::graphics::state_object_mtr_t state ) //
    {
        auto res = _fe->decode( state );
        if( res.first == motor::graphics::object_state::ready )
        {
            _fe->pop( motor::graphics::gen4::backend::pop_type::render_state );
        }
    } );
}

//*****************************************************************************************
void_t light_pass_render_visitor::on_start( void_t ) noexcept {}

//*****************************************************************************************
void_t light_pass_render_visitor::on_finish( void_t ) noexcept {}

//*****************************************************************************************
void_t light_pass_render_visitor::determine_and_set_shadow_data_cast_funk(
    motor::gfx::ishadow_data_mtr_t sd ) noexcept
{
    if( auto * ptr = dynamic_cast< motor::gfx::single_shadow_data_ptr_t >( sd ); ptr != nullptr )
    {
        _shadow_data_ptr = sd;
        _cast_shadow_data = [ = ]( size_t const rd_id, motor::scene::msl_component_mtr_t comp )
        {
            comp->set_light_projection( rd_id, ptr->proj );
            comp->set_light_view( rd_id, ptr->view );
            comp->set_light_shadow_map( rd_id, ptr->shadow_map );
        };

        return;
    }

    _shadow_data_ptr = nullptr;
    _cast_shadow_data = []( size_t const, motor::scene::msl_component_mtr_t ) {
    };
}