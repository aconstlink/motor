#pragma once

#include "variable_set.hpp"

namespace motor
{
namespace graphics
{
// DO NOT USE - SAVED FOR LATER
// a variable set view allows to mix own and borrowed
// variables in its internal variable set.
class variable_set_view
{
    motor_this_typedefs( variable_set_view );

  private:

    motor::graphics::variable_set_mtr_t _var_set =
        motor::shared( motor::graphics::variable_set_t() );

    // shared variables require its variable set to be
    // referenced. So variables in _var_set may be borrowed.
    struct hold_data
    {
        // count how many variables we borrowed.
        // if count == 0, remove this entry and release
        // the variable set itself.
        size_t var_count;
        motor::graphics::variable_set_mtr_t vs;
    };
    motor::vector< hold_data > _hold;

  public:

    variable_set_view( void_t ) noexcept {}
    variable_set_view( this_rref_t rhv ) noexcept
        : _var_set( motor::move( rhv._var_set ) ), _hold( std::move( rhv._hold ) )
    {
    }
    ~variable_set_view( void_t ) noexcept
    {
        this_t::clear();
    }

  public: // wrapper to variable set

    void_t clear( void_t ) noexcept
    {
        _var_set->clear();
    }

    template < class T >
    motor::graphics::data_variable< T > * data_variable( char const * const name ) noexcept
    {
        return _var_set->data_variable< T >( name );
    }

    template < class T >
    motor::graphics::data_variable< T > * data_variable( motor::string_cref_t name ) noexcept
    {
        return _var_set->data_variable< T >( name );
    }

    motor::graphics::texture_variable_t * find_texture_variable(
        char const * const name ) const noexcept
    {
        return _var_set->find_texture_variable( name );
    }

    //***************************************************************************************
    bool_t has_texture_variable( motor::string_in_t name ) const noexcept
    {
        return _var_set->has_texture_variable( name );
    }

    //***************************************************************************************
    motor::graphics::texture_variable_t * texture_variable( char const * const name ) noexcept
    {
        return _var_set->texture_variable( name );
    }

    //***************************************************************************************
    motor::graphics::texture_variable_t * texture_variable( motor::string_in_t name ) noexcept
    {
        return _var_set->texture_variable( name );
    }

    //***************************************************************************************
    motor::graphics::array_variable_t * array_variable( char const * const name ) noexcept
    {
        return _var_set->array_variable( name );
    }

    //***************************************************************************************
    motor::graphics::array_variable_t * array_variable( motor::string_in_t name ) noexcept
    {
        return _var_set->array_variable( name );
    }

    //***************************************************************************************
    motor::graphics::streamout_variable_t * array_variable_streamout(
        motor::string_in_t name ) noexcept
    {
        return _var_set->array_variable_streamout( name );
    }

    //***************************************************************************************
    bool_t has_any_variable( motor::string_in_t name ) const noexcept
    {
        _var_set->has_any_variable( name );
    }

  public: // borrow interface

    

  public:

    motor::graphics::variable_set_mtr_safe_t get_variable_set( void_t ) noexcept
    {
        return motor::share( _var_set );
    }

    motor::graphics::variable_set_mtr_t borrow_variable_set( void_t ) noexcept
    {
        return _var_set;
    }
};
motor_typedef( variable_set_view );

} // namespace graphics
} // namespace motor