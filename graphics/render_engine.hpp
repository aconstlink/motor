// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/std/vector>
#include <motor/log/global.h>

#include <mutex>
#include <functional>
#include <array>

namespace motor
{
namespace graphics
{

//  @todo rename to command_engine
// the name render_engine is maybe a little bit misleading.
class render_engine
{
    motor_this_typedefs( render_engine );

  public:

    using notify_funk_t = std::function< void_t( void_t ) >;
    using commit_snapshot_funk_t = std::function< void_t( void_t ) >;

    struct command
    {
        using funk_t = std::function< void_t( void_t ) >;

        funk_t exec_funk;
        funk_t cancel_funk = []( void_t ) {
        };
    };
    motor_typedef( command );

  private:

    mutable std::mutex _mtx_has_commands;
    bool_t _has_commands = false;

    // update commands
    motor::vector< command_t > _coms_update;

    // execute (like rendering, compute) commands
    ushort_t _coms_exec_read_idx = 0;
    std::array< motor::vector< command_t >, 2 > _coms_exec;

    // if someone else needs to know if work has arrieved.
    // so that other entity can be woken up.
    // this functionality is currently reserved for the
    // application carrier with the the render pipeline
    notify_funk_t _notify_has_work;
    commit_snapshot_funk_t _commit_snapshot = []( void_t ) {
    };

    ushort_t coms_exec_read_idx( void_t ) const noexcept
    {
        return _coms_exec_read_idx;
    }

    ushort_t coms_exec_write_idx( void_t ) const noexcept
    {
        return ( _coms_exec_read_idx + 1 ) % 2;
    }

  public:

    render_engine( void_t ) noexcept {}
    render_engine( this_cref_t ) = delete;
    render_engine( this_rref_t rhv ) noexcept
        : _has_commands( rhv._has_commands ), _coms_update( std::move( rhv._coms_update ) ),
          _coms_exec( std::move( rhv._coms_exec ) ),
          _notify_has_work( std::move( rhv._notify_has_work ) ),
          _commit_snapshot( std::move( rhv._commit_snapshot ) ),
          _coms_exec_read_idx( rhv._coms_exec_read_idx )
    {
    }
    ~render_engine( void_t ) noexcept
    {
        std::lock_guard< std::mutex > lk( _mtx_has_commands );

        for( auto & c : _coms_exec[ 0 ] )
        {
            c.cancel_funk();
        }

        for( auto & c : _coms_exec[ 1 ] )
        {
            c.cancel_funk();
        }

        _coms_update.clear();
        _coms_exec[ 0 ].clear();
        _coms_exec[ 1 ].clear();
        _has_commands = false;
    }

  public:

    // clear out command queue
    void_t force_clear( void_t ) noexcept
    {
        std::lock_guard< std::mutex > lk( _mtx_has_commands );

        for( auto & c : _coms_update )
        {
            c.cancel_funk();
        }
        _coms_update.clear();

        for( auto & c : _coms_exec[ 0 ] )
        {
            c.cancel_funk();
        }

        for( auto & c : _coms_exec[ 1 ] )
        {
            c.cancel_funk();
        }

        _coms_exec[ 0 ].clear();
        _coms_exec[ 1 ].clear();
        _has_commands = false;
    }

    bool_t can_execute( void_t ) noexcept
    {
        return this_t::has_frame_commands();
    }

    // returns true if a frame has been rendered
    // otherwise false
    bool_t execute_frame( void_t ) noexcept
    {
        assert( _has_commands == true );

        // 1. execute prepare commands
        for( auto & c : _coms_update )
        {
            c.exec_funk();
        }
        _coms_update.clear();

        // 2. commit all data, i.e.
        // do the frame snapshot
        _commit_snapshot();

        // @todo this should go after _commit_snapshot()
        // X. swap when upstream commands are processed
        // At this point, the outer entity can write in the back buffer again
        this_t::swap_has_commands();

        // 3. execute "execute" commands
        for( auto & c : _coms_exec[ this_t::coms_exec_read_idx() ] )
        {
            c.exec_funk();
        }
        _coms_exec[ this_t::coms_exec_read_idx() ].clear();

        return true;
    }

  public: // interface for external entity

    bool_t can_enter_frame( void_t ) const noexcept
    {
        std::lock_guard< std::mutex > lk( _mtx_has_commands );
        return !_has_commands;
    }

    // only enters if no commands are in the back buffer
    bool_t enter_frame( void_t ) noexcept
    {
        std::lock_guard< std::mutex > lk( _mtx_has_commands );
        return !_has_commands;
    }

    void_t leave_frame( void_t ) noexcept
    {
        {
            std::lock_guard< std::mutex > lk( _mtx_has_commands );
            _has_commands = true;
        }

        if( _notify_has_work ) _notify_has_work();
    }

    // !not thread safe! Only use within enter_frame/leave_frame
    void_t send_update( this_t::command_t c ) noexcept
    {
        _coms_update.emplace_back( std::move( c ) );
    }

    // !not thread safe! Only use within enter_frame/leave_frame
    void_t send_execute( this_t::command_t c ) noexcept
    {
        _coms_exec[ this_t::coms_exec_write_idx() ].emplace_back( std::move( c ) );
    }

    void_t set_commit_snapshot_funk( this_t::commit_snapshot_funk_t && f ) noexcept
    {
        _commit_snapshot = std::move( f );
    }

  public: // reserved for the application carrier

    void_t set_work_notify( notify_funk_t f ) noexcept
    {
        _notify_has_work = f;
    }

  private: // has commands specific

    void_t swap_has_commands( void_t ) noexcept
    {
        {
            std::lock_guard< std::mutex > lk( _mtx_has_commands );
            _has_commands = false;
            _coms_exec_read_idx = ( _coms_exec_read_idx + 1 ) % 2;
        }

        if( _notify_has_work ) _notify_has_work();
    }

    bool_t has_frame_commands( void_t ) const noexcept
    {
        std::lock_guard< std::mutex > lk( _mtx_has_commands );
        return _has_commands;
    }
};
motor_typedef( render_engine );
} // namespace graphics
} // namespace motor