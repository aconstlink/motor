

#include "render_settings.h"

#include "../visitor/ivisitor.h"

using namespace motor::scene ;

#if 0

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

//**********************************************************************************
motor::scene::result render_settings::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept 
{
    motor::scene::result const r = vptr->visit( this ) ;

    if ( motor::scene::success( r ) )
    {
        base_t::traverse_decorated( vptr ) ;
    }

    return vptr->post_visit( this, r ) ;
}

#endif