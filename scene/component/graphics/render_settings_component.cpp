
#include "render_settings_component.h"

using namespace motor::scene ;

//************************************************************************************************
render_settings_component::render_settings_component( this_rref_t rhv ) noexcept : _rs( motor::move( rhv._rs )  )
{
}

//************************************************************************************************
render_settings_component::render_settings_component( node_ptr_t decorated ) noexcept
{
}

//************************************************************************************************
render_settings_component::render_settings_component( motor::graphics::state_object_mtr_safe_t rs ) noexcept : _rs( motor::move( rs ) )
{
}

//************************************************************************************************
render_settings_component::render_settings_component( motor::graphics::state_object_mtr_safe_t rs, node_ptr_t decorated ) noexcept : 
    _rs( motor::move( rs ) )
{
}

//************************************************************************************************
render_settings_component::~render_settings_component( void_t ) noexcept
{
    motor::memory::release_ptr( motor::move( _rs ) ) ;
}