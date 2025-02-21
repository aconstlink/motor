

#include "msl_object.h"

using namespace motor::graphics ;

//****************************************************************************
msl_object::msl_object( void_t ) noexcept
{
    this_t::register_listener( motor::share( _comp_lst ) ) ;
}

//****************************************************************************
msl_object::msl_object( motor::string_in_t name ) noexcept : _name( name )
{
    this_t::register_listener( motor::share( _comp_lst ) ) ;
}

//****************************************************************************
msl_object::msl_object( this_rref_t rhv ) noexcept : object( std::move( rhv ) )
{
    _name = std::move( rhv._name ) ;
    _datas = std::move( rhv._datas ) ;
    _geo = std::move( rhv._geo ) ;
    _soo = std::move( rhv._soo ) ;

    for ( auto * vs : _vars ) motor::memory::release_ptr( vs ) ;
    _vars = std::move( rhv._vars ) ;

    // compilation listeners
    {
        motor::release( motor::move( _comp_lst ) ) ;
        _comp_lst = motor::move( rhv._comp_lst ) ;

        for ( auto * l : _compilation_listeners ) motor::memory::release_ptr( l ) ;
        _compilation_listeners = std::move( rhv._compilation_listeners ) ;
    }
}

//****************************************************************************
msl_object::this_ref_t msl_object::operator = ( this_rref_t rhv ) noexcept
{
    object::operator = ( std::move( rhv ) ) ;

    _name = std::move( rhv._name ) ;
    _datas = std::move( rhv._datas ) ;
    _geo = std::move( rhv._geo ) ;
    _soo = std::move( rhv._soo ) ;

    for ( auto * vs : _vars ) motor::memory::release_ptr( vs ) ;
    _vars = std::move( rhv._vars ) ;

    // compilation listeners
    {
        motor::release( motor::move( _comp_lst ) ) ;
        _comp_lst = motor::move( rhv._comp_lst ) ;

        for ( auto * l : _compilation_listeners ) motor::memory::release_ptr( l ) ;
        _compilation_listeners = std::move( rhv._compilation_listeners ) ;
    }

    return *this ;
}

//****************************************************************************
msl_object::msl_object( this_cref_t rhv ) noexcept : object( rhv )
{
    _name = rhv._name ;
    _datas = rhv._datas ;
    _geo = rhv._geo ;
    _soo = rhv._soo ;

    for ( auto * vs : _vars )
        motor::memory::release_ptr( vs ) ;

    _vars.resize( rhv._vars.size() ) ;
    for ( size_t i = 0; i < rhv._vars.size(); ++i )
        _vars[ i ] = motor::memory::copy_ptr( rhv._vars[ i ] ) ;

    // compilation listeners
    {
        if ( _comp_lst != rhv._comp_lst )
        {
            motor::release( motor::move( _comp_lst ) ) ;
            _comp_lst = motor::share( rhv._comp_lst ) ;
        }

        for ( auto * l : _compilation_listeners )
            motor::memory::release_ptr( l ) ;

        _compilation_listeners.resize( rhv._compilation_listeners.size() ) ;
        for ( size_t i = 0; i < rhv._compilation_listeners.size(); ++i )
            _compilation_listeners[ i ] = motor::share( rhv._compilation_listeners[ i ] ) ;
    }
}

//****************************************************************************
msl_object::this_ref_t msl_object::operator = ( this_cref_t rhv ) noexcept
{
    object::operator = ( rhv ) ;

    _name = rhv._name ;
    _datas = rhv._datas ;
    _geo = rhv._geo ;
    _soo = rhv._soo ;

    for ( auto * vs : _vars )
        motor::memory::release_ptr( vs ) ;

    _vars.resize( rhv._vars.size() ) ;
    for ( size_t i = 0; i < rhv._vars.size(); ++i )
        _vars[ i ] = motor::memory::copy_ptr( rhv._vars[ i ] ) ;

    // compilation listeners
    {
        if ( _comp_lst != rhv._comp_lst )
        {
            motor::release( motor::move( _comp_lst ) ) ;
            _comp_lst = motor::share( rhv._comp_lst ) ;
        }

        for ( auto * l : _compilation_listeners )
            motor::memory::release_ptr( l ) ;

        _compilation_listeners.resize( rhv._compilation_listeners.size() ) ;
        for ( size_t i = 0; i < rhv._compilation_listeners.size(); ++i )
            _compilation_listeners[ i ] = motor::share( rhv._compilation_listeners[ i ] ) ;
    }

    return *this ;
}

//****************************************************************************
msl_object::~msl_object( void_t ) noexcept
{
    for ( auto * vs : _vars )
        motor::memory::release_ptr( vs ) ;

    for ( auto * lst : _compilation_listeners )
        motor::release( motor::move( lst ) ) ;

    motor::release( motor::move( _comp_lst ) ) ;

}

//****************************************************************************
motor::string_cref_t msl_object::name( void_t ) const noexcept
{
    return _name ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::add( motor::graphics::msl_api_type const t, motor::string_in_t msl_code ) noexcept
{
    _datas.emplace_back( this_t::data { t, msl_code } ) ;
    return *this ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::clear_shaders( void_t ) noexcept
{
    _datas.clear() ;
    return *this ;
}

//****************************************************************************
void_t msl_object::for_each_msl( motor::graphics::msl_api_type const t, foreach_funk_t funk ) const noexcept
{
    for ( auto & d : _datas )
    {
        if ( d.t == t ) funk( d.code ) ;
    }
}

//****************************************************************************
msl_object::this_ref_t msl_object::link_geometry( motor::string_cref_t name ) noexcept
{
    _geo.emplace_back( name ) ;
    return *this ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::link_geometry( std::initializer_list< motor::string_t > const & names ) noexcept
{
    for ( auto const & name : names ) _geo.emplace_back( name ) ;
    return *this ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::link_geometry( motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept
{
    _geo.emplace_back( name ) ;
    _soo.emplace_back( soo_name ) ;
    return *this ;
}

//****************************************************************************
motor::vector< motor::string_t > const & msl_object::get_geometry( void_t ) const noexcept 
{ 
    return _geo ; 
}

//****************************************************************************
motor::vector< motor::string_t > const & msl_object::get_streamout( void_t ) const noexcept 
{ 
    return _soo ; 
}

//****************************************************************************
msl_object::this_ref_t msl_object::add_variable_set( motor::graphics::variable_set_mtr_safe_t vs ) noexcept
{
    _vars.emplace_back( vs ) ;
    return *this ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::fill_variable_sets( size_t const idx ) noexcept 
{
    if( idx < _vars.size() ) return *this ;

    auto old = std::move( _vars ) ;
    _vars.resize( idx + 1 ) ;

    for( size_t i=0; i<old.size(); ++i ) _vars[i] = old[i] ;
    for( size_t i=old.size(); i < _vars.size(); ++i ) _vars[i] = 
        motor::shared( motor::graphics::variable_set_t() ) ;

    return *this ;
}

//****************************************************************************
motor::graphics::variable_set_mtr_safe_t msl_object::get_varibale_set( size_t const id ) noexcept 
{
    this_t::fill_variable_sets( id ) ;
    return motor::share( _vars[ id ] ) ;
}

//****************************************************************************
motor::graphics::variable_set_mtr_t msl_object::borrow_varibale_set( size_t const id ) const noexcept 
{
    if ( id >= _vars.size() ) return nullptr ;
    return motor::share_unsafe( _vars[ id ] ) ;
}

//****************************************************************************
motor::vector< motor::graphics::variable_set_mtr_safe_t > msl_object::get_varibale_sets( void_t ) const noexcept
{
    motor::vector< motor::graphics::variable_set_mtr_safe_t > ret ;

    for ( auto * mtr : _vars ) ret.emplace_back( motor::share( mtr ) ) ;

    return ret ;
}

//****************************************************************************
motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > & msl_object::borrow_varibale_sets( void_t ) noexcept
{
    return _vars ;
}

//****************************************************************************
motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > const & msl_object::borrow_varibale_sets( void_t ) const noexcept 
{
    return _vars ;
}

//****************************************************************************
void_t msl_object::for_each( for_each_var_funk_t funk ) noexcept
{
    size_t i = 0 ;
    for ( auto const & v : _vars )
    {
        funk( i++, v ) ;
    }
}

//****************************************************************************
void_t msl_object::register_listener( motor::graphics::compilation_listener_mtr_safe_t lst ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_compilation_listeners ) ;

    auto iter = std::find_if( _compilation_listeners.begin(), _compilation_listeners.end(),
        [&] ( motor::graphics::compilation_listener_mtr_t l )
    {
        return l == lst ;
    } ) ;

    if ( iter != _compilation_listeners.end() )
    {
        motor::release( lst ) ;
        return ;
    }

    _compilation_listeners.push_back( motor::move( lst ) ) ;
}

//****************************************************************************
void_t msl_object::unregister_listener( motor::graphics::compilation_listener_mtr_t lst ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_compilation_listeners ) ;

    auto iter = std::find_if( _compilation_listeners.begin(), _compilation_listeners.end(),
        [&] ( motor::graphics::compilation_listener_mtr_t l )
    {
        return l == lst ;
    } ) ;

    if ( iter == _compilation_listeners.end() ) return ;

    motor::release( motor::move( *iter ) ) ;
    _compilation_listeners.erase( iter ) ;
}

//****************************************************************************
void_t msl_object::for_each( for_each_change_listerner_funk_t f ) noexcept
{
    motor::concurrent::mrsw_t::reader_lock lk( _mtx_compilation_listeners ) ;
    for ( auto * lst : _compilation_listeners ) f( lst ) ;
}

//****************************************************************************
motor::graphics::compilation_listener_mtr_t msl_object::borrow_compilation_listener( void_t ) const noexcept
{
    return _comp_lst ;
}

//****************************************************************************
bool_t msl_object::has_shader_changed( void_t ) const noexcept
{
    return _comp_lst->has_changed() ;
}

//****************************************************************************
bool_t msl_object::get_if_successful( motor::graphics::shader_bindings_out_t sb ) noexcept 
{
    return _comp_lst->get_if_successful( sb ) ;
}

//****************************************************************************
bool_t msl_object::reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
{
    return _comp_lst->reset_and_successful( sb ) ;
}