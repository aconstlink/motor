
#include "render_object.h"

using namespace motor::graphics;

render_object::render_object( void_t ) noexcept {}
render_object::render_object( motor::string_cref_t name ) noexcept : _name( name ) {}

render_object::~render_object( void_t ) noexcept
{
    for( auto & v : _vars ) motor::memory::release_ptr( v.vs );
}

#if 0
render_object::render_object( this_cref_t rhv ) noexcept
    : object( rhv ), _name( rhv._name ), _geo( rhv._geo ), _shader( rhv._shader ),
      _states( rhv._states ), _soo( rhv._soo )
{
    _vars.resize( rhv._vars.size() );
    for( size_t i = 0; i < rhv._vars.size(); ++i )
        _vars[ i ] = motor::memory::copy_ptr( rhv._vars[ i ] );
}
#endif

render_object::render_object( this_rref_t rhv ) noexcept
    : object( std::move( rhv ) ), _name( std::move( rhv._name ) ), _geo( std::move( rhv._geo ) ),
      _shader( std::move( rhv._shader ) ), _states( std::move( rhv._states ) ),
      _soo( std::move( rhv._soo ) )
{
    for( auto & vs : _vars ) motor::memory::release_ptr( vs.vs );
    _vars = std::move( rhv._vars );
}
#if 0
render_object::this_ref_t render_object::operator=( this_cref_t rhv ) noexcept
{
    object::operator=( rhv );

    _name = rhv._name;
    _geo = rhv._geo;
    _shader = rhv._shader;
    _states = rhv._states;
    _soo = rhv._soo;

    for( auto * vs : _vars ) motor::memory::release_ptr( vs );

    _vars.resize( rhv._vars.size() );
    for( size_t i = 0; i < rhv._vars.size(); ++i )
        _vars[ i ] = motor::memory::copy_ptr( rhv._vars[ i ] );

    return *this;
}
#endif
render_object::this_ref_t render_object::operator=( this_rref_t rhv ) noexcept
{
    object::operator=( std::move( rhv ) );

    _name = std::move( rhv._name );
    _geo = std::move( rhv._geo );
    _shader = std::move( rhv._shader );
    _states = std::move( rhv._states );
    _soo = std::move( rhv._soo );

    for( auto & v : _vars ) motor::memory::release_ptr( v.vs );
    _vars = std::move( rhv._vars );

    return *this;
}

void_t render_object::for_each_geometry_link( for_each_geo_link_funk_t funk ) const noexcept
{
    size_t i = size_t( -1 );
    for( auto const & gl : _geo )
    {
        funk( ++i, gl );
    }
}

size_t render_object::link_geometry( motor::string_cref_t name ) noexcept
{
    size_t free_i = size_t( -1 );
    size_t i = size_t( -1 );
    while( _geo.size() > ++i && _geo[ i ].name != name )
    {
        free_i = _geo[ i ].ref_count == 0 ? i : free_i;
    }

    if( i != _geo.size() )
    {
        ++_geo[ i ].hash;
        ++_geo[ i ].ref_count;
        return i;
    }

    if( free_i != size_t( -1 ) )
    {
        // just let is go...
        // so the backends can see a change and
        // react accodingly. Resetting the hash could
        // end up in a collision.
        ++_geo[ free_i ].hash;
        ++_geo[ free_i ].ref_count;
        _geo[ free_i ].name = name;
    }
    else
    {
        _geo.emplace_back( this_t::geometry_link_t{ 0, 1, name } );
    }

    return free_i != size_t( -1 ) ? free_i : _geo.size() - 1;
}

bool_t render_object::unlink_geometry( motor::string_cref_t name ) noexcept
{
    size_t i = size_t( -1 );
    while( _geo.size() > ++i && _geo[ i ].name != name );

    if( i == _geo.size() ) return false;

    return this_t::unlink_geometry( i );
}

bool_t render_object::unlink_geometry( size_t const geo_idx ) noexcept
{
    if( _geo.size() <= geo_idx ) return false;

    assert( _geo[ geo_idx ].ref_count > 0 );
    --_geo[ geo_idx ].ref_count;
    ++_geo[ geo_idx ].hash;

    if( _geo[ geo_idx ].ref_count == 0 )
    {
        _geo[ geo_idx ].name = "";
    }

    return true;
}

render_object::this_ref_t render_object::link_geometry(
    motor::vector< motor::string_t > const & names ) noexcept
{
    for( auto const & name : names )
    {
        this_t::link_geometry( name );
    }
    return *this;
}

// link to stream out object so geometry can be fed from there.
// the geometry is then mainly used for geometry layout.
render_object::this_ref_t render_object::link_geometry(
    motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept
{
    this_t::link_geometry( name );
    _soo.emplace_back( soo_name );
    return *this;
}

size_t render_object::get_num_geometry( void_t ) const noexcept
{
    return _geo.size();
}
size_t render_object::get_num_streamout( void_t ) const noexcept
{
    return _soo.size();
}

render_object::geometry_link_cref_t render_object::get_geometry_link(
    size_t const i ) const noexcept
{
    assert( i < _geo.size() );
    return _geo[ i ];
}

motor::vector< motor::string_t > const & render_object::get_streamouts( void_t ) const noexcept
{
    return _soo;
}

motor::string_cref_t render_object::get_streamout( size_t const i ) const noexcept
{
    return _soo[ i ];
}

bool_t render_object::has_streamout_link( void_t ) const noexcept
{
    return !_soo.empty();
}

render_object::this_ref_t render_object::link_shader( motor::string_cref_t name ) noexcept
{
    _shader = name;
    return *this;
}

motor::string_cref_t render_object::get_shader( void_t ) const noexcept
{
    return _shader;
}

size_t render_object::add_variable_set( motor::graphics::variable_set_mtr_safe_t vs ) noexcept
{
    size_t i = size_t( -1 );
    while( ++i < _vars.size() && _vars[ i ].vs != nullptr );

    if( i == _vars.size() )
    {
        _vars.emplace_back( this_t::variable_set_t{ 0, motor::move( vs ) } );
    }
    else
    {
        _vars[ i ].hash++;
        _vars[ i ].vs = motor::move( vs );
    }

    return i;
}

void_t render_object::drop_variable_set( size_t const idx ) noexcept
{
    if( _vars.size() <= idx ) return;

    _vars[ idx ].hash++;
    motor::release( motor::move( _vars[ idx ].vs ) );
}

render_object::this_ref_t render_object::add_variable_sets(
    motor::vector< motor::graphics::variable_set_mtr_safe_t > && vss ) noexcept
{
    for( auto & utr : vss ) _vars.emplace_back( this_t::variable_set{ 0, motor::move( utr ) } );

    return *this;
}

render_object::this_ref_t render_object::remove_variable_sets( void_t ) noexcept
{
    for( auto & v : _vars ) motor::memory::release_ptr( v.vs );
    _vars.clear();
    return *this;
}

void_t render_object::for_each( for_each_var_funk_t funk ) noexcept
{
    for( size_t i = 0; i < _vars.size(); ++i )
    {
        auto const & v = _vars[ i ];
        if( v.vs == nullptr ) continue;
        funk( i, v );
    }
}

size_t render_object::get_num_variable_sets( void_t ) const noexcept
{
    return _vars.size();
}

motor::graphics::variable_set_mtr_safe_t render_object::get_variable_set( size_t const i ) noexcept
{
    this_t::fill_variable_sets( i );
    return motor::share( _vars[ i ].vs );
}

render_object::safe_variable_set_t render_object::get_safe_variable_set( size_t const i ) noexcept
{
    this_t::fill_variable_sets( i );
    return safe_variable_set_t( _vars[ i ].hash, motor::share( _vars[ i ].vs ) );
}

motor::vector< motor::graphics::variable_set_mtr_safe_t > render_object::get_varibale_sets(
    void_t ) const noexcept
{
    motor::vector< motor::graphics::variable_set_mtr_safe_t > ret;

    for( auto & v : _vars ) ret.emplace_back( motor::share( v.vs ) );

    return ret;
}

motor::vector< render_object::variable_set_t > & render_object::borrow_varibale_sets(
    void_t ) noexcept
{
    return _vars;
}

motor::vector< render_object::variable_set_t > const & render_object::borrow_varibale_sets(
    void_t ) const noexcept
{
    return _vars;
}

// fast version for quick access without ref counting
render_object::variable_set_t render_object::borrow_variable_set( size_t const i ) const noexcept
{
    return _vars.size() <= i ? this_t::variable_set{ 0, nullptr } : _vars[ i ];
}

render_object::this_ref_t render_object::fill_variable_sets( size_t const idx ) noexcept
{
    if( idx < _vars.size() ) return *this;

    auto old = std::move( _vars );
    _vars.resize( idx + 1 );

    for( size_t i = 0; i < old.size(); ++i ) _vars[ i ] = old[ i ];
    for( size_t i = old.size(); i < _vars.size(); ++i )
    {
        _vars[ i ] =
            this_t::variable_set_t{ 0, motor::shared( motor::graphics::variable_set_t() ) };
    }

    return *this;
}

render_object::this_ref_t render_object::add_render_state_set(
    motor::graphics::render_state_sets_cref_t rs ) noexcept
{
    _states.emplace_back( rs );
    return *this;
}

void_t render_object::for_each( for_each_render_state_funk_t funk ) noexcept
{
    size_t i = 0;
    for( auto const & v : _states )
    {
        funk( i++, v );
    }
}

motor::string_cref_t render_object::name( void_t ) const noexcept
{
    return _name;
}