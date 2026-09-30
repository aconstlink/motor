#pragma once

#include "default_visitor.h"

#include "../node/group.h"
#include "../node/leaf.h"

#include "../component/light_component.hpp"

namespace motor
{
namespace scene
{
// collect all lights this visitor traverses.
// could collect all lights based on the camera.
class light_collector_visitor : public default_visitor
{
    motor_this_typedefs( light_collector_visitor );

    using lights_t = motor::vector< motor::scene::light_component_mtr_t >;

    // borrowd lights
    lights_t _lights;

  public:

    light_collector_visitor( void_t ) noexcept {}

    light_collector_visitor( this_rref_t rhv ) noexcept : _lights( std::move( rhv._lights ) ) {}
    light_collector_visitor( this_cref_t ) = delete;
    virtual ~light_collector_visitor( void_t ) noexcept {}

  public:

    virtual motor::scene::result visit( motor::scene::node_ptr_t nptr ) noexcept
    {
        {
            motor::scene::light_component_mtr_t cptr;
            if( nptr->has_component_and_borrow< motor::scene::light_component_t >( cptr ) )
            {
                // borrow light component
                _lights.emplace_back( cptr );
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

  public:

    this_t::lights_t move_lights_out( void_t ) noexcept
    {
        return std::move( _lights );
    }

    using for_each_funk_t =
        std::function< void_t( size_t const, motor::scene::light_component_mtr_t ) >;
    void_t for_each_light( this_t::for_each_funk_t f ) noexcept
    {
        size_t i = size_t( -1 );
        for( auto * ptr : _lights )
        {
            f( ++i, ptr );
        }
    }

    void_t clear_lights( void_t ) noexcept
    {
        _lights.clear() ;
    }
};
motor_typedef( light_collector_visitor );
} // namespace scene
} // namespace motor