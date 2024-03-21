
#include "xinput_module.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/log/global.h>

#include <windows.h>
#include <Xinput.h>

#include <array>

using namespace motor::controls ;
using namespace motor::platform::win32 ;

// 
// For class definition see far below
//


//
// 1. Xinput Helper
//
class xinput_module::xinput_device
{
    motor_this_typedefs( xinput_device ) ;

private:

    /// the xinput device id.
    DWORD _id = DWORD( -1 ) ;

    /// the current state
    XINPUT_STATE _state ;

    /// the current vibration state
    XINPUT_VIBRATION _vib ;

public:

    xinput_device( void_t ) noexcept
    {
        std::memset( &_state, 0, sizeof( XINPUT_STATE ) ) ;
        std::memset( &_vib, 0, sizeof( XINPUT_VIBRATION ) ) ;
    }

    xinput_device( this_cref_t ) = delete ;
    xinput_device( this_rref_t rhv ) noexcept
    {
        _id = rhv._id ;
        rhv._id = DWORD( -1 ) ;

        std::memcpy( &_state, &rhv._state, sizeof( XINPUT_STATE ) ) ;
        std::memcpy( &_vib, &rhv._state, sizeof( XINPUT_VIBRATION ) ) ;
    }
    xinput_device( DWORD id ) noexcept
    {
        _id = id ;
        std::memset( &_state, 0, sizeof( XINPUT_STATE ) ) ;
        std::memset( &_vib, 0, sizeof( XINPUT_VIBRATION ) ) ;
    }

public:

    DWORD get_id( void_t ) noexcept
    {
        return _id ;
    }

    /// check if a pressed event occurred.
    bool_t is_pressed( XINPUT_STATE const& new_state, DWORD button ) const noexcept
    {
        bool_t const old_button = bool_t( ( _state.Gamepad.wButtons & button ) != 0 ) ;
        bool_t const new_button = bool_t( ( new_state.Gamepad.wButtons & button ) != 0 ) ;

        return !old_button && new_button ;
    }

    /// check if a pressing event occurred.
    bool_t is_pressing( XINPUT_STATE const& new_state, DWORD button ) const noexcept
    {
        bool_t const old_button = bool_t( ( _state.Gamepad.wButtons & button ) != 0 ) ;
        bool_t const new_button = bool_t( ( new_state.Gamepad.wButtons & button ) != 0 ) ;

        return old_button && new_button ;
    }

    /// check if a released event occurred.
    bool_t is_released( XINPUT_STATE const& new_state, DWORD button ) const noexcept
    {
        bool_t const old_button = bool_t( ( _state.Gamepad.wButtons & button ) != 0 ) ;
        bool_t const new_button = bool_t( ( new_state.Gamepad.wButtons & button ) != 0 ) ;

        return old_button && !new_button ;
    }

    /// check the state of the left trigger
    motor::controls::components::button_state check_left_trigger(
        XINPUT_STATE const& new_state, uint16_t& intensity_out ) const noexcept
    {
        bool_t const old_press = bool_t( _state.Gamepad.bLeftTrigger != 0 ) ;
        bool_t const new_press = bool_t( new_state.Gamepad.bLeftTrigger != 0 ) ;

        intensity_out = uint16_t( new_state.Gamepad.bLeftTrigger ) ;

        if( new_press && !old_press ) return motor::controls::components::button_state::pressed ;
        if( new_press && old_press ) return motor::controls::components::button_state::pressing ;
        if( old_press && !new_press ) return motor::controls::components::button_state::released ;

        return motor::controls::components::button_state::none ;
    }

    /// check the state of the right trigger
    motor::controls::components::button_state check_right_trigger(
        XINPUT_STATE const& new_state, uint16_t& intensity_out ) const noexcept
    {
        bool_t const old_press = bool_t( _state.Gamepad.bRightTrigger != 0 ) ;
        bool_t const new_press = bool_t( new_state.Gamepad.bRightTrigger != 0 ) ;

        intensity_out = uint16_t( new_state.Gamepad.bRightTrigger ) ;

        if( new_press && !old_press ) return motor::controls::components::button_state::pressed ;
        if( new_press && old_press ) return motor::controls::components::button_state::pressing ;
        if( old_press && !new_press ) return motor::controls::components::button_state::released ;
        return motor::controls::components::button_state::none ;
    }


    motor::controls::components::stick_state check_left_stick(
        XINPUT_STATE const& new_state, motor::math::vec2f_t& nnc_out ) const noexcept
    {
        motor::math::vec2b_t const old_tilt = motor::math::vec2b_t(
            _state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || _state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
            _state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || _state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) ;
        motor::math::vec2b_t const new_tilt = motor::math::vec2b_t(
            new_state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || new_state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
            new_state.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || new_state.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) ;

        // according to ms doc, max range is in [0, 1<<16]
        // using range [0,1<<15] to get [-1,1] value instead of [-0.5,0.5] values.
        motor::math::vec2f_t const inv_width =
            motor::math::vec2f_t( 1.0f / float_t( ( 1 << 15 ) - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) ) ;

        motor::math::vector2< SHORT > const tmp = motor::math::vector2< SHORT >(
            new_state.Gamepad.sThumbLX, new_state.Gamepad.sThumbLY ).
            dead_zone( XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) ;

        SHORT X = tmp.x() ;
        SHORT Y = tmp.y() ;

        X -= X > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : 0 ;
        X += X < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : 0 ;

        Y -= Y > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : 0 ;
        Y += Y < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE : 0 ;

        nnc_out = motor::math::vec2f_t( float_t( X ), float_t( Y ) ) * inv_width ;

        if( !old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilted ;
        if( old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilting ;
        if( old_tilt.any() && !new_tilt.any() ) return motor::controls::components::stick_state::untilted ;

        return motor::controls::components::stick_state::none ;
    }

    motor::controls::components::stick_state check_left_stick(
        XINPUT_STATE const& new_state, motor::math::vector2<int16_t>& val_out ) const noexcept
    {
        motor::math::vector2<int16_t> old_xy(
            _state.Gamepad.sThumbLX, _state.Gamepad.sThumbLY ) ;

        motor::math::vector2<int16_t> new_xy(
            new_state.Gamepad.sThumbLX, new_state.Gamepad.sThumbLY ) ;

        motor::math::vector2<int16_t> const dead_zone_xy(
            XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ) ;

        motor::math::vector2<int16_t> const new_xy_dif = new_xy.clamped( dead_zone_xy.negated(), dead_zone_xy ) ;
        motor::math::vector2<int16_t> const old_xy_dif = old_xy.clamped( dead_zone_xy.negated(), dead_zone_xy ) ;

        old_xy = old_xy + old_xy_dif.negated() ;
        new_xy = new_xy + new_xy_dif.negated() ;

        val_out = new_xy ;

        motor::math::vec2b_t const old_tilt = old_xy.absed().greater_than( motor::math::vec2i16_t( 0 ) ) ;
        motor::math::vec2b_t const new_tilt = new_xy.absed().greater_than( motor::math::vec2i16_t( 0 ) ) ;

        if( !old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilted ;
        if( old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilting ;
        if( old_tilt.any() && !new_tilt.any() ) return motor::controls::components::stick_state::untilted ;

        return motor::controls::components::stick_state::none ;
    }

    motor::controls::components::stick_state check_right_stick(
        XINPUT_STATE const& new_state, motor::math::vec2f_t& nnc_out ) const noexcept
    {
        motor::math::vec2b_t const old_tilt = motor::math::vec2b_t(
            _state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || _state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
            _state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || _state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) ;
        motor::math::vec2b_t const new_tilt = motor::math::vec2b_t(
            new_state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || new_state.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
            new_state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || new_state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) ;

        // according to ms doc, max range is in [0, 1<<16]
        // using range [0,1<<15] to get [-1,1] value instead of [-0.5,0.5] values.
        motor::math::vec2f_t const inv_width =
            motor::math::vec2f_t( 1.0f / float_t( ( 1 << 15 ) - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) ) ;

        motor::math::vector2< SHORT > const tmp = motor::math::vector2< SHORT >(
            new_state.Gamepad.sThumbRX, new_state.Gamepad.sThumbRY ).dead_zone( XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) ;

        SHORT X = tmp.x() ;
        SHORT Y = tmp.y() ;

        X -= X > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : 0 ;
        X += X < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : 0 ;

        Y -= Y > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : 0 ;
        Y += Y < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE : 0 ;

        nnc_out = motor::math::vec2f_t( float_t( X ), float_t( Y ) ) * inv_width ;

        if( !old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilted ;
        if( old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilting ;
        if( old_tilt.any() && !new_tilt.any() ) return motor::controls::components::stick_state::untilted ;
        return motor::controls::components::stick_state::none ;
    }

    motor::controls::components::stick_state check_right_stick(
        XINPUT_STATE const& new_state, motor::math::vector2<int16_t>& val_out ) const noexcept
    {
        motor::math::vector2<int16_t> old_xy(
            _state.Gamepad.sThumbRX, _state.Gamepad.sThumbRY ) ;

        motor::math::vector2<int16_t> new_xy(
            new_state.Gamepad.sThumbRX, new_state.Gamepad.sThumbRY ) ;

        motor::math::vector2<int16_t> const dead_zone_xy(
            XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ) ;

        motor::math::vector2<int16_t> const new_xy_dif = new_xy.clamped( dead_zone_xy.negated(), dead_zone_xy ) ;
        motor::math::vector2<int16_t> const old_xy_dif = old_xy.clamped( dead_zone_xy.negated(), dead_zone_xy ) ;

        old_xy = old_xy + old_xy_dif.negated() ;
        new_xy = new_xy + new_xy_dif.negated() ;

        val_out = new_xy ;

        motor::math::vec2b_t const old_tilt = old_xy.absed().greater_than( motor::math::vec2i16_t( 0 ) ) ;
        motor::math::vec2b_t const new_tilt = new_xy.absed().greater_than( motor::math::vec2i16_t( 0 ) ) ;

        if( !old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilted ;
        if( old_tilt.any() && new_tilt.any() ) return motor::controls::components::stick_state::tilting ;
        if( old_tilt.any() && !new_tilt.any() ) return motor::controls::components::stick_state::untilted ;

        return motor::controls::components::stick_state::none ;
    }

    bool_t check_vibration( XINPUT_VIBRATION const & new_state ) const noexcept
    {
        return ( _vib.wLeftMotorSpeed != new_state.wLeftMotorSpeed ||
            _vib.wRightMotorSpeed != new_state.wRightMotorSpeed ) ;
    }

    /// exchange the old xinput state with the new one.
    void_t exchange_state( XINPUT_STATE const& new_state ) noexcept
    {
        _state = new_state ;
    }

    void_t exchange_state( XINPUT_VIBRATION const & new_state ) noexcept
    {
        _vib = new_state ;
    }
};

//
// 2. mappings
//

namespace this_file
{
    typedef motor::controls::types::xbox_controller_t::input_component ic_t ;
    static ic_t map_button_xinput_to_component( DWORD const b ) noexcept
    {
        ic_t ret = ic_t::num_components ;

        switch( b )
        {
        case XINPUT_GAMEPAD_BACK: ret = ic_t::button_back ;  break ;
        case XINPUT_GAMEPAD_START: ret = ic_t::button_start ;  break ;
        case XINPUT_GAMEPAD_A: ret = ic_t::button_a ;  break ;
        case XINPUT_GAMEPAD_B: ret = ic_t::button_b ;  break ;
        case XINPUT_GAMEPAD_X: ret = ic_t::button_x ;  break ;
        case XINPUT_GAMEPAD_Y: ret = ic_t::button_y ;  break ;
        case XINPUT_GAMEPAD_LEFT_THUMB: ret = ic_t::button_thumb_left ;  break ;
        case XINPUT_GAMEPAD_RIGHT_THUMB: ret = ic_t::button_thumb_right;  break ;
        case XINPUT_GAMEPAD_LEFT_SHOULDER: ret = ic_t::button_shoulder_left ;  break ;
        case XINPUT_GAMEPAD_RIGHT_SHOULDER: ret = ic_t::button_shoulder_right ;  break ;
        case XINPUT_GAMEPAD_DPAD_UP: ret = ic_t::button_dpad_up ;  break ;
        case XINPUT_GAMEPAD_DPAD_DOWN: ret = ic_t::button_dpad_down ;  break ;
        case XINPUT_GAMEPAD_DPAD_LEFT: ret = ic_t::button_dpad_left ;  break ;
        case XINPUT_GAMEPAD_DPAD_RIGHT: ret = ic_t::button_dpad_right ;  break ;
        default: break ;
        }

        return ret ;
    }

    typedef motor::controls::types::xbox_controller_t::button button_t ;

    static button_t map_button_xinput_to_layout( DWORD const b ) noexcept
    {
        button_t ret = button_t::none ;

        switch( b )
        {
        case XINPUT_GAMEPAD_BACK: ret = button_t::back ;  break ;
        case XINPUT_GAMEPAD_START: ret = button_t::start ;  break ;
        case XINPUT_GAMEPAD_A: ret = button_t::a ;  break ;
        case XINPUT_GAMEPAD_B: ret = button_t::b ;  break ;
        case XINPUT_GAMEPAD_X: ret = button_t::x ;  break ;
        case XINPUT_GAMEPAD_Y: ret = button_t::y ;  break ;
        default: break ;
        }

        return ret ;
    }

    typedef motor::controls::types::xbox_controller_t::thumb thumb_t ;

    static thumb_t map_thumb_xinput_to_layout( DWORD const b ) noexcept
    {
        thumb_t ret = thumb_t::none ;

        switch( b )
        {
        case XINPUT_GAMEPAD_LEFT_THUMB: ret = thumb_t::left ;  break ;
        case XINPUT_GAMEPAD_RIGHT_THUMB: ret = thumb_t::right ;  break ;
        default: break ;
        }

        return ret ;
    }

    typedef motor::controls::types::xbox_controller_t::shoulder shoulder_t ;

    static shoulder_t map_shoulder_xinput_to_layout( DWORD const b ) noexcept
    {
        shoulder_t ret = shoulder_t::none ;

        switch( b )
        {
        case XINPUT_GAMEPAD_LEFT_SHOULDER: ret = shoulder_t::left ;  break ;
        case XINPUT_GAMEPAD_RIGHT_SHOULDER: ret = shoulder_t::right ;  break ;
        default: break ;
        }

        return ret ;
    }

    typedef motor::controls::types::xbox_controller_t::dpad dpad_t ;

    static dpad_t map_dpad_xinput_to_layout( DWORD const b ) noexcept
    {
        dpad_t ret = dpad_t::none ;

        switch( b )
        {
        case XINPUT_GAMEPAD_DPAD_UP: ret = dpad_t::up ;  break ;
        case XINPUT_GAMEPAD_DPAD_DOWN: ret = dpad_t::down ;  break ;
        case XINPUT_GAMEPAD_DPAD_LEFT: ret = dpad_t::left ;  break ;
        case XINPUT_GAMEPAD_DPAD_RIGHT: ret = dpad_t::right ;  break ;
        default: break ;
        }

        return ret ;
    }
}

//***
xinput_module::xinput_module( void_t ) noexcept 
{
    #if defined( MOTOR_TARGET_OS_WIN10 ) || defined( MOTOR_TARGET_OS_WIN8 )
    //XInputEnable( true ) ;
    #endif

    // by XInput 1.4 spec. only 4 devices supported.
    for( DWORD i=0; i<4; ++i )
    {
        gamepad_data_t gd ;
        gd.xinput_ptr = motor::memory::global_t::alloc( xinput_device( i ),
            "[xinput_module] : xinput_device" ) ;

        XINPUT_STATE state ;
        std::memset( ( void_ptr_t ) &state, 0, sizeof(XINPUT_STATE) ) ;
        gd.xinput_ptr->exchange_state( state ) ;
        gd.dev = motor::memory::create_ptr( motor::controls::xbc_device_t("xbox controller #"+motor::to_string(i)),
            "[xinput_module] : xbc_device" ) ;

        _devices.push_back( gd ) ;
    }
}

//***
xinput_module::xinput_module( this_rref_t rhv ) noexcept 
{
    _devices = std::move( rhv._devices ) ;
}

//***
xinput_module::~xinput_module( void_t ) noexcept 
{
    #if defined( MOTOR_TARGET_OS_WIN10 ) || defined( MOTOR_TARGET_OS_WIN8 )
    //XInputEnable( false ) ;
    #endif

    this_t::release() ;
}

//***
xinput_module::this_ref_t xinput_module::operator = ( this_rref_t rhv ) noexcept 
{
    _devices = std::move( rhv._devices ) ;
    return *this ;
}

//***
void_t xinput_module::search( motor::controls::imodule::search_funk_t funk )  noexcept
{
    for( auto & d : _devices )
    {
        funk( d.dev ) ;
    }
}

//***
void_t xinput_module::update( void_t ) noexcept 
{
    this_t::check_gamepads() ;

    for( auto& item : _devices )
    {
        if( !item.connected ) continue ;

        item.dev->update_all() ;

        motor::controls::xbc_device_mtr_t dev = item.dev ;
        xinput_device & helper = *item.xinput_ptr ;

        XINPUT_STATE state ;
        DWORD const res = XInputGetState( item.xinput_ptr->get_id(), &state ) ;
        if( motor::log::global_t::warning( res != ERROR_SUCCESS,
            motor_log_fn("XInputGetState") ) ) continue ;

        motor::controls::types::xbox_controller_t ctrl( dev ) ;

        // buttons : Just the on/off buttons
        {
            constexpr const std::array<DWORD, 14> buttons__ (
                { 
                    XINPUT_GAMEPAD_BACK, XINPUT_GAMEPAD_START, XINPUT_GAMEPAD_A,
                    XINPUT_GAMEPAD_B, XINPUT_GAMEPAD_X, XINPUT_GAMEPAD_Y,
                    XINPUT_GAMEPAD_DPAD_UP, XINPUT_GAMEPAD_DPAD_DOWN,
                    XINPUT_GAMEPAD_DPAD_RIGHT, XINPUT_GAMEPAD_DPAD_LEFT,
                    XINPUT_GAMEPAD_LEFT_SHOULDER, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                    XINPUT_GAMEPAD_LEFT_THUMB, XINPUT_GAMEPAD_RIGHT_THUMB
                } ) ;

            for( auto const & i : buttons__ )
            {
                auto const b = this_file::map_button_xinput_to_component( i ) ;
                if( helper.is_pressed( state, i ) )
                {
                    *ctrl.comp_button( b ) = motor::controls::components::button_state::pressed ;
                    *ctrl.comp_button( b ) = 1.0f ;
                }
                else if( helper.is_pressing( state, i ) )
                {
                    *ctrl.comp_button( b ) = motor::controls::components::button_state::pressing ;
                    *ctrl.comp_button( b ) = 1.0f ;
                }
                else if( helper.is_released(state, i ) )
                {
                    *ctrl.comp_button( b ) = motor::controls::components::button_state::released ;
                    *ctrl.comp_button( b ) = 0.0f ;
                }
            }
        }

        // triggers
        {
            uint16_t intensity = 0 ;
            {
                auto const bs = helper.check_left_trigger( state, intensity ) ;
                if( bs != motor::controls::components::button_state::none )
                {
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::trigger::left ) = bs ;
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::trigger::left ) =
                        float_t( intensity ) / 255.0f ; ;
                }
            }
            {
                auto const bs = helper.check_right_trigger( state, intensity ) ;
                if( bs != motor::controls::components::button_state::none )
                {
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::trigger::right ) = bs ;
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::trigger::right ) =
                        float_t( intensity ) / 255.0f ; ;
                }
            }
        }

        // sticks
        {
            motor::math::vec2f_t change ;
            {
                auto const ss = helper.check_left_stick( state, change ) ;
                if( ss != motor::controls::components::stick_state::none )
                {
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::stick::left ) = ss ;
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::stick::left ) = change ;
                }
            }

            {
                auto const ss = helper.check_right_stick( state, change ) ;
                if( ss != motor::controls::components::stick_state::none )
                {
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::stick::right ) = ss ;
                    *ctrl.get_component( motor::controls::types::xbox_controller_t::stick::right ) = change ;
                }
            }
        }

        // motors - this is output to the controller
        {
            XINPUT_VIBRATION vib ;
            std::memset( &vib, 0, sizeof( XINPUT_VIBRATION ) ) ;
            
            {
                auto * motor = ctrl.get_component( motor::controls::types::xbox_controller_t::vibrator::left ) ;
                vib.wLeftMotorSpeed = WORD( 65535.0f * motor->value() ) ;
            }

            {
                auto* motor = ctrl.get_component( motor::controls::types::xbox_controller_t::vibrator::right ) ;
                vib.wRightMotorSpeed = WORD( 65535.0f * motor->value() ) ;
            }


            if( helper.check_vibration( vib ) ) 
            {
                XInputSetState( item.xinput_ptr->get_id(), &vib ) ;
                helper.exchange_state( vib ) ;
            }
        }

        //
        // must be done after the new state is consumed.
        // i.e. the gamepad received all new states
        //
        {
            helper.exchange_state( state ) ;
        }
    }
}

//********************************************************************************
void_t xinput_module::release( void_t ) noexcept 
{
    for( auto & item : _devices )
    {
        motor::memory::global_t::dealloc( item.xinput_ptr ) ;
        motor::memory::release_ptr( motor::move( item.dev ) ) ;
        item.xinput_ptr = nullptr ;
    }
    _devices.clear() ;
}

//********************************************************************************
void_t xinput_module::check_gamepads( void_t ) noexcept 
{    
    DWORD i = 0 ; 
    for( auto & gd : _devices )
    {
        XINPUT_CAPABILITIES caps ;
        DWORD res = XInputGetCapabilities( i, XINPUT_FLAG_GAMEPAD, &caps ) ;

        if( res == ERROR_DEVICE_NOT_CONNECTED ) 
        {
            if( gd.connected )
            {
                motor::log::global_t::status("[XInput] : device disconnected (" + motor::from_std( std::to_string(i) ) + ")" ) ;
                XINPUT_STATE state ;
                std::memset( ( void_ptr_t ) &state, 0, sizeof( XINPUT_STATE ) ) ;
                gd.xinput_ptr->exchange_state( state ) ;
            }
            gd.connected = false ;
            gd.dev->set_plugged( false ) ;
            continue ;
        }

        if( !gd.connected )
        {
            motor::log::global_t::status("[XInput] : device connected (" + motor::from_std( std::to_string(i) ) + ")" ) ;
            gd.connected = true ;
            gd.dev->set_plugged( true ) ;
        }

        ++i ;
    }
}

