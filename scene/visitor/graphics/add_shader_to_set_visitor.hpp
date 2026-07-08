#pragma once

#include "../default_visitor.h"

#include "../../node/group.h"
#include "../../node/leaf.h"

#include "../../component/graphics/msl_set_component.hpp"

#include <motor/application/typedefs.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>
#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
// search a node by a name component. It will break the traversal
// if the first occurance is found.
class add_shader_to_set_visitor : public default_visitor
{
    motor_this_typedefs( add_shader_to_set_visitor );

    motor::scene::msl_set_component_t::id_t _id = motor::scene::msl_set_component_t::invalid_id();

    motor::string_t _name;
    motor::string_t _shader;

    using map_t = motor::hash_map< motor::string_t, motor::graphics::msl_object_mtr_t >;
    map_t _name_to_msl;

  public:

    add_shader_to_set_visitor( motor::scene::msl_set_component_t::id_t const id,
        motor::string_cref_t name, motor::string_cref_t shader ) noexcept
        : _id( id ), _name( name ), _shader( shader )
    {
    }

    add_shader_to_set_visitor( this_rref_t rhv ) noexcept
        : _id( rhv._id ), _name( std::move( rhv._name ) ), _shader( std::move( rhv._shader ) )
    {
    }
    add_shader_to_set_visitor( this_cref_t ) = delete;
    virtual ~add_shader_to_set_visitor( void_t ) noexcept
    {
        for( auto i : _name_to_msl )
        {
            motor::release( motor::move( i.second ) );
        }
        _name_to_msl.clear();
    }

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        motor::scene::msl_set_component_mtr_t comp;
        if( nptr->has_component_and_borrow< motor::scene::msl_set_component_t >( comp ) )
        {
            // clone and set new shader
            motor::scene::msl_component_mtr_t msl_comp;
            if( comp->borrow_msl_component( 0, msl_comp ) )
            {
                auto const name = msl_comp->borrow_msl()->name() + "." + _name;
                auto iter = _name_to_msl.find( name );
                if( iter == _name_to_msl.end() )
                {
                    auto clone = msl_comp->light_clone( name );
                    comp->add_component( _id, motor::shared( std::move( clone ) ) );

                    if( comp->borrow_msl_component( _id, msl_comp ) )
                    {
                        msl_comp->borrow_msl()->clear_shaders().add(
                            motor::graphics::msl_api_type::msl_4_0, _shader );

                        // because msls can be used multiple times,
                        // we have to remember and only clone once.
                        _name_to_msl[ name ] = msl_comp->get_msl();
                    }
                }
                else
                {
                    auto clone = msl_comp->light_clone( name );

                    // if already clone once before, just overwrite the
                    // cloned msl object with the stored one.
                    clone.set_msl( motor::share( iter->second ) );
                    comp->add_component( _id, motor::shared( std::move( clone ) ) );
                }
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

    map_t move_msl_map( void_t ) noexcept
    {
        return std::move( _name_to_msl );
    }
};
motor_typedef( add_shader_to_set_visitor );
} // namespace scene
} // namespace motor