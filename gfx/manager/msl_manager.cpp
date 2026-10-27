// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "msl_manager.h"

#include <motor/io/global.h>

using namespace motor::gfx;

//**************************************************************************
msl_manager::msl_manager( motor::io::database_mtr_safe_t db ) noexcept : _db( motor::move( db ) )
{
    this_t::start_thread();
}

//**************************************************************************
msl_manager::msl_manager( this_rref_t rhv ) noexcept
    : _db( motor::move( rhv._db ) ), _location_to_id( std::move( rhv._location_to_id ) ),
      _name_to_id( std::move( rhv._name_to_id ) ), _msls( std::move( rhv._msls ) ),
      _msls_config( std::move( rhv._msls_config ) ),
      _msls_release( std::move( rhv._msls_release ) ),
      _configures_in_process( std::move( rhv._configures_in_process ) ),
      _loads( std::move( rhv._loads ) ), _directs( std::move( rhv._directs ) ),
      _conf_funks( std::move( rhv._conf_funks ) ),
      _conf_funk_on_msls( std::move( rhv._conf_funk_on_msls ) ),
      _conf_funk_on_fresh( std::move( rhv._conf_funk_on_fresh ) )
{
    rhv.stop_thread();
    this_t::start_thread( true );
}

//**************************************************************************
msl_manager::~msl_manager( void_t ) noexcept
{
    this_t::stop_thread();

    for( auto & item : _msls )
    {
        motor::release( motor::move( item.mon ) );
        motor::release( motor::move( item.msl ) );
    }
    motor::release( motor::move( _db ) );
}

//**************************************************************************
bool_t msl_manager::add( motor::string_cref_t name, motor::io::location_cref_t loc,
    completion_funk_t comp_funk ) noexcept
{
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

        auto iter = _location_to_id.find( loc.as_string() );
        if( iter != _location_to_id.end() )
        {
            // return what?
            // id
            // msl
        }

        auto ca = _db->load( loc );
        _loads.emplace_back( load_operation{ loc, name, std::move( ca ), comp_funk } );
    }

    {
        std::lock_guard< std::mutex > lk( _sd->mtx );
        _sd->has_work = true;
    }
    _sd->cv.notify_one();

    return true;
}

//**************************************************************************
bool_t msl_manager::add(
    motor::string_cref_t name, motor::string_in_t shader, completion_funk_t comp_funk ) noexcept
{
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );
        _directs.emplace_back( direct_operation{ name, shader, comp_funk } );
    }

    {
        std::lock_guard< std::mutex > lk( _sd->mtx );
        _sd->has_work = true;
    }
    _sd->cv.notify_one();

    return true;
}

//**************************************************************************
void_t msl_manager::on_update( void_t ) noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );

    // check the in progress msls and call the callback
    {
        auto iter = _configures_in_process.begin();
        while( iter != _configures_in_process.end() )
        {
            size_t const id = *iter;
            if( !( _msls[ id ].msl->is_ready() ) )
            {
                _msls[ id ].is_in_process_list = true;
                ++iter;
                continue;
            }

            for( size_t i = 0; i < _conf_funk_on_fresh.size(); ++i )
            {
                if( _conf_funk_on_fresh[ i ] == size_t( -1 ) ) continue;

                _conf_funks[ _conf_funk_on_fresh[ i ] ]( _msls[ id ].msl->name(), _msls[ id ].msl );
            }
            _msls[ id ].conf_funk_called = true;
            _msls[ id ].is_in_process_list = false;

            iter = _configures_in_process.erase( iter );
        }
    }

    // call for each msl that is already ready
    {
        auto iter = _msls.begin();
        while( iter != _msls.end() )
        {
            auto res = iter->msl->is_ready();
            if( res && !iter->conf_funk_called && !iter->is_in_process_list )
            {
                for( size_t i = 0; i < _conf_funk_on_msls.size(); ++i )
                {
                    if( _conf_funk_on_msls[ i ] == size_t( -1 ) ) continue;
                    _conf_funks[ _conf_funk_on_msls[ i ] ]( iter->msl->name(), iter->msl );
                }
            }
            iter->conf_funk_called = false;

            ++iter;
        }

        _conf_funk_on_msls.clear();
    }
}

//**************************************************************************
void_t msl_manager::on_render( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

    for( auto const idx : _msls_config )
    {
        auto & item = _msls[ idx ];
        fe->configure< motor::graphics::msl_object_t >( item.msl );
    }

    for( auto const idx : _msls_release )
    {
        auto & item = _msls[ idx ];
        fe->release< motor::graphics::msl_object_t >( item.msl );
    }

    for( auto idx : _configures_in_process )
    {
        auto & item = _msls[ idx ];
        auto [ state, result ] = fe->decode( item.msl );
        if( state == motor::graphics::object_state::invalid ||
            state == motor::graphics::object_state::raw )
        {
            fe->configure< motor::graphics::msl_object_t >( item.msl );
        }
    }
}

//**************************************************************************
void_t msl_manager::on_render_init( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
    for( auto & item : _msls )
    {
        auto [ state, result ] = fe->decode( item.msl );
        if( state != motor::graphics::object_state::ready &&
            state != motor::graphics::object_state::in_transit )
        {
            fe->configure< motor::graphics::msl_object_t >( item.msl );
        }
    }
}

//**************************************************************************
void_t msl_manager::on_render_release( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
    for( auto & item : _msls )
    {
        fe->release< motor::graphics::msl_object_t >( item.msl );
    }
}

//**************************************************************************
void_t msl_manager::on_frame_done( void_t ) noexcept
{
    _msls_config.clear();
    _msls_release.clear();
}

//**************************************************************************
void_t msl_manager::start_thread( bool_t const has_work ) noexcept
{
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

        if( _sd != nullptr ) return;

        {
            this_t::shared_data sd;
            sd.has_work = has_work;
            sd.owner = this;
            sd.running = true;

            _sd = motor::shared( std::move( sd ) );
        }
    }

    _th = std::thread( [ & ]( void_t )
    {
        motor::log::global_t::status( "[msl_manager] : thread started" );

        while( _sd->running )
        {
            // little shortcut
            this_ptr_t owner = _sd->owner;

            {
                std::unique_lock< std::mutex > lk( _sd->mtx );
                while( !_sd->has_work && _sd->running ) _sd->cv.wait( lk );
                _sd->has_work = false;
            }

            struct tmp_
            {
                motor::string_t name;
                motor::io::location_t loc;
                motor::string_t shd;
                this_t::completion_funk_t comp_funk;
            };
            motor::vector< tmp_ > tmps;

            // #1 collecing all shaders from the database
            {
                decltype( _loads ) tmp;
                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _sd->owner->_mutex );
                    tmp = std::move( _sd->owner->_loads );
                }

                for( auto & item : tmp )
                {
                    motor::string_t shd;

                    {
                        item.ca.wait_for_operation(
                            [ & ]( char_cptr_t data, size_t const sib, motor::io::result const res )
                        {
                            if( res != motor::io::result::ok )
                            {
                                motor::log::global_t::warning(
                                    "[msl_manager] : could not load file : " +
                                    item.loc.as_string() );
                                return;
                            }

                            shd = motor::string_t( data, sib );
                        } );
                    }

                    tmps.emplace_back( tmp_{ item.name, item.loc, shd, item.comp_funk } );
                }
            }

            // #2 collecting all the shaders directly added
            {
                decltype( _directs ) tmp;
                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _sd->owner->_mutex );
                    tmp = std::move( _sd->owner->_directs );
                }

                for( auto & item : tmp )
                {
                    tmps.emplace_back(
                        tmp_{ item.name, motor::io::location_t(), item.shd, item.comp_funk } );
                }
            }

            // create/update msl object
            {
                for( auto & item : tmps )
                {
                    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

                    auto iter = _name_to_id.find( item.name );
                    if( iter != _name_to_id.end() )
                    {
                        size_t const idx = ( *iter ).second;

                        if( _msls[ idx ].msl->is_any_in_transit() )
                        {
                            motor::log::global_t::warning(
                                "[msl_manager] : the shader is currently being processed: " +
                                item.name );
                        }
                        else
                        {
                            // this would be a reconfigure.
                            _msls[ idx ].msl->clear_shaders().add(
                                motor::graphics::msl_api_type::msl_4_0, item.shd );
                            _msls_config.emplace_back( idx );

                            item.comp_funk( { this_t::completion_stage::msl_reload, item.name,
                                _msls[ idx ].msl } );

                            _configures_in_process.emplace_back( idx );
                        }
                    }
                    else
                    {
                        motor::graphics::msl_object_t msl( item.name, true );
                        msl.add( motor::graphics::msl_api_type::msl_4_0, item.shd );

                        this_t::msl_data md;
                        md.msl = motor::shared( std::move( msl ) );
                        md.mon = motor::shared( motor::io::monitor_t() );
                        _msls_config.emplace_back( _msls.size() );

                        item.comp_funk(
                            { this_t::completion_stage::msl_reload, item.name, md.msl } );

                        _configures_in_process.emplace_back( _msls.size() );

                        _msls.emplace_back( std::move( md ) );
                    }
                }
            }
        }

        motor::log::global_t::status( "[msl_manager] : thread stopped" );
    } );
}

//**************************************************************************
void_t msl_manager::stop_thread( void_t ) noexcept
{
    if( _sd == nullptr ) return;

    {
        std::lock_guard< std::mutex > _lk( _sd->mtx );
        _sd->running = false;
    }
    _sd->cv.notify_one();

    if( _th.joinable() ) _th.join();

    motor::release( motor::move( _sd ) );
}

//**************************************************************************
size_t msl_manager::register_funk( on_configure_funk_t f ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

    size_t i = size_t( -1 );

    // insert into comp funk container
    {
        while( ++i < _conf_funks.size() && _conf_funks[ i ] );

        if( i == _conf_funks.size() ) _conf_funks.resize( _conf_funks.size() + 5, nullptr );

        _conf_funks[ i ] = std::move( f );
    }

    // insert into indirection containers
    {
        auto insertion_funk = []( motor::vector< size_t > & cont, size_t const idx )
        {
            size_t i = size_t( -1 );
            while( ++i < cont.size() && cont[ i ] != size_t( -1 ) );

            if( i == cont.size() ) cont.resize( cont.size() + 5, size_t( -1 ) );

            cont[ i ] = idx;

            return i;
        };

        insertion_funk( _conf_funk_on_msls, i );
        insertion_funk( _conf_funk_on_fresh, i );
    }

    return i;
}

//**************************************************************************
size_t msl_manager::unregister_funk( size_t const idx ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );

    if( idx >= _conf_funks.size() ) return size_t( -1 );

    auto remove_funk = []( motor::vector< size_t > & cont, size_t const idx )
    {
        size_t i = size_t( -1 );
        while( ++i < cont.size() && cont[ i ] != idx );
        if( i != cont.size() ) cont[ i ] = size_t( -1 );
    };

    remove_funk( _conf_funk_on_msls, idx );
    remove_funk( _conf_funk_on_fresh, idx );

    _conf_funks[ idx ] = nullptr;

    return size_t( -1 );
}
