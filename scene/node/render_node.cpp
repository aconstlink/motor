#include "render_node.h"

using namespace motor::scene ;

motor_core_dd_id_init( render_node ) ;

//*****************************************************************
render_node::render_node( this_rref_t rhv ) noexcept : _msl( motor::move( rhv._msl ) )
{
}

//*****************************************************************
render_node::render_node( node_ptr_t ) noexcept
{
}

//*****************************************************************
render_node::render_node( motor::graphics::msl_object_mtr_safe_t msl ) noexcept : _msl( motor::move( msl ) )
{
}

//*****************************************************************
render_node::~render_node( void_t ) noexcept
{
}

//*****************************************************************