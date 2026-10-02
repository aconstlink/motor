#pragma once

#include "../../api.h"
#include "../../typedefs.h"

#include <motor/gfx/camera/generic_camera.h>
#include <motor/gfx/util/light.hpp>

#include <motor/graphics/variable/wire_variable_input_bridge.hpp>
#include <motor/graphics/shader/shader_bindings.hpp>
#include <motor/graphics/variable/variable_set.hpp>
#include <motor/math/utility/3d/transformation.hpp>

namespace motor
{
namespace scene
{
class render_data_set
{
    motor_this_typedefs( render_data_set );

  public:

    using vs_idx_t = size_t;

  private:

    vs_idx_t _vs_idx = vs_idx_t( -1 );
    motor::graphics::variable_set_mtr_t _var_set = nullptr;

  private: // bindings

    struct camera_variables
    {
        motor::graphics::data_variable< motor::math::mat4f_t > * proj;
        motor::graphics::data_variable< motor::math::mat4f_t > * cam;
        motor::graphics::data_variable< motor::math::mat4f_t > * view;
        motor::graphics::data_variable< motor::math::vec3f_t > * cam_pos;

        void_t clear( void_t ) noexcept
        {
            proj = nullptr;
            cam = nullptr;
            view = nullptr;
            cam_pos = nullptr;
        }
    };

    camera_variables _cam_vars;

    struct light_variables
    {
        motor::graphics::data_variable< motor::math::vec3f_t > * light_dir;
        motor::graphics::data_variable< motor::math::mat4f_t > * light_proj;
        motor::graphics::data_variable< motor::math::mat4f_t > * light_view;
        motor::graphics::texture_variable_t * light_shadow_map;

        void_t clear( void_t ) noexcept
        {
            light_dir = nullptr;
            light_proj = nullptr;
            light_view = nullptr;
            light_shadow_map = nullptr;
        }
    };

    light_variables _light_vars;

    struct trafo_variables
    {
        motor::graphics::data_variable< motor::math::mat4f_t > * world;

        void_t clear( void_t ) noexcept
        {
            world = nullptr;
        }
    };

    trafo_variables _trafo_vars;

    // allows to connect to shader variables via input slots.
    motor::graphics::wire_variable_input_bridge_t _bridge;

  public:

    render_data_set( void_t ) noexcept
    {
        std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
        std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
        std::memset( reinterpret_cast< void * >( &_trafo_vars ), 0, sizeof( _trafo_vars ) );
    }

    render_data_set(
        this_t::vs_idx_t const id, motor::graphics::variable_set_mtr_safe_t vs ) noexcept
        : _vs_idx( id ), _var_set( motor::move( vs ) )
    {
        std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
        std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
        std::memset( reinterpret_cast< void * >( &_trafo_vars ), 0, sizeof( _trafo_vars ) );

        _bridge.update_bindings( motor::share( _var_set ) );
    }

    render_data_set( this_rref_t rhv ) noexcept
        : _vs_idx( rhv._vs_idx ), _bridge( std::move( rhv._bridge ) )
    {
        motor::release( motor::move( _var_set ) );
        _var_set = motor::move( rhv._var_set );

        std::memcpy( reinterpret_cast< void * >( &_cam_vars ),
            reinterpret_cast< void * >( &rhv._cam_vars ), sizeof( _cam_vars ) );

        std::memcpy( reinterpret_cast< void * >( &_light_vars ),
            reinterpret_cast< void * >( &rhv._light_vars ), sizeof( _light_vars ) );

        std::memcpy( reinterpret_cast< void * >( &_trafo_vars ),
            reinterpret_cast< void * >( &rhv._trafo_vars ), sizeof( _trafo_vars ) );
    }
    ~render_data_set( void_t ) noexcept
    {
        motor::release( motor::move( _var_set ) );
    }

    vs_idx_t get_variable_set_idx( void_t ) const noexcept
    {
        return _vs_idx;
    }

  public:

  public: // light interface

    // set a light direction on the shader variable
    // if there is a bindings.
    void_t set_light_direction( motor::math::vec3f_cref_t dir ) noexcept
    {
        if( _light_vars.light_dir )
        {
            _light_vars.light_dir->set( dir );
        }
    }

    void_t set_light_projection( motor::math::mat4f_cref_t mat ) noexcept
    {
        if( _light_vars.light_proj )
        {
            _light_vars.light_proj->set( mat );
        }
    }
    void_t set_light_view( motor::math::mat4f_cref_t mat ) noexcept
    {
        if( _light_vars.light_view )
        {
            _light_vars.light_view->set( mat );
        }
    }
    void_t set_light_shadow_map( motor::string_cref_t name ) noexcept
    {
        if( _light_vars.light_shadow_map )
        {
            _light_vars.light_shadow_map->set( name );
        }
    }

    //*****************************************************************
    void_t update_camera( motor::gfx::generic_camera_ptr_t cam ) noexcept
    {
        if( _cam_vars.proj != nullptr )
        {
            _cam_vars.proj->set( cam->get_proj_matrix() );
        }

        if( _cam_vars.view != nullptr )
        {
            _cam_vars.view->set( cam->get_view_matrix() );
        }

        if( _cam_vars.cam != nullptr )
        {
            _cam_vars.cam->set( cam->get_camera_matrix() );
        }

        if( _cam_vars.cam_pos != nullptr )
        {
            _cam_vars.cam_pos->set( cam->get_position() );
        }
    }

    //*****************************************************************
    void_t update_bindings( this_t::vs_idx_t const idx, motor::graphics::variable_set_mtr_safe_t vs,
        motor::graphics::shader_bindings_cref_t sb ) noexcept
    {
        // #1 reset direct shader bindings
        {
            std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
            std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
            std::memset( reinterpret_cast< void * >( &_trafo_vars ), 0, sizeof( _trafo_vars ) );
        }

        // #2
        {
            _vs_idx = idx;
            motor::release( motor::move( _var_set ) );
            _var_set = motor::move( vs );
        }

        // update shader variable bindings
        {
            motor::string_t name;
            {
                if( sb.has_variable_binding(
                        motor::graphics::binding_point::projection_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name );
                    if( var != nullptr )
                    {
                        _cam_vars.proj = var;
                    }
                }

                if( sb.has_variable_binding( motor::graphics::binding_point::camera_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name );
                    if( var != nullptr )
                    {
                        _cam_vars.cam = var;
                    }
                }

                if( sb.has_variable_binding( motor::graphics::binding_point::view_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name );
                    if( var != nullptr )
                    {
                        _cam_vars.view = var;
                    }
                }

                if( sb.has_variable_binding(
                        motor::graphics::binding_point::camera_position, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::vec3f_t >( name );
                    if( var != nullptr )
                    {
                        _cam_vars.cam_pos = var;
                    }
                }

                if( sb.has_variable_binding(
                        motor::graphics::binding_point::light_direction, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::vec3f_t >( name );
                    if( var != nullptr )
                    {
                        _light_vars.light_dir = var;
                    }
                }

                if( sb.has_variable_binding(
                        motor::graphics::binding_point::light_projection, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name );
                    if( var != nullptr )
                    {
                        _light_vars.light_proj = var;
                    }
                }

                if( sb.has_variable_binding( motor::graphics::binding_point::light_view, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name );
                    if( var != nullptr )
                    {
                        _light_vars.light_view = var;
                    }
                }

                if( sb.has_variable_binding(
                        motor::graphics::binding_point::light_shadow_map, name ) )
                {
                    auto * var = _var_set->texture_variable( name );
                    if( var != nullptr )
                    {
                        _light_vars.light_shadow_map = var;
                    }
                }

                if( sb.has_variable_binding( motor::graphics::binding_point::world_matrix, name ) )
                {
                    auto * var = _var_set->data_variable< motor::math::mat4f_t >( name );
                    if( var != nullptr )
                    {
                        _trafo_vars.world = var;
                    }
                }
            }
        }

        _bridge.update_bindings( motor::share( _var_set ) );
    }

    motor::graphics::wire_variable_input_bridge_ref_t variable_bridge( void_t ) noexcept
    {
        return _bridge;
    }

    motor::graphics::wire_variable_input_bridge_cref_t variable_bridge( void_t ) const noexcept
    {
        return _bridge;
    }

    void_t set_world( motor::math::m3d::trafof_cref_t trafo ) noexcept
    {
        if( _trafo_vars.world ) _trafo_vars.world->set( trafo.get_transformation() );
    }

    void_t clear( void_t ) noexcept
    {
        std::memset( reinterpret_cast< void * >( &_cam_vars ), 0, sizeof( _cam_vars ) );
        std::memset( reinterpret_cast< void * >( &_light_vars ), 0, sizeof( _light_vars ) );
        std::memset( reinterpret_cast< void * >( &_trafo_vars ), 0, sizeof( _trafo_vars ) );

        motor::release( motor::move( _var_set ) );
        _vs_idx = this_t::vs_idx_t( -1 );

        _bridge.clear();
    }

    void_t set_varset(
        this_t::vs_idx_t const idx, motor::graphics::variable_set_mtr_safe_t vs ) noexcept
    {
        _vs_idx = idx;

        motor::release( motor::move( _var_set ) );
        _var_set = motor::move( vs );
    }
};
motor_typedef( render_data_set );

} // namespace scene
} // namespace motor