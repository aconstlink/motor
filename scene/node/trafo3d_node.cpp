
#include "trafo3d_node.h"

#include "../visitor/ivisitor.h"

using namespace motor::scene ;

motor_core_dd_id_init( trafo3d_node ) ;

//**********************************************************************************
trafo3d_node::trafo3d_node( void_t ) noexcept 
{}

//**********************************************************************************
trafo3d_node::trafo3d_node( motor::math::m3d::trafof_cref_t trafo ) noexcept :
    _trafo( motor::shared( this_t::in_slot_t( trafo ) ) )
{
}

//**********************************************************************************
trafo3d_node::trafo3d_node( this_rref_t rhv ) noexcept : base_t( std::move( rhv ) ),
    _trafo( motor::move( rhv._trafo ) ), _computed( motor::move( rhv._computed ) )
{}

//**********************************************************************************
trafo3d_node::~trafo3d_node( void_t ) noexcept
{
    motor::release( motor::move( _trafo ) ) ;
    motor::release( motor::move( _computed ) ) ;
}

//**********************************************************************************
bool_t trafo3d_node::inputs( motor::wire::inputs_out_t ins ) noexcept 
{
    ins = motor::wire::inputs_t( { { "trafo", motor::share( this_t::_trafo )}  } ) ;
    return true ;
}

#if 0
//**********************************************************************************
motor::scene::result trafo3d_node::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept 
{
    motor::scene::result const r = vptr->visit( this ) ;

    if ( motor::scene::success( r ) )
    {
        base_t::traverse_decorated( vptr ) ;
    }

    return vptr->post_visit( this, r ) ;
}
#endif