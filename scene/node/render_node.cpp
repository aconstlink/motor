#include "render_node.h"

using namespace motor::scene ;

motor_core_dd_id_init( render_node ) ;

//*****************************************************************
render_node::render_node( this_rref_t rhv ) noexcept : 
    base_t( std::move( rhv) ), _msl( motor::move( rhv._msl ) ), _vs( rhv._vs )
{
    
}

//*****************************************************************
render_node::render_node( motor::graphics::msl_object_mtr_safe_t msl ) noexcept : 
    _msl( motor::move( msl ) )
{
}

//*****************************************************************
render_node::render_node( motor::graphics::msl_object_mtr_safe_t msl, size_t const vs ) noexcept : 
    _msl( motor::move(msl) ), _vs(vs)
{
}

//*****************************************************************
render_node::~render_node( void_t ) noexcept
{
    motor::memory::release_ptr( _msl ) ;
}

//*****************************************************************