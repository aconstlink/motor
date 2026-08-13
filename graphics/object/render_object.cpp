
#include "render_object.h"

using namespace motor::graphics;

render_object::render_object( void_t ) noexcept {}
render_object::render_object( motor::string_cref_t name ) noexcept : _name( name ) {}

render_object::~render_object( void_t ) noexcept
{
    for( auto * v : _vars ) motor::memory::release_ptr( v );
}

render_object::render_object( this_cref_t rhv ) noexcept
    : object( rhv ), _name( rhv._name ), _geo( rhv._geo ), _shader( rhv._shader ),
      _states( rhv._states ), _soo( rhv._soo )
{
    _vars.resize( rhv._vars.size() );
    for( size_t i = 0; i < rhv._vars.size(); ++i )
        _vars[ i ] = motor::memory::copy_ptr( rhv._vars[ i ] );
}

render_object::render_object( this_rref_t rhv ) noexcept
    : object( std::move( rhv ) ), _name( std::move( rhv._name ) ), _geo( std::move( rhv._geo ) ),
      _shader( std::move( rhv._shader ) ), _states( std::move( rhv._states ) ),
      _soo( std::move( rhv._soo ) )
{
    for( auto * vs : _vars ) motor::memory::release_ptr( vs );

    _vars.resize( rhv._vars.size() );
    for( size_t i = 0; i < rhv._vars.size(); ++i )
        _vars[ i ] = motor::memory::copy_ptr( rhv._vars[ i ] );
}

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

render_object::this_ref_t render_object::operator=( this_rref_t rhv ) noexcept
{
    object::operator=( std::move( rhv ) );

    _name = std::move( rhv._name );
    _geo = std::move( rhv._geo );
    _shader = std::move( rhv._shader );
    _states = std::move( rhv._states );
    _soo = std::move( rhv._soo );

    for( auto * v : _vars ) motor::memory::release_ptr( v );
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
    size_t i = size_t( -1 );
    while( _geo.size() > ++i && _geo[ i ].name != name );

    if( i != _geo.size() )
    {
        ++_geo[ i ].hash;
        ++_geo[ i ].ref_count;
        return i;
    }

    {
        _geo.emplace_back( this_t::geometry_link_t{ 0, 1, name } );
    }

    return _geo.size() - 1;
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
    _vars.emplace_back( vs );
    return _vars.size() - 1;
}

render_object::this_ref_t render_object::add_variable_sets(
    motor::vector< motor::graphics::variable_set_mtr_safe_t > && vss ) noexcept
{
    for( auto & utr : vss ) _vars.emplace_back( motor::move( utr ) );

    return *this;
}

render_object::this_ref_t render_object::remove_variable_sets( void_t ) noexcept
{
    for( auto * v : _vars ) motor::memory::release_ptr( v );
    _vars.clear();
    return *this;
}

void_t render_object::for_each( for_each_var_funk_t funk ) noexcept
{
    size_t i = size_t( -1 );
    for( auto const & v : _vars )
    {
        funk( ++i, v );
    }
}

size_t render_object::get_num_variable_sets( void_t ) const noexcept
{
    return _vars.size();
}

motor::graphics::variable_set_mtr_safe_t render_object::get_variable_set( size_t const i ) noexcept
{
    this_t::fill_variable_sets( i );
    return motor::share( _vars[ i ] );
}

motor::vector< motor::graphics::variable_set_mtr_safe_t > render_object::get_varibale_sets(
    void_t ) const noexcept
{
    motor::vector< motor::graphics::variable_set_mtr_safe_t > ret;

    for( auto * mtr : _vars ) ret.emplace_back( motor::share( mtr ) );

    return ret;
}

motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > &
render_object::borrow_varibale_sets( void_t ) noexcept
{
    return _vars;
}

motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > const &
render_object::borrow_varibale_sets( void_t ) const noexcept
{
    return _vars;
}

// fast version for quick access without ref counting
motor::graphics::variable_set_borrow_t::mtr_t render_object::borrow_variable_set(
    size_t const i ) const noexcept
{
    return _vars.size() <= i ? nullptr : _vars[ i ];
}

render_object::this_ref_t render_object::fill_variable_sets( size_t const idx ) noexcept
{
    if( idx < _vars.size() ) return *this;

    auto old = std::move( _vars );
    _vars.resize( idx + 1 );

    for( size_t i = 0; i < old.size(); ++i ) _vars[ i ] = old[ i ];
    for( size_t i = old.size(); i < _vars.size(); ++i )
        _vars[ i ] = motor::shared( motor::graphics::variable_set_t() );

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