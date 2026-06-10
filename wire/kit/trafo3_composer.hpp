#pragma once

#include "../node/node.h"
#include <motor/math/utility/3d/transformation.hpp>
#include <motor/math/quaternion/quaternion4.hpp>

namespace motor
{
namespace wire
{
// this adapter node allows to compose a trafo from position, scale and rotation.
// output_slot : transformatoin > trafo3
// input_slot0 : position > vec3
// input_slot1 : scale > vec3
// input_slot2 : rotation > quaternion maybe?
template < typename T >
class trafo3_composer : public motor::wire::funk_node< motor::wire::unnamed_slot_sheet_policy >
{
    using base_t = motor::wire::funk_node< motor::wire::unnamed_slot_sheet_policy >;
    motor_this_typedefs( trafo3_composer );

    motor_typedefs( motor::math::m3d::transformation< T >, trafo );
    motor_typedefs( motor::math::vector3< T >, vec3 );
    motor_typedefs( motor::math::quaternion4< T >, quat4 );

    motor_typedefs( motor::wire::output_slot< this_t::trafo_t >, os );
    motor_typedefs( motor::wire::input_slot< this_t::vec3_t >, is_vec3 );
    motor_typedefs( motor::wire::input_slot< this_t::quat4_t >, is_quat4 );

  private:

    os_mtr_t _os = nullptr;

    is_vec3_mtr_t _is_pos = nullptr;
    is_vec3_mtr_t _is_scl = nullptr;
    is_quat4_mtr_t _is_rot = nullptr;

  public:

    trafo3_composer( void_t ) noexcept : base_t( this_t::make_funk() ) {}
    trafo3_composer( this_cref_t ) = delete;
    trafo3_composer( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _os( motor::move( rhv._os ) ),
          _is_pos( motor::move( rhv._is_pos ) ), _is_scl( motor::move( rhv._is_scl ) ),
          _is_rot( motor::move( rhv._is_rot ) )
    {
        this_t::make_and_set_composer_funk();
    }

    virtual ~trafo3_composer( void_t ) noexcept
    {
        motor::release( motor::move( _os ) );
        motor::release( motor::move( _is_pos ) );
        motor::release( motor::move( _is_scl ) );
        motor::release( motor::move( _is_rot ) );
    }

  public: // position

    is_vec3_ptr_t ensure_and_borrow_position_is( void_t ) noexcept
    {
        if( _is_pos == nullptr ) _is_pos = motor::shared( is_vec3_t() );
        if( _os == nullptr ) _os = motor::shared( os_t() );

        return _is_pos;
    }

    is_vec3_mtr_safe_t ensure_and_get_position_is( void_t ) noexcept
    {
        if( _is_pos == nullptr ) _is_pos = motor::shared( is_vec3_t() );
        if( _os == nullptr ) _os = motor::shared( os_t() );

        return motor::share( _is_pos );
    }

  public: // scaling

    is_vec3_ptr_t ensure_and_borrow_scaling_is( void_t ) noexcept
    {
        if( _is_scl == nullptr ) _is_scl = motor::shared( is_vec3_t() );
        if( _os == nullptr ) _os = motor::shared( os_t() );

        return _is_scl;
    }

    is_vec3_mtr_safe_t ensure_and_get_scaling_is( void_t ) noexcept
    {
        if( _is_scl == nullptr ) _is_scl = motor::shared( is_vec3_t() );
        if( _os == nullptr ) _os = motor::shared( os_t() );

        return motor::share( _is_scl );
    }

  public: // rotation

    is_quat4_mtr_t ensure_and_borrow_rotation_is( void_t ) noexcept
    {
        if( _is_rot == nullptr ) _is_rot = motor::shared( is_quat4_t() );
        if( _os == nullptr ) _os = motor::shared( os_t() );

        return _is_rot;
    }

    is_quat4_mtr_safe_t ensure_and_get_rotation_is( void_t ) noexcept
    {
        if( _is_rot == nullptr ) _is_rot = motor::shared( is_quat4_t() );
        if( _os == nullptr ) _os = motor::shared( os_t() );

        return motor::share( _is_rot );
    }

  public: // os

    this_t::os_mtr_safe_t ensure_and_get_os( void_t ) noexcept
    {
        if( _os == nullptr ) _os = motor::shared( os_t() );
        return motor::share( _os );
    }

  private:

    base_t::funk_t make_funk( void_t ) noexcept
    {
        return [ & ]( base_t * )
        {
            if( _os == nullptr ) return;

            this_t::trafo_t t;

            if( _is_scl != nullptr )
            {
                t.set_scale( _is_scl->get_value() );
            }

            if( _is_rot != nullptr )
            {
                t.rotate_by_quaternion_fr( _is_rot->get_value() );
            }

            if( _is_pos != nullptr )
            {
                t.set_translation( _is_pos->get_value() );
            }            

            _os->set_and_exchange( t );
        };
    }

    void_t make_and_set_composer_funk( void_t ) noexcept
    {
        this_t::set_funk( this_t::make_funk() );
    }
};
motor_typedefs( trafo3_composer< float_t >, trafo3f_composer );
} // namespace wire
} // namespace motor