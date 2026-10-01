#pragma once
#include "../api.h"
#include "../typedefs.h"

#include "variable_set.hpp"

#include <motor/wire/slot/sheet.hpp>

namespace motor
{
namespace graphics
{
class wire_variable_output_bridge
{
    motor_this_typedefs( wire_variable_output_bridge );

  private:

    motor::graphics::variable_set_mtr_t _vs = nullptr;
    motor::wire::outputs_t _sheet;

    // will only hold borrowed pointers.
    // ref counted pointers are stored above.
    // binding in this case means:
    // shader var to slot binding
    struct variable_binding
    {
        using pull_funk_t = std::function< void_t( variable_binding & ) >;
        motor::graphics::ivariable_ptr_t gvar;
        motor::wire::ioutput_slot_ptr_t slot;
        pull_funk_t pull_funk;
    };

    motor::vector< variable_binding > _bindings;

  public:

    wire_variable_output_bridge( void_t ) noexcept {}
    wire_variable_output_bridge( motor::graphics::variable_set_mtr_safe_t vs ) noexcept
        : _vs( motor::move( vs ) )
    {
        this_t::update_bindings() ;
    }
    wire_variable_output_bridge( this_rref_t rhv ) noexcept
    {
        motor::release( motor::move( _vs ) );
        _vs = motor::move( rhv._vs );

        _sheet = std::move( rhv._sheet );

        this_t::clear_bindings();
        _bindings = std::move( rhv._bindings );
    }
    wire_variable_output_bridge( this_cref_t ) = delete;
    ~wire_variable_output_bridge( void_t ) noexcept
    {
        this_t::clear();
    }

    this_ref_t operator=( this_rref_t rhv ) noexcept
    {
        motor::release( motor::move( _vs ) );
        _vs = motor::move( rhv._vs );

        _sheet = std::move( rhv._sheet );

        this_t::clear_bindings();
        _bindings = std::move( rhv._bindings );

        return *this;
    }

  public: // update interface

    // pull data from graphics variables to output slots
    void_t pull_data( void_t ) noexcept
    {
        for( auto & b : _bindings )
        {
            b.pull_funk( b );
        }
    }

    // exchange all pulled values with the connected
    // input slots. Requires a pull_data before.
    void_t exchange( void_t ) noexcept
    {
        for( auto & b : _bindings )
        {
            b.slot->exchange() ;
        }
    }

    // pull and exchange in one function.
    void_t pull_and_exchange( void_t ) 
    {
        for( auto & b : _bindings )
        {
            b.pull_funk( b );
            b.slot->exchange() ;
        }
    }

    public: // bindings

    // redo graphics variables to output bindings
    void_t update_bindings( void_t ) noexcept
    {
        this_t::create_bindings();

        if( _vs == nullptr ) return;
    }

    // redo graphics variables to output bindings
    // by introducing a new variable set
    void_t update_bindings( motor::graphics::variable_set_mtr_safe_t vs ) noexcept
    {
        motor::release( motor::move( _vs ) );
        _vs = motor::move( vs );

        this_t::update_bindings();
    }

    void_t clear( void_t ) noexcept
    {
        this_t::clear_bindings();
        _sheet.clear();
        motor::release( motor::move( _vs ) );
    }

  public: // get/set

    motor::wire::outputs_ref_t borrow_outputs( void_t ) noexcept
    {
        return _sheet;
    }
    motor::wire::outputs_cref_t borrow_outputs( void_t ) const noexcept
    {
        return _sheet;
    }

  private:

    void_t create_bindings( void_t ) noexcept
    {
        this_t::clear_bindings();

        if( _vs == nullptr )
        {
            _sheet.clear();
            return;
        }

        // for each variable in the graphics variable set, make a binding
        {
            _vs->for_each_data_variable(
                [ & ]( motor::string_in_t name, motor::graphics::ivariable_ptr_t var )
            {
                if( this_t::make_binding< int_t >( name, var ) ) return;
                if( this_t::make_binding< float_t >( name, var ) ) return;
                if( this_t::make_binding< uint_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec2f_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec3f_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec4f_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec2i_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec3i_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec4i_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec2ui_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec3ui_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::vec4ui_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::mat2f_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::mat3f_t >( name, var ) ) return;
                if( this_t::make_binding< motor::math::mat4f_t >( name, var ) ) return;
            } );

            _vs->for_each_texture_variable(
                [ & ]( motor::string_in_t name,
                    motor::graphics::data_variable< motor::string_t > * var )
            {
                if( this_t::make_binding< motor::graphics::texture_variable_data >( name, var ) )
                    return;
            } );
        }

        #if 0
        // for each variable in the slot sheet, make a binding
        {
            _sheet.for_each_slot(
                [ & ]( motor::string_in_t name, motor::wire::ioutput_slot_mtr_t s )
            {
                if( this_t::make_binding< int_t >( name, s ) ) return;
                if( this_t::make_binding< float_t >( name, s ) ) return;
                if( this_t::make_binding< uint_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec2f_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec3f_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec4f_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec2i_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec3i_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec4i_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec2ui_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec3ui_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::vec4ui_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::mat2f_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::mat3f_t >( name, s ) ) return;
                if( this_t::make_binding< motor::math::mat4f_t >( name, s ) ) return;
                if( this_t::make_binding< motor::graphics::texture_variable_data >( name, s ) )
                    return;
                if( this_t::make_binding< motor::graphics::array_variable_data >( name, s ) )
                    return;
                if( this_t::make_binding< motor::graphics::streamout_variable_data >( name, s ) )
                    return;
            } );
        }
        #endif
    }
    void_t clear_bindings( void_t ) noexcept
    {
        for( auto & b : _bindings )
        {
            motor::release( motor::move( b.slot ) );
        }
        _bindings.clear();
    }

    // #1 : this function creates output slots from shader variables
    template < typename T >
    bool_t make_binding( motor::string_in_t name, motor::graphics::ivariable_ptr_t var ) noexcept
    {
        using type_t = T;

        auto [ a, b ] = motor::graphics::cast_data_variable< type_t >( var );
        if( a )
        {
            using slot_t = motor::wire::output_slot< type_t >;
            using var_t = motor::graphics::data_variable< type_t >;

            auto s = _sheet.get_or_add( name, motor::shared( slot_t( b->get() ) ) );
            ((slot_t *)s.mtr())->set_value( b->get() ) ;

            _bindings.emplace_back(
                variable_binding{ var, motor::move( s ), [ = ]( this_t::variable_binding & v )
            {
                auto * s_local = reinterpret_cast< slot_t * >( v.slot );
                auto * v_local = reinterpret_cast< var_t * >( v.gvar );

                s_local->set_value( v_local->get() );
            } } );
            return true;
        }
        return false;
    }

    #if 0
    // #2 : this function creates shader variables from output slots
    template < typename T >
    bool_t make_binding( motor::string_in_t name, motor::wire::ioutput_slot_mtr_t s ) noexcept
    {
        using type_t = T;

        auto * slot = dynamic_cast< motor::wire::output_slot< type_t > * >( s );
        if( slot != nullptr )
        {
            using slot_t = motor::wire::output_slot< type_t >;
            using var_t = motor::graphics::data_variable< type_t >;

            auto * var = motor::graphics::any_variable< type_t >( _vs, name );

            // dont create a binding if the variable is already added.
            {
                size_t idx = size_t( -1 );
                while( ++idx < _bindings.size() && _bindings[ idx ].gvar != var );
                if( idx != _bindings.size() ) return true;
            }

            slot->set_value( var->get() );

            _bindings.emplace_back(
                variable_binding{ var, motor::share( s ), [ = ]( this_t::variable_binding & v )
            {
                auto * s_local = reinterpret_cast< slot_t * >( v.slot );
                auto * v_local = reinterpret_cast< var_t * >( v.gvar );

                s_local->set_value( v_local->get() );
            } } );

            return true;
        }

        return false;
    }
    #endif
};
motor_typedef( wire_variable_output_bridge );
} // namespace graphics
} // namespace motor