#include "object.h"

using namespace motor::graphics;

object::data_manipulator::data_manipulator( object * ptr, size_t const bid ) noexcept
    : _obj( ptr ), _bid( bid )
{
}
object::data_manipulator::~data_manipulator( void_t ) noexcept {}
bool_t object::data_manipulator::change_to_in_transit( void_t ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() )
    {
        _obj->_datas.emplace_back( backend_data_t( { _bid, size_t( -1 ), false, false,
            motor::graphics::object_state::in_transit, motor::graphics::result::invalid } ) );
        return true;
    }

    if( _obj->_datas[ i ].os == motor::graphics::object_state::in_transit ) return false;
    _obj->_datas[ i ].os = motor::graphics::object_state::in_transit;
    return true;
}

bool_t object::data_manipulator::change_if( motor::graphics::object_state const os,
    std::function< bool_t( motor::graphics::object_state const ) > funk ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() ) return false;
    if( !funk( _obj->_datas[ i ].os ) ) return false;
    _obj->_datas[ i ].os = os;
    return true;
}

bool_t object::data_manipulator::change( motor::graphics::result const res ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() ) return false;
    _obj->_datas[ i ].res = res;
    return true;
}

// only change the internal result if the last one was not ok.
// if the last one was ok but the current is not ok, the object
// still is considered ok.
bool_t object::data_manipulator::change_to_ready( motor::graphics::result const res ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() ) return false;

    if( _obj->_datas[ i ].res != motor::graphics::result::ok ) //
        _obj->_datas[ i ].res = res;

    if( _obj->_datas[ i ].res == motor::graphics::result::ok )
        _obj->_datas[ i ].os = motor::graphics::object_state::ready;
    else
        _obj->_datas[ i ].os = motor::graphics::object_state::raw;

    return true;
}

bool_t object::data_manipulator::change_to_raw( motor::graphics::result const res ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() ) return false;

    _obj->_datas[ i ].res = res;
    _obj->_datas[ i ].os = motor::graphics::object_state::raw;

    return true;
}

object::data_manipulator::state_pair_t object::data_manipulator::get_status( void_t ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );
    if( i == _obj->_datas.size() )
    {
        return std::make_pair(
            motor::graphics::object_state::invalid, motor::graphics::result::invalid );
    }
    else
    {
        return std::make_pair( _obj->_datas[ i ].os, _obj->_datas[ i ].res );
    }
}

bool_t object::data_manipulator::set_in_transit( void_t ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() ) return false;

    _obj->_datas[ i ].is_in_transit = true;
    return true;
}

bool_t object::data_manipulator::reset_in_transit( void_t ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _obj->_mutex );

    size_t i = size_t( -1 );
    while( ( _obj->_datas.size() > ++i ) && ( _obj->_datas[ i ].bid != _bid ) );

    if( i == _obj->_datas.size() ) return false;

    _obj->_datas[ i ].is_in_transit = false;
    return true;
}

bool_t object::is_ready( size_t const bid ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
    size_t i = size_t( -1 );
    while( ++i < _datas.size() && _datas[ i ].bid != bid );

    if( i == _datas.size() ) return false;

    return _datas[ i ].os == motor::graphics::object_state::ready;
}

// are all backend objects ready.
// ready means successfully configured.
bool_t object::is_ready( void_t ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );

    bool_t b = true;
    for( auto const & d : _datas )
    {
        if( d.os != motor::graphics::object_state::ready )
        {
            b = false;
            break;
        }
    }

    return b;
}

// is any backend object in transit?
bool_t object::is_any_in_transit( void_t ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );

    for( auto const & d : _datas )
    {
        if( d.os == motor::graphics::object_state::in_transit )
        {
            return true;
        }
    }

    return false;
}

object::object( void_t ) noexcept {}
object::object( this_cref_t rhv ) noexcept : _datas( rhv._datas ) {}
object::object( this_rref_t rhv ) noexcept : _datas( std::move( rhv._datas ) ) {}
object::~object( void_t ) noexcept {}

object::this_ref_t object::operator=( this_cref_t rhv ) noexcept
{
    _datas = rhv._datas;
    return *this;
}

object::this_ref_t object::operator=( this_rref_t rhv ) noexcept
{
    _datas = std::move( rhv._datas );
    return *this;
}

size_t object::set_oid( size_t const bid, size_t const oid ) noexcept
{
    // update oid ...
    {
        motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
        for( auto & id : _datas )
        {
            if( id.bid == bid )
            {
                id.oid = oid;
                return oid;
            }
        }
    }

    // ... or create new
    {
        motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );
        _datas.emplace_back( backend_data_t( { bid, oid, false, false,
            motor::graphics::object_state::raw, motor::graphics::result::invalid } ) );
    }

    return oid;
}

size_t object::get_oid( size_t const bid ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
    for( auto & id : _datas )
    {
        if( id.bid == bid )
        {
            return id.oid;
        }
    }
    return size_t( -1 );
}

void_t object::remove_bid( size_t const bid ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );
    auto iter = std::find_if(
        _datas.begin(), _datas.end(), [ & ]( backend_data_t const & d ) { return d.bid == bid; } );

    if( iter == _datas.end() ) return;

    _datas.erase( iter );
}

void_t object::set_changed( void_t ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );
    for( auto & id : _datas )
    {
        id.changed = true;
    }
}

bool_t object::has_changed( size_t const bid ) const noexcept
{
    motor::concurrent::mrsw_t::reader_lock_t lk( _mutex );
    for( size_t i = 0; i < _datas.size(); ++i )
    {
        if( _datas[ i ].bid == bid ) return _datas[ i ].changed;
    }
    return false;
}

void_t object::reset_changed( size_t const bid ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );
    for( size_t i = 0; i < _datas.size(); ++i )
    {
        if( _datas[ i ].bid == bid )
        {
            _datas[ i ].changed = false;
            break;
        }
    }
}

bool_t object::check_and_reset_changed( size_t const bid ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mutex );
    for( size_t i = 0; i < _datas.size(); ++i )
    {
        if( _datas[ i ].bid == bid )
        {
            bool_t const ret = _datas[ i ].changed;
            _datas[ i ].changed = false;
            return ret;
        }
    }
    return false;
}