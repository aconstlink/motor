#include "render_node.h"

using namespace motor::scene ;

motor_core_dd_id_init( render_node ) ;

//*****************************************************************
render_node::render_node( this_rref_t rhv ) noexcept : 
    base_t( std::move( rhv) ), _msl( motor::move( rhv._msl ) ), _vs( rhv._vs ), _var_set( motor::move( rhv._var_set ) )
{
    std::memcpy( reinterpret_cast<void*>( &_cam_vars ), 
        reinterpret_cast<void*>( &rhv._cam_vars ), 
        sizeof( _cam_vars ) ) ;
}

//*****************************************************************
render_node::render_node( motor::graphics::msl_object_mtr_safe_t msl ) noexcept : 
    _msl( motor::move( msl ) )
{
    std::memset( reinterpret_cast<void*>( &_cam_vars ), 0, sizeof( _cam_vars ) ) ;
}

//*****************************************************************
render_node::render_node( motor::graphics::msl_object_mtr_safe_t msl, size_t const vs ) noexcept : 
    _msl( motor::move(msl) ), _vs(vs)
{
    std::memset( reinterpret_cast<void*>( &_cam_vars ), 0, sizeof( _cam_vars ) ) ;
}

//*****************************************************************
render_node::~render_node( void_t ) noexcept
{
    motor::memory::release_ptr( _msl ) ;
    motor::release( motor::move( _var_set ) ) ;
}

//*****************************************************************
void_t render_node::update_bindings( void_t ) noexcept 
{
    if( _msl->has_shader_changed() )
    {
        motor::graphics::shader_bindings_t sb ;
        if ( _msl->get_if_successful( sb ) )
        {
            motor::release( motor::move( _var_set) ) ;
            _var_set = _msl->get_varibale_set( _vs ) ;

            motor::string_t name ;
            {
                if( sb.has_variable_binding( motor::graphics::binding_point::projection_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name ) ;
                    if( var != nullptr )
                    {
                        _cam_vars.proj = var ;
                    }
                }

                if ( sb.has_variable_binding( motor::graphics::binding_point::camera_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name ) ;
                    if ( var != nullptr )
                    {
                        _cam_vars.cam = var ;
                    }
                }

                if ( sb.has_variable_binding( motor::graphics::binding_point::view_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name ) ;
                    if ( var != nullptr )
                    {
                        _cam_vars.view = var ;
                    }
                }

                if ( sb.has_variable_binding( motor::graphics::binding_point::camera_position, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::vec3f_t >( name ) ;
                    if ( var != nullptr )
                    {
                        _cam_vars.cam_pos = var ;
                    }
                }
                
            }
        }
    }
}

//*****************************************************************
void_t render_node::update_camera( motor::gfx::generic_camera_ptr_t cam ) noexcept 
{
    if( _cam_vars.proj != nullptr )
    {
        _cam_vars.proj->set( cam->get_proj_matrix() ) ;
    }

    if ( _cam_vars.view != nullptr )
    {
        _cam_vars.view->set( cam->get_view_matrix() ) ;
    }

    if ( _cam_vars.cam != nullptr )
    {
        _cam_vars.cam->set( cam->get_camera_matrix() ) ;
    }

    if ( _cam_vars.cam_pos != nullptr )
    {
        _cam_vars.cam_pos->set( cam->get_position() ) ;
    }
}