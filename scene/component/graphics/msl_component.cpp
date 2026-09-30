
#include "msl_component.h"

using namespace motor::scene;

//*****************************************************************
msl_component::msl_component( this_rref_t rhv ) noexcept
    : _vs( rhv._vs ), _geo_id( rhv._geo_id ), _trafo_vars( std::move( rhv._trafo_vars ) ),
      _base_data_set( std::move( rhv._base_data_set ) ), _sub_sets( std::move( rhv._sub_sets ) )
{
#if 0
    std::memcpy( reinterpret_cast< void * >( &_cam_vars ),
        reinterpret_cast< void * >( &rhv._cam_vars ), sizeof( _cam_vars ) );

    std::memcpy( reinterpret_cast< void * >( &_light_vars ),
        reinterpret_cast< void * >( &rhv._light_vars ), sizeof( _light_vars ) );    

    motor::release( motor::move( _var_set ) );
    _var_set = motor::move( rhv._var_set );
#endif

    motor::release( motor::move( _msl ) );
    _msl = motor::move( rhv._msl );

    motor::release( motor::move( _comp_lst ) );
    _comp_lst = motor::move( rhv._comp_lst );
}

//*****************************************************************
msl_component::msl_component( motor::graphics::msl_object_mtr_safe_t msl ) noexcept
    : _msl( motor::move( msl ) )
{
#if 0
    std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
    std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
#endif

    if( _msl != nullptr ) _msl->register_listener( motor::share( _comp_lst ) );
}

//*****************************************************************
msl_component::msl_component(
    motor::graphics::msl_object_mtr_safe_t msl, vs_idx_t const vs, geo_idx_t const geo_id ) noexcept
    : _msl( motor::move( msl ) ), _vs( vs ), _geo_id( geo_id )
{
#if 0
    std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
    std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
#endif

    if( _msl != nullptr )
    {
        _msl->register_listener( motor::share( _comp_lst ) );
        _msl->fill_variable_sets( vs );
    }
}

//*****************************************************************
msl_component::~msl_component( void_t ) noexcept
{
#if 0
    motor::release( motor::move( _var_set ) );
#endif
    motor::release( motor::move( _msl ) );
    motor::release( motor::move( _comp_lst ) );
}

//*****************************************************************
size_t msl_component::set_msl( motor::graphics::msl_object_mtr_safe_t msl ) noexcept
{
#if 0
    std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
    std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
#endif
    _base_data_set.clear();

    if( _msl != nullptr ) motor::release( motor::move( _msl ) );

    _msl = motor::move( msl );
    _vs = size_t( -1 );
    if( _msl != nullptr )
    {
        _msl->register_listener( motor::share( _comp_lst ) );
        _vs = _msl->borrow_varibale_sets().size();
        _msl->fill_variable_sets( _vs );
    }

    return _vs;
}

//*****************************************************************
bool_t msl_component::render_init( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    auto res = fe->decode( _msl );
    if( res.first == motor::graphics::object_state::ready )
    {
        return true;
    }
    else if( ( res.first == motor::graphics::object_state::raw ||
                 res.first == motor::graphics::object_state::invalid ) &&
             !_msl->is_managed() )
    {

        fe->configure< motor::graphics::msl_object_t >( _msl );
    }

    return false;
}

//*****************************************************************
bool_t msl_component::render_release( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    auto res = fe->decode( _msl );

    if( res.first == motor::graphics::object_state::ready && !_msl->is_managed() )
    {
        fe->release< motor::graphics::msl_object_t >( _msl );
    }
    else if( res.first == motor::graphics::object_state::in_transit )
    {
        return false;
    }

    return true;
}

//*****************************************************************
void_t msl_component::render_update( motor::gfx::generic_camera_ptr_t cam ) noexcept
{
    this_t::update_bindings();
    this_t::update_camera( cam );
}

//*****************************************************************
void_t msl_component::render_update(
    size_t const render_id, motor::gfx::generic_camera_ptr_t cam ) noexcept
{
    this_t::ensure_render_data( render_id + 1 );

    this_t::update_bindings( render_id );
    this_t::update_camera( render_id, cam );
}

//*****************************************************************
void_t msl_component::update_bindings( size_t const render_id ) noexcept
{    
    // #1 : update base set
    // @todo called too often if muliple render ids are used
    this_t::update_bindings();

    // #2 : update render data set based on id
    // pull data from base render data set.    
}

//*****************************************************************
void_t msl_component::update_bindings( void_t ) noexcept
{
    if( _comp_lst->has_changed() )
    {
        motor::graphics::shader_bindings_t sb;
        if( _comp_lst->reset_and_successful( sb ) )
        {
            {
                for( auto & rd : _sub_sets )
                {
                    _msl->drop_variable_set( rd.get_render_data_set_idx() );
                    rd.clear();
                }
            }

            {
                _trafo_vars.world->disconnect();
            }

            // init base data set
            {
                auto var_set = _msl->get_varibale_set( _vs );
                _base_data_set.update_bindings( _vs, motor::move( var_set ) , sb );

                // init world transformation slot
                {
                    motor::string_t name;

                    if( sb.has_variable_binding(
                            motor::graphics::binding_point::world_matrix, name ) )
                    {
                        _trafo_vars.world->connect( motor::share(
                            _base_data_set.variable_bridge().borrow_inputs()->borrow_or_add(
                                name, motor::shared(
                                          motor::wire::input_slot< motor::math::mat4f_t >() ) ) ) );
                    }
                }
            }
        }
    }

    _base_data_set.variable_bridge().pull_data();
}

//*****************************************************************
void_t msl_component::update_camera( motor::gfx::generic_camera_ptr_t cam ) noexcept
{
    _base_data_set.update_camera( cam );
}

//*****************************************************************
void_t msl_component::update_camera(
    size_t const render_id, motor::gfx::generic_camera_ptr_t cam ) noexcept
{
    this_t::ensure_render_data( render_id );
    _sub_sets[ render_id ].update_camera( cam );
}

//*****************************************************************
void_t msl_component::set_world( motor::math::m3d::trafof_cref_t trafo ) noexcept
{
    _trafo_vars.world->set_and_exchange( trafo.get_transformation() );
}

//*****************************************************************
void_t msl_component::set_light_direction( motor::math::vec3f_cref_t dir ) noexcept
{
    _base_data_set.set_light_direction( dir );
}

//*****************************************************************
void_t msl_component::set_light_projection( motor::math::mat4f_cref_t mat ) noexcept
{
    _base_data_set.set_light_projection( mat );
}

//*****************************************************************
void_t msl_component::set_light_view( motor::math::mat4f_cref_t mat ) noexcept
{
    _base_data_set.set_light_view( mat );
}

//*****************************************************************
void_t msl_component::set_light_shadow_map( motor::string_cref_t name ) noexcept
{
    _base_data_set.set_light_shadow_map( name );
}

//*****************************************************************
motor::wire::inputs_cptr_t msl_component::borrow_shader_inputs( void_t ) const noexcept
{
    return _base_data_set.variable_bridge().borrow_inputs();
}

//*****************************************************************
motor::wire::inputs_ptr_t msl_component::borrow_shader_inputs( void_t ) noexcept
{
    return _base_data_set.variable_bridge().borrow_inputs();
}

//*****************************************************************
void_t msl_component::ensure_render_data( size_t const id ) noexcept
{
    if( _sub_sets.size() <= id )
    {
        _sub_sets.resize( id + 1 );
    }
}