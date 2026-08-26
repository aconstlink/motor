#pragma once

#include "../default_visitor.h"

#include "../../node/group.h"
#include "../../node/leaf.h"

#include "../../component/graphics/msl_set_component.hpp"
#include "../../component/graphics/geometry_name_component.hpp"

#include <motor/application/typedefs.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>
#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
// search a node by a name component. It will break the traversal
// if the first occurance is found.
class add_msl_to_set_visitor : public default_visitor
{
    motor_this_typedefs( add_msl_to_set_visitor );

    motor::scene::msl_set_component_t::id_t _id = motor::scene::msl_set_component_t::invalid_id();

    motor::graphics::msl_object_mtr_t _msl;

    using variable_set_init_funk_t =
        std::function< void_t( motor::string_in_t, motor::graphics::variable_set_mtr_t ) >;

    // called when msl is added and new variable
    // set is created for poluting the set with variables.
    variable_set_init_funk_t _vsf = []( motor::string_in_t name,
                                        motor::graphics::variable_set_mtr_t ) {
    };

  public:

    add_msl_to_set_visitor( motor::scene::msl_set_component_t::id_t const id,
        motor::graphics::msl_object_mtr_safe_t msl ) noexcept
        : _id( id ), _msl( motor::move( msl ) )
    {
    }

    add_msl_to_set_visitor( motor::scene::msl_set_component_t::id_t const id,
        motor::graphics::msl_object_mtr_safe_t msl, variable_set_init_funk_t vsf ) noexcept
        : _id( id ), _msl( motor::move( msl ) ), _vsf( std::move( vsf ) )
    {
    }

    add_msl_to_set_visitor( this_rref_t rhv ) noexcept
        : _id( rhv._id ), _msl( motor::move( rhv._msl ) ), _vsf( std::move( rhv._vsf ) )
    {
    }

    add_msl_to_set_visitor( this_cref_t ) = delete;

    virtual ~add_msl_to_set_visitor( void_t ) noexcept
    {
        motor::release( motor::move( _msl ) );
    }

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        motor::scene::msl_set_component_mtr_t comp;
        if( nptr->has_component_and_borrow< motor::scene::msl_set_component_t >( comp ) )
        {
            motor::string_t name;
            {
                motor::scene::name_component_mtr_t nc;
                if( nptr->has_component_and_borrow< motor::scene::name_component_t >( nc ) )
                {
                    name = nc->get_name();
                }
            }

            motor::scene::msl_component_mtr_t msl_comp;
            if( comp->borrow_msl_component( _id, msl_comp ) )
            {
                // there is already a msl component.
                size_t const geo_idx = msl_comp->get_geo_idx();
                size_t const vs_idx = msl_comp->get_variable_set_idx();

                auto * msl = msl_comp->borrow_msl();

                motor::string_t geo_name = msl->get_geo_link( geo_idx ).name;
                auto vs = msl->get_varibale_set( vs_idx );

                msl->unlink_geometry( geo_idx );
                msl->drop_variable_set( vs_idx );

                _msl->link_geometry( geo_name );
                _msl->add_variable_set( motor::move( vs ) );

                _vsf( name, vs );

                msl_comp->set_msl( motor::share( _msl ) );
            }
            else
            {
                size_t geo_idx = size_t( -1 );

                motor::scene::geometry_name_component_mtr_t geo_comp;
                if( nptr->has_component_and_borrow( geo_comp ) )
                {
                    geo_idx = _msl->link_geometry( geo_comp->get_name() );
                }
                else
                {
                    // do not have any geometry, so we can not
                    // link the shader to any geoemtry.
                    motor::log::global_t::warning(
                        "[add_msl_to_set_visitor] : need geometry_name_component in order to link "
                        "any geometry and use it in the msl_component." );
                }

                auto vs = motor::shared( motor::graphics::variable_set_t() );
                _vsf( name, vs );

                size_t const vs_idx = _msl->add_variable_set( motor::move( vs ) );

                motor::scene::msl_component_t new_comp( motor::share( _msl ), vs_idx, geo_idx );

                comp->add_component( _id, motor::shared( std::move( new_comp ) ) );
            }
        }

        return motor::scene::result::ok;
    }

    virtual motor::scene::result post_visit(
        motor::scene::node_ptr_t, motor::scene::result const res ) noexcept
    {
        return res;
    }

    virtual void_t on_start( void_t ) noexcept {}
    virtual void_t on_finish( void_t ) noexcept {}
};
motor_typedef( add_msl_to_set_visitor );
} // namespace scene
} // namespace motor