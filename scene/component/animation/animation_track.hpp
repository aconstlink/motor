

#pragma once

#include "animation_controller.h"

#include <motor/wire/slot/input_slot.h>
#include <motor/wire/slot/output_slot.h>
#include <motor/math/animation/keyframe_sequence.hpp>

namespace motor
{
namespace scene
{

// T : must be a spline type
// in slot  : time_ms_t
// out slot : what T is animating
template < typename T >
class animation_track : public animation_controller
{
    using base_t = animation_controller;
    motor_this_typedefs( animation_track< T > );
    motor_typedefs( motor::math::keyframe_sequence< T >, kfs );

    motor_typedefs( motor::wire::input_slot< typename kfs_t::time_stamp_t >, is );
    motor_typedefs( motor::wire::output_slot< typename kfs_t::value_t >, os );

  private:

    kfs_t _kfs;
    is_t * _is = nullptr;
    os_t * _os = nullptr;

  public:

    animation_track( kfs_cref_t kfs ) noexcept : _kfs( kfs )
    {
        this_t::init_wire_components();
    }

    animation_track( kfs_rref_t kfs ) noexcept : _kfs( std::move( kfs ) )
    {
        this_t::init_wire_components();
    }

    animation_track( this_cref_t ) = delete;
    animation_track( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _kfs( std::move( rhv._kfs ) ), _is( motor::move( rhv._is ) ),
          _os( motor::move( rhv._os ) )
    {
        auto * node = static_cast< motor::wire::funk_node_ptr_t >( base_t::borrow_node() );
        node->set_funk( [ & ]( motor::wire::funk_node_ptr_t /*self*/ ) { this->evaluate(); } );
    }

    virtual ~animation_track( void_t ) noexcept
    {
        motor::release( motor::move( _is ) );
        motor::release( motor::move( _os ) );
    }

  public:

    is_mtr_safe_t get_is( void_t ) noexcept
    {
        return motor::share( _is ) ; 
    }
    
    is_mtr_t borrow_is( void_t ) noexcept
    {
        return _is;
    }

    os_mtr_safe_t get_os( void_t ) noexcept
    {
        return motor::share( _os ) ;
    }

    os_mtr_t borrow_os( void_t ) noexcept
    {
        return _os;
    }

  private:

    void_t init_wire_components( void_t ) noexcept
    {
        _is = motor::shared( is_t( 0 ) );
        _os = motor::shared( os_t() );

        auto node = motor::wire::funk_node_t( [ & ]( motor::wire::funk_node_ptr_t /*self*/ )
        { this->evaluate(); } );

        node.inputs().add( motor::share( _is ) );
        node.outputs().add( motor::share( _os ) );
        this_t::set_node( motor::shared( std::move( node ) ) );
    }

    void_t evaluate( void_t ) noexcept
    {
        auto const ts = _is->pull_data();
        _os->set_and_exchange( _kfs( ts ) );
    }
};

} // namespace scene
} // namespace motor