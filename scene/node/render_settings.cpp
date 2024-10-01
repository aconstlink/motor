

#include "render_settings.h"

using namespace motor::scene ;

motor_core_dd_id_init( render_settings ) ;

//************************************************************************************************
render_settings::render_settings( this_rref_t rhv ) noexcept : _rs( motor::move( rhv._rs )  )
{
}

//************************************************************************************************
render_settings::render_settings( node_ptr_t decorated ) noexcept : base_t( decorated ) 
{
}

//************************************************************************************************
render_settings::render_settings( motor::graphics::state_object_mtr_safe_t rs ) noexcept : _rs( motor::move( rs ) )
{
}

//************************************************************************************************
render_settings::render_settings( motor::graphics::state_object_mtr_safe_t rs, node_ptr_t decorated ) noexcept : 
    _rs( motor::move( rs ) ), base_t( decorated )
{
}

//************************************************************************************************
render_settings::~render_settings( void_t ) noexcept
{
    motor::memory::release_ptr( motor::move( _rs ) ) ;
}