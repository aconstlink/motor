//------------------------------------------------------------
// verzerrt (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#include "transform_3d.h"

#include "../../visitor/ivisitor.h"

#include <verzerrt/log/log.h>

using namespace vzt_scene ;
using namespace vzt_scene::vzt_node ;

//*******************************************************************
void_t transform_3d::ctor_init( void_t )
{
    _var_trafo = var_mat4f_t::create( vzt_math::mat4f_t().identity(),
        "[transform_3d::transform_3d] : matrix variable" ) ;

    this_t::bind_variable( "self.trafo", _var_trafo ) ;
    this_t::reconnect_flow_node() ;
}

//*******************************************************************
transform_3d::transform_3d( void_t ) 
{
    this_t::ctor_init() ;
}

//*******************************************************************
transform_3d::transform_3d( vzt_math::vzt_3d::trafof_cref_t trafo ) : _trafo( trafo )
{
    this_t::ctor_init() ;
}

//*******************************************************************
transform_3d::transform_3d( iparent_ptr_t parent_ptr ) : base_t( parent_ptr )
{
    this_t::ctor_init() ;
}

//*******************************************************************
transform_3d::transform_3d( node_ptr_t transformee_ptr, iparent_ptr_t parent_ptr ) : base_t( parent_ptr )
{
    this_t::ctor_init() ;
    this_t::set_decorated( transformee_ptr ) ;
}

//*******************************************************************
transform_3d::transform_3d( this_rref_t rhv ) : base_t( std::move(rhv) ) 
{
    _trafo = rhv._trafo ;
    vzt_move_member_ptr( _var_trafo, rhv ) ;
}

//*******************************************************************
transform_3d::transform_3d( this_cref_t rhv ) : base_t()
{
    _trafo = rhv._trafo ;
}

//*******************************************************************
transform_3d::~transform_3d( void_t ) 
{}

//*******************************************************************
transform_3d::this_ptr_t transform_3d::create( vzt_memory::purpose_cref_t purp )
{
    return vzt_scene::memory::alloc( this_t(), purp ) ;
}

//*******************************************************************
transform_3d::this_ptr_t transform_3d::create( this_rref_t rhv, vzt_memory::purpose_cref_t purp ) 
{
    return vzt_scene::memory::alloc( std::move(rhv), purp ) ;
}

//*******************************************************************
void_t transform_3d::destroy( this_ptr_t ptr ) 
{
    vzt_scene::memory::dealloc( ptr ) ;
}

//*******************************************************************
transform_3d::this_ref_t transform_3d::set_decorated( node_ptr_t transformee_ptr ) 
{
    base_t::set_decorated( transformee_ptr ) ;
    return *this ;
}

//*******************************************************************
void_t transform_3d::set_transformation( trafo_cref_t trafo ) 
{
    _trafo = trafo ;
}

//*******************************************************************
transform_3d::trafo_t transform_3d::compute_trafo( void_t ) const 
{
    return trafo_t(_var_trafo->get_data()) * _trafo ;
}

//*******************************************************************
transform_3d::trafo_cref_t transform_3d::get_transformation( void_t ) const 
{
    return _trafo ;
}

//*******************************************************************
vzt_scene::result transform_3d::apply( vzt_scene::vzt_visitor::ivisitor_ptr_t vptr ) 
{
    auto res = vptr->visit( this ) ;
    vzt_log::log::error( vzt_scene::no_success(res), 
        "[vzt_scene::transform::apply] : visit" ) ;

    this_t::traverse_decorated( vptr ) ;

    return vptr->post_visit( this ) ;
}

//*******************************************************************
void_t transform_3d::destroy( void_t ) 
{
    this_t::destroy( this ) ;
}
