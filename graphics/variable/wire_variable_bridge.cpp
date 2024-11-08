
#include "wire_variable_bridge.h"

using namespace motor::graphics ;

//*****************************************************************
wire_variable_bridge::wire_variable_bridge( void_t ) noexcept 
{
}

//*****************************************************************
wire_variable_bridge::wire_variable_bridge( motor::graphics::variable_set_mtr_safe_t vs ) noexcept : 
    _vs( motor::move( vs ) )
{
}

//*****************************************************************
wire_variable_bridge::wire_variable_bridge( this_rref_t rhv ) noexcept
{
    motor::release( motor::move( _vs ) ) ;
    _vs = motor::move( rhv._vs ) ;

    _inputs = std::move( rhv._inputs ) ;

    this_t::clear_bindings() ;
    _bindings = std::move( rhv._bindings ) ;
}

//*****************************************************************
wire_variable_bridge::~wire_variable_bridge( void_t ) noexcept 
{    
    this_t::clear_bindings() ;
    motor::release( motor::move( _vs  ) ) ;
}

//*****************************************************************
void_t wire_variable_bridge::pull_data( void_t ) noexcept 
{
    for( auto & b : _bindings )
    {
        b.pull_funk( b ) ;
    }
}

//*****************************************************************
void_t wire_variable_bridge::update_bindings( bool_t const clear ) noexcept 
{
    this_t::create_bindings() ;
    
    if( _vs == nullptr ) return ;

    // clear up every slot which is not in the shader
    // variable set.
    if( clear )
    {
        std::vector< motor::string_t > removeables ;

        _inputs.for_each_slot( [&] ( motor::string_in_t name, motor::wire::iinput_slot_ptr_t )
        {
            if ( !_vs->has_any_variable( name ) )
            {
                removeables.emplace_back( name ) ;
            }
        } ) ;

        for ( auto const & name : removeables )
        {
            _inputs.remove( name ) ;
        }
    }
}

//*****************************************************************
void_t wire_variable_bridge::update_bindings( motor::graphics::variable_set_mtr_safe_t vs ) noexcept 
{
    motor::release( motor::move( _vs ) ) ;
    _vs = motor::move( vs ) ;
    this_t::update_bindings() ;
}

//*****************************************************************
motor::wire::inputs_ptr_t wire_variable_bridge::borrow_inputs( void_t ) noexcept 
{
    return &_inputs ;
}

//*****************************************************************
motor::wire::inputs_cptr_t wire_variable_bridge::borrow_inputs( void_t ) const noexcept 
{
    return &_inputs ;
}

//*****************************************************************
void_t wire_variable_bridge::create_bindings( void_t ) noexcept 
{
    this_t::clear_bindings() ;

    if( _vs == nullptr ) 
    {
        _inputs.clear() ;
        return ;
    }

    // for each variable in the graphics variable set, make a binding
    {
        _vs->for_each_data_variable( [&] ( motor::string_in_t name, motor::graphics::ivariable_ptr_t var )
        {
            if ( this_t::make_binding< int_t >( name, var ) ) return ;
            if ( this_t::make_binding< float_t >( name, var ) ) return ;
            if ( this_t::make_binding< uint_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec2f_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec3f_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec4f_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec2i_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec3i_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec4i_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec2ui_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec3ui_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::vec4ui_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::mat2f_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::mat3f_t >( name, var ) ) return ;
            if ( this_t::make_binding< motor::math::mat4f_t >( name, var ) ) return ;
        } ) ;

        _vs->for_each_texture_variable( [&] ( motor::string_in_t name, motor::graphics::data_variable<motor::string_t> * var )
        {
            if ( this_t::make_binding< motor::graphics::texture_variable_data >( name, var ) ) return ;
        } ) ;
    }

    // for each variable in the input slots sheet, make a binding
    {
        _inputs.for_each_slot( [&] ( motor::string_in_t name, motor::wire::iinput_slot_mtr_t s )
        {
            if ( this_t::make_binding< int_t >( name, s ) ) return ;
            if ( this_t::make_binding< float_t >( name, s ) ) return ;
            if ( this_t::make_binding< uint_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec2f_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec3f_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec4f_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec2i_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec3i_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec4i_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec2ui_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec3ui_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::vec4ui_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::mat2f_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::mat3f_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::math::mat4f_t >( name, s ) ) return ;
            if ( this_t::make_binding< motor::graphics::texture_variable_data >( name, s ) ) return ;
            if ( this_t::make_binding< motor::graphics::array_variable_data >( name, s ) ) return ;
            if ( this_t::make_binding< motor::graphics::streamout_variable_data >( name, s ) ) return ;
        } ) ;
    }
}

//*****************************************************************
void_t wire_variable_bridge::clear_bindings( void_t ) noexcept 
{
    for( auto & b : _bindings )
    {
        motor::release( motor::move( b.slot ) ) ;
    }
    _bindings.clear() ;
}