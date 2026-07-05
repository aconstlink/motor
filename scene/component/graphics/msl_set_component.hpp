#pragma once

#include "../icomponent.h"
#include "msl_component.h"

#include <motor/application/typedefs.h>

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
class msl_set_component : public motor::scene::icomponent
{
    motor_this_typedefs( msl_set_component );

  public:

    using id_t = size_t;

    static id_t invalid_id( void_t ) noexcept
    {
        return id_t( -1 );
    }

  private:

    using init_map_t = motor::hash_map< motor::application::window_id_t, bool_t >;

    struct entry_data
    {
        motor::scene::msl_component_mtr_t msl;
    };
    motor::hash_map< size_t, entry_data > _components;

  public:

    msl_set_component( void_t ) noexcept {}
    msl_set_component( id_t const id, motor::scene::msl_component_mtr_safe_t comp ) noexcept
    {
        this_t::add_component( id, motor::move( comp ) );
    }

    msl_set_component( this_cref_t ) = delete;
    msl_set_component( this_rref_t rhv ) noexcept
        : icomponent( std::move( rhv ) ), _components( std::move( rhv._components ) )
    {
    }

    virtual ~msl_set_component( void_t ) noexcept
    {
        for( auto & item : _components )
        {
            motor::release( motor::move( item.second.msl ) );
        }
        _components.clear();
    }

  public:

    void_t add_component( id_t const id, motor::scene::msl_component_mtr_safe_t msl ) noexcept
    {
        auto iter = _components.find( id );
        if( iter != _components.end() )
        {
            motor::release( motor::move( iter->second.msl ) );
            _components.erase( iter );
        }

        _components[ id ] = { motor::move( msl ) };
    }

    bool_t init_msl( motor::application::window_id_t const wid, id_t const id,
        motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        auto iter = _components.find( id );
        if( iter == _components.end() )
        {
            // component does not exist for id.
            // must be added first
            return false;
        }

        return iter->second.msl->render_init( wid, fe );
    }

    bool_t release_msl( motor::application::window_id_t const wid, id_t const id,
        motor::graphics::gen4::frontend_ptr_t fe ) noexcept
    {
        auto iter = _components.find( id );
        if( iter == _components.end() )
        {
            // component does not exist for id.
            // must be added first
            return false;
        }

        return iter->second.msl->render_init( wid, fe );
    }

    bool_t borrow_msl_component( id_t const id, motor::scene::msl_component_mtr_t & ptr ) noexcept
    {
        auto iter = _components.find( id );
        if( iter == _components.end() ) return false;

        ptr = iter->second.msl;

        return true;
    }

    using for_each_msl_funk_t = std::function< void_t( motor::scene::msl_component_mtr_t ) >;
    void_t for_each_msl( for_each_msl_funk_t f ) noexcept
    {
        for( auto & d : _components )
        {
            f( d.second.msl );
        }
    }
};
motor_typedef( msl_set_component );
} // namespace scene
} // namespace motor