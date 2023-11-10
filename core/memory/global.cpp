
#include "global.h"

#include "manager/manager.h"

#include <motor/core/log/global.h>


using namespace motor::memory ;

std::mutex global::_mtx ;
global::this_ptr_t global::_ptr = nullptr ;

//*************************************************************************************
global::global( void_t )
{
    _manager = motor::memory::manager_t::create() ;
}

//*************************************************************************************
global::global( this_rref_t rhv )
{
    motor_move_member_ptr( _manager, rhv ) ;
}

//*************************************************************************************
global::~global( void_t )
{
    //motor_save_call( _manager, destroy )
    if( _manager != nullptr )
        _manager->destroy() ;
}

//*************************************************************************************
global::this_ptr_t global::init( void_t )
{
    if( this_t::_ptr != nullptr ) return this_t::_ptr ;

    {
        std::lock_guard< std::mutex > lk( this_t::_mtx ) ;

        if( this_t::_ptr != nullptr ) return this_t::_ptr ;
        this_t::_ptr = new this_t() ;

        motor::log::global_t::status( "[online] : natus memory" ) ;
    }

    return this_t::_ptr ;
}

//*************************************************************************************
void_t global::deinit( void_t )
{
    if( global_t::_ptr == nullptr )
        return ;

    delete global_t::_ptr ;
    this_t::_ptr = nullptr ;
}

//*************************************************************************************
global::this_ptr_t global::get( void_t )
{
    return this_t::init() ;
}

//*************************************************************************************
void global::dump_to_std( void_t )
{
    this_t::get()->_manager->dump_to_std() ;
}

//*************************************************************************************
void_ptr_t global::alloc( size_t sib, motor::memory::purpose_cref_t purpose )
{
    return this_t::get()->_manager->alloc( sib, purpose ) ;
}

//*************************************************************************************
void_ptr_t global::alloc( size_t sib )
{
    return this_t::get()->_manager->alloc( sib ) ;
}

//*************************************************************************************
void_ptr_t global::dealloc( void_ptr_t ptr )
{
    if( ptr == nullptr ) return nullptr ;

    this_t::get()->_manager->dealloc( ptr ) ;

    return nullptr ;
}

//*************************************************************************************
size_t global::get_sib( void_t )
{
    return this_t::get()->_manager->get_sib() ;
}

//*************************************************************************************
bool_t global::get_purpose( void_ptr_t ptr, motor::memory::purpose_ref_t pout )
{
    return this_t::get()->_manager->get_purpose( ptr, pout ) ;
}
