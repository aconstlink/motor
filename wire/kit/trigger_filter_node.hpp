
#pragma once

#include "../node/node.h"
#include <motor/math/typedefs.h>

namespace motor::wire
{

class trigger_filter_node : public motor::wire::funk_node< motor::wire::unnamed_slot_sheet_policy >
{
    using base_t = motor::wire::funk_node< motor::wire::unnamed_slot_sheet_policy >;
    motor_this_typedefs( trigger_filter_node );

  public:

    enum class trigger_behavoir
    {
        reset_on_trigger,
        ignore_while_active,
        num_behavoirs
    };

    using trigger_is_t = motor::wire::input_slot< uint_t >;
    using trigger_os_t = motor::wire::output_slot< uint_t >;

    using active_is_t = motor::wire::input_slot< float_t >;
    using active_os_t = motor::wire::output_slot< float_t >;

    using time_is_t = motor::wire::input_slot< float_t >;
    using time_os_t = motor::wire::output_slot< float_t >;

    using signal_is_t = motor::wire::input_slot< float_t >;
    using signal_os_t = motor::wire::output_slot< float_t >;

    struct filter_data
    {
        float_t const sig_abs;   // absolute signal
        float_t const sig_start; // signal at trigger
        float_t const time;      // absolute time
        float_t const time_rel;  // relative time since trigger
        float_t const duration;  // duration of the filter
        float_t const dt;        // delta to the last filter call
    };
    using filter_kernel_t = std::function< float_t( this_t::filter_data const & d ) >;

  private:

    // input time
    time_is_t * _time_is = motor::shared( time_is_t( 0.0f ) );

    // counts the triggers
    // if triggered, the nodes plays
    trigger_is_t * _trigger_is = motor::shared( trigger_is_t( 0 ) );

  private:

    struct data
    {
        bool_t used = false;

        float_t start_sig = 0.0f;
        float_t start_time = 0.0f;
        float_t last_time = 0.0f;
        float_t reset_sig_value = 0.0f;
        bool_t reset_sig = false;

        // how long is the puls active
        time_is_t * duration_is = motor::shared( time_is_t( -1.0f ) );

        // if active : 1.0
        // if inactive : 0.0
        active_os_t * active_os = motor::shared( active_os_t( 0.0f ) );

        signal_is_t * sig_is = motor::shared( signal_is_t( 0.0f ) );
        signal_os_t * sig_os = motor::shared( signal_os_t( 0.0f ) );

        this_t::trigger_behavoir tb = this_t::trigger_behavoir::ignore_while_active;

        // by default, the filtered output will be linear.
        filter_kernel_t fk = []( this_t::filter_data const & d )
        {
            return d.sig_abs;
        };
    };
    motor::vector< data > _datas;

  public:

    trigger_filter_node( motor::string_in_t name ) noexcept : base_t( name, this_t::make_funk() )
    {
        this_t::init_slots();
    }

    trigger_filter_node( void_t ) noexcept : trigger_filter_node( "trigger/filter" ) {}

    trigger_filter_node( this_cref_t ) = delete;
    trigger_filter_node( this_rref_t rhv ) noexcept
        : base_t( std::move( rhv ) ), _time_is( motor::move( rhv._time_is ) ),
          _trigger_is( motor::move( rhv._trigger_is ) ), _datas( std::move( rhv._datas ) )
    {
        base_t::set_funk( this_t::make_funk() );
    }

    virtual ~trigger_filter_node( void_t ) noexcept
    {
        if( _time_is ) _time_is->disconnect();
        if( _trigger_is ) _trigger_is->disconnect();

        for( auto & d : _datas )
        {
            active_os_t::release( motor::move( d.active_os ) );
            time_is_t::release( motor::move( d.duration_is ) );
            signal_is_t::release( motor::move( d.sig_is ) );
            signal_os_t::release( motor::move( d.sig_os ) );
        }
    }

    struct add_data
    {
        float_t duration = -1.0f;
        this_t::trigger_behavoir tb = this_t::trigger_behavoir::ignore_while_active;
    };

    size_t add_filter( this_t::filter_kernel_t funk, add_data d ) noexcept
    {
        size_t idx = size_t( -1 );
        while( ++idx < _datas.size() && _datas[ idx ].used );

        if( idx == _datas.size() )
        {
            _datas.emplace_back( this_t::data{} );
        }

        _datas[ idx ].used = true;
        _datas[ idx ].fk = std::move( funk );
        _datas[ idx ].tb = d.tb;
        _datas[ idx ].duration_is->set_value( d.duration );

        return idx;
    }

    size_t add_filter( this_t::filter_kernel_t funk ) noexcept
    {
        return this_t::add_filter( std::move( funk ), {} );
    }

    void_t remove_filter( size_t const id ) noexcept
    {
        if( _datas.size() <= id || !_datas[ id ].used ) return;

        auto & d = _datas[ id ];

        d.used = false;
        d.fk = {};

        d.active_os->set_and_exchange( 0.0f );
        d.active_os->disconnect();

        d.duration_is->set_value( -1.0f );
        d.duration_is->disconnect();

        d.reset_sig = false;
        d.sig_is->set_value( 0.0f );
        d.sig_is->disconnect();
        d.sig_os->set_value( 0.0f );
        d.sig_os->disconnect();
        d.start_time = 0.0f;
    }

    bool_t connect_signal_is(
        size_t const id, motor::core::mtr_safe< signal_os_t > os ) const noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used )
        {
            motor::release( motor::move( os ) );
            return false;
        }
        return _datas[ id ].sig_is->connect( motor::move( os ) );
    }

    bool_t connect_signal_os(
        size_t const id, motor::core::mtr_safe< signal_is_t > is ) const noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used )
        {
            motor::release( motor::move( is ) );
            return false;
        }
        return _datas[ id ].sig_os->connect( motor::move( is ) );
    }

    float_t get_filter_output_signal( size_t const id ) const noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used ) return false;
        return _datas[ id ].sig_os->get_value();
    }

    bool_t connect_active_os(
        size_t const id, motor::core::mtr_safe< active_is_t > is ) const noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used )
        {
            motor::release( motor::move( is ) );
            return false;
        }
        return _datas[ id ].active_os->connect( motor::move( is ) );
    }

    bool_t is_filter_active( size_t const id ) const noexcept
    {
        bool_t const cond = id < _datas.size() && _datas[ id ].used;
        return cond && _datas[ id ].active_os->get_value() > 0.5f;
    }

    bool_t connect_duration_is(
        size_t const id, motor::core::mtr_safe< time_os_t > os ) const noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used )
        {
            motor::release( motor::move( os ) );
            return false;
        }
        return _datas[ id ].duration_is->connect( motor::move( os ) );
    }

    bool_t set_duration( size_t const id, float_t const dur ) noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used ) return false;
        _datas[ id ].duration_is->set_value( dur );
        return true;
    }

    bool_t connect_time_is( motor::core::mtr_safe< time_os_t > os ) noexcept
    {
        return _time_is->connect( motor::move( os ) );
    }

    bool_t connect_trigger_is( motor::core::mtr_safe< trigger_os_t > os ) noexcept
    {
        return _trigger_is->connect( motor::move( os ) );
    }

    void_t trigger( void_t ) noexcept
    {
        _trigger_is->set_value( _trigger_is->get_value() + 1 );
    }

    bool_t reset_on_detrigger( size_t const id, bool_t const b ) noexcept
    {
        if( id >= _datas.size() || !_datas[ id ].used ) return false;
        _datas[ id ].reset_sig = b;
        return true;
    }

  private:

    base_t::funk_t make_funk( void_t ) noexcept
    {
        return [ = ]( base_t * self_in )
        {
            auto * self = static_cast< this_t * >( self_in );
            assert( self != nullptr );

            float_t const cur_time = _time_is->get_value();

            // reset trigger
            for( auto & d : self->_datas )
            {
                float_t const start_time = d.start_time;
                float_t const duration = d.duration_is->get_value();

                auto const duration_zero =
                    std::abs( duration ) <= std::numeric_limits< float_t >::epsilon();

                if( !d.used || d.active_os->get_value() < 0.5f || duration < 0.0f ) continue;

                if( ( start_time > cur_time ) || duration_zero ||
                    ( cur_time - start_time >= duration ) )
                {
                    d.start_time = 0.0f;
                    d.start_sig = 0.0f;
                    d.last_time = 0.0f;
                    d.active_os->set_and_exchange( 0.0f );
                    if( d.reset_sig ) d.sig_os->set_and_exchange( d.reset_sig_value );
                }
            }

            // check if we need to retrigger.
            if( _trigger_is->has_changed() && _trigger_is->get_value_and_reset() != 0 )
            {
                for( auto & d : self->_datas )
                {
                    float_t const duration = d.duration_is->get_value();
                    auto const duration_zero =
                        std::abs( duration ) <= std::numeric_limits< float_t >::epsilon();

                    if( !d.used || duration_zero ) continue;

                    // if inactive => activate
                    if( d.active_os->get_value() < 0.5f ||
                        d.tb == this_t::trigger_behavoir::reset_on_trigger )
                    {
                        if( d.active_os->get_value() < 0.5f ) d.active_os->set_and_exchange( 1.0f );
                        d.start_time = cur_time;
                        d.last_time = cur_time;
                        d.start_sig = d.sig_is->get_value();
                    }
                }
                _trigger_is->set_value( 0 );
            }

            // evaluate filter
            for( auto & d : self->_datas )
            {
                if( !d.used || d.active_os->get_value() < 0.5f ) continue;

                float_t const duration = d.duration_is->get_value();

                this_t::filter_data fd{
                    d.sig_is->get_value(),             // signal
                    d.start_sig,                       // start signal
                    cur_time,                          // time
                    cur_time - d.start_time,           // time rel
                    duration < 0.0f ? 1.0f : duration, // duration
                    cur_time - d.last_time             // dt
                };

                d.sig_os->set_and_exchange( d.fk( fd ) );
                d.last_time = cur_time;
            }
        };
    }

    void_t init_slots( void_t ) noexcept
    {
        base_t::inputs().add( motor::share_unsafe( _time_is ) );
        base_t::inputs().add( motor::share_unsafe( _trigger_is ) );
    }
};
motor_typedef( trigger_filter_node );
} // namespace motor::wire