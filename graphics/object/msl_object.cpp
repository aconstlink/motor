

#include "msl_object.h"

using namespace motor::graphics;

//****************************************************************************
msl_object::msl_object( void_t ) noexcept
{
    _ro = motor::shared( motor::graphics::render_object() );
    this_t::register_listener( motor::share( _comp_lst ) );
}

//****************************************************************************
msl_object::msl_object( motor::string_in_t name ) noexcept : _name( name )
{
    _ro = motor::shared( motor::graphics::render_object( name + ".render_object" ) );
    this_t::register_listener( motor::share( _comp_lst ) );
}

//****************************************************************************
msl_object::msl_object( motor::string_in_t name, bool_t const managed ) noexcept
    : object( managed ), _name( name )
{
    _ro = motor::shared( motor::graphics::render_object( name + ".render_object" ) );
    this_t::register_listener( motor::share( _comp_lst ) );
}

//****************************************************************************
msl_object::msl_object( this_rref_t rhv ) noexcept
    : object( std::move( rhv ) ), _name( std::move( rhv._name ) ),
      _datas( std::move( rhv._datas ) ), _ro( motor::move( rhv._ro ) )
{
    // compilation listeners
    {
        motor::release( motor::move( _comp_lst ) );
        _comp_lst = motor::move( rhv._comp_lst );

        for( auto * l : _compilation_listeners ) motor::memory::release_ptr( l );
        _compilation_listeners = std::move( rhv._compilation_listeners );
    }
}

//****************************************************************************
msl_object::this_ref_t msl_object::operator=( this_rref_t rhv ) noexcept
{
    object::operator=( std::move( rhv ) );

    _name = std::move( rhv._name );
    _datas = std::move( rhv._datas );
    _ro = motor::move( rhv._ro );

    // compilation listeners
    {
        motor::release( motor::move( _comp_lst ) );
        _comp_lst = motor::move( rhv._comp_lst );

        for( auto * l : _compilation_listeners ) motor::memory::release_ptr( l );
        _compilation_listeners = std::move( rhv._compilation_listeners );
    }

    return *this;
}
#if 0
//****************************************************************************
msl_object::msl_object( this_cref_t rhv ) noexcept
    : object( rhv ), _name( rhv._name ), _datas( rhv._datas ), _ro( rhv._ro )
{
    // compilation listeners
    {
        if( _comp_lst != rhv._comp_lst )
        {
            motor::release( motor::move( _comp_lst ) );
            _comp_lst = motor::share( rhv._comp_lst );
        }

        for( auto * l : _compilation_listeners ) motor::memory::release_ptr( l );

        _compilation_listeners.resize( rhv._compilation_listeners.size() );
        for( size_t i = 0; i < rhv._compilation_listeners.size(); ++i )
            _compilation_listeners[ i ] = motor::share( rhv._compilation_listeners[ i ] );
    }
}

//****************************************************************************
msl_object::this_ref_t msl_object::operator=( this_cref_t rhv ) noexcept
{
    object::operator=( rhv );

    _name = rhv._name;
    _datas = rhv._datas;
    _ro = rhv._ro;

    // compilation listeners
    {
        if( _comp_lst != rhv._comp_lst )
        {
            motor::release( motor::move( _comp_lst ) );
            _comp_lst = motor::share( rhv._comp_lst );
        }

        for( auto * l : _compilation_listeners ) motor::memory::release_ptr( l );

        _compilation_listeners.resize( rhv._compilation_listeners.size() );
        for( size_t i = 0; i < rhv._compilation_listeners.size(); ++i )
            _compilation_listeners[ i ] = motor::share( rhv._compilation_listeners[ i ] );
    }

    return *this;
}
#endif

//****************************************************************************
msl_object::~msl_object( void_t ) noexcept
{
    for( auto * lst : _compilation_listeners ) motor::release( motor::move( lst ) );
    motor::release( motor::move( _comp_lst ) );

    motor::release( motor::move( _ro ) );
}

//****************************************************************************
motor::string_cref_t msl_object::name( void_t ) const noexcept
{
    return _name;
}

//****************************************************************************
msl_object::this_ref_t msl_object::add(
    motor::graphics::msl_api_type const t, motor::string_in_t msl_code ) noexcept
{
    _datas.emplace_back( this_t::data{ t, msl_code } );
    return *this;
}

//****************************************************************************
msl_object::this_ref_t msl_object::clear_shaders( void_t ) noexcept
{
    _datas.clear();
    return *this;
}

//****************************************************************************
void_t msl_object::for_each_msl(
    motor::graphics::msl_api_type const t, foreach_funk_t funk ) const noexcept
{
    for( auto & d : _datas )
    {
        if( d.t == t ) funk( d.code );
    }
}

//****************************************************************************
size_t msl_object::link_geometry( motor::string_cref_t name ) noexcept
{
    return _ro->link_geometry( name );
}


bool_t msl_object::unlink_geometry( motor::string_cref_t name ) noexcept
{
    return _ro->unlink_geometry( name ) ;
}

//****************************************************************************
bool_t msl_object::unlink_geometry( size_t const geo_idx ) noexcept
{
    return _ro->unlink_geometry( geo_idx ) ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::link_geometry(
    std::initializer_list< motor::string_t > const & names ) noexcept
{
    _ro->link_geometry( names );

    return *this;
}

//****************************************************************************
msl_object::this_ref_t msl_object::link_geometry(
    motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept
{
    _ro->link_geometry( name, soo_name );
    return *this;
}

//****************************************************************************
void_t msl_object::for_each_geometry_link( for_each_geo_link_funk_t funk ) const noexcept
{
    _ro->for_each_geometry_link( funk );
}

//****************************************************************************
motor::graphics::render_object_t::geometry_link_cref_t msl_object::get_geo_link(
    size_t const idx ) const noexcept
{
    return _ro->get_geometry_link( idx );
}

//****************************************************************************
size_t msl_object::get_num_geo_links( void_t ) const noexcept
{
    return _ro->get_num_geometry();
}

//****************************************************************************
motor::vector< motor::string_t > const & msl_object::get_streamout( void_t ) const noexcept
{
    return _ro->get_streamouts();
}
#if 0
//****************************************************************************
msl_object::this_t msl_object::light_clone( motor::string_in_t name ) const noexcept
{
    this_t ret;

    ret._name = name;
    ret._geo = this_t::_geo;
    ret._soo = this_t::_soo;
    ret._datas = this_t::_datas;

    return std::move( ret );
}
#endif
//****************************************************************************
size_t msl_object::add_variable_set( motor::graphics::variable_set_mtr_safe_t vs ) noexcept
{
    return _ro->add_variable_set( motor::move( vs ) );
}

//****************************************************************************
void_t msl_object::drop_variable_set( size_t const idx ) noexcept 
{
    _ro->drop_variable_set( idx ) ;
}

//****************************************************************************
msl_object::this_ref_t msl_object::fill_variable_sets( size_t const idx ) noexcept
{
    _ro->fill_variable_sets( idx );
    return *this;
}

//****************************************************************************
motor::graphics::variable_set_mtr_safe_t msl_object::get_varibale_set( size_t const id ) noexcept
{
    return _ro->get_variable_set( id );
}

//****************************************************************************
motor::graphics::variable_set_mtr_t msl_object::borrow_varibale_set(
    size_t const id ) const noexcept
{
    return _ro->borrow_variable_set( id );
}

//****************************************************************************
motor::vector< motor::graphics::variable_set_mtr_safe_t > msl_object::get_varibale_sets(
    void_t ) const noexcept
{
    return _ro->get_varibale_sets();
}

//****************************************************************************
motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > & msl_object::borrow_varibale_sets(
    void_t ) noexcept
{
    return _ro->borrow_varibale_sets();
}

//****************************************************************************
motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > const &
msl_object::borrow_varibale_sets( void_t ) const noexcept
{
    return _ro->borrow_varibale_sets();
}

//****************************************************************************
void_t msl_object::for_each( for_each_var_funk_t funk ) noexcept
{
    _ro->for_each( funk );
}

//****************************************************************************
void_t msl_object::register_listener(
    motor::graphics::compilation_listener_mtr_safe_t lst ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_compilation_listeners );

    auto iter = std::find_if( _compilation_listeners.begin(), _compilation_listeners.end(),
        [ & ]( motor::graphics::compilation_listener_mtr_t l ) { return l == lst; } );

    if( iter != _compilation_listeners.end() )
    {
        motor::release( lst );
        return;
    }

    // in case the shader was compiled before the the listener
    // is attached, the listener can be used to rewire the
    // attached logic.
    lst->copy_from( *_comp_lst );

    _compilation_listeners.push_back( motor::move( lst ) );
}

//****************************************************************************
void_t msl_object::unregister_listener( motor::graphics::compilation_listener_mtr_t lst ) noexcept
{
    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx_compilation_listeners );

    auto iter = std::find_if( _compilation_listeners.begin(), _compilation_listeners.end(),
        [ & ]( motor::graphics::compilation_listener_mtr_t l ) { return l == lst; } );

    if( iter == _compilation_listeners.end() ) return;

    motor::release( motor::move( *iter ) );
    _compilation_listeners.erase( iter );
}

//****************************************************************************
void_t msl_object::for_each( for_each_change_listerner_funk_t f ) noexcept
{
    motor::concurrent::mrsw_t::reader_lock lk( _mtx_compilation_listeners );
    f( _comp_lst );
    for( auto * lst : _compilation_listeners ) f( lst );
}

//****************************************************************************
motor::graphics::compilation_listener_mtr_t msl_object::borrow_compilation_listener(
    void_t ) const noexcept
{
    return _comp_lst;
}

//****************************************************************************
bool_t msl_object::has_shader_changed( void_t ) const noexcept
{
    return _comp_lst->has_changed();
}

//****************************************************************************
bool_t msl_object::get_if_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
{
    return _comp_lst->get_if_successful( sb );
}

//****************************************************************************
bool_t msl_object::reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept
{
    return _comp_lst->reset_and_successful( sb );
}

//****************************************************************************
motor::graphics::render_object_mtr_t msl_object::borrow_render_object( void_t ) noexcept 
{
    return _ro ;
}

//****************************************************************************
motor::graphics::render_object_mtr_safe_t msl_object::get_render_object( void_t ) noexcept 
{
    return motor::share( _ro ) ;
}