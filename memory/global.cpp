
#include "global.h"

#include "manager/manager.h"

using namespace motor::memory ;

std::mutex global::_mtx ;
global::this_ptr_t global::_ptr = nullptr ;

global::get_manager_funk_t global::get_mgr = [=]( void_t )
{
    return motor::memory::global_t::init()->_manager ;
} ;

//*************************************************************************************
global::global( void_t ) noexcept 
{
    _manager = motor::memory::manager_t::create() ;
}

//*************************************************************************************
global::global( this_rref_t rhv ) noexcept 
{
    motor_move_member_ptr( _manager, rhv ) ;
}

//*************************************************************************************
global::~global( void_t ) noexcept 
{
    //motor_save_call( _manager, destroy )
    if( _manager != nullptr )
        _manager->destroy() ;
}

//*************************************************************************************
global::this_ptr_t global::init( void_t ) noexcept 
{
    if( this_t::_ptr != nullptr ) return this_t::_ptr ;

    {
        std::lock_guard< std::mutex > lk( this_t::_mtx ) ;

        if( this_t::_ptr != nullptr ) return this_t::_ptr ;
        this_t::_ptr = new this_t() ;

        this_t::get_mgr = [=]( void_t ){ return this_t::_ptr->_manager ; } ;
    }
    //motor::log::global_t::status( "[online] : natus memory" ) ;

    return this_t::_ptr ;
}

//*************************************************************************************
void_t global::deinit( void_t ) noexcept 
{
    if( global_t::_ptr == nullptr )
        return ;

    delete global_t::_ptr ;
    this_t::_ptr = nullptr ;
}

//*************************************************************************************
global::this_ptr_t global::get( void_t ) noexcept 
{
    return this_t::init() ;
}

//*************************************************************************************
void global::dump_to_std( void_t ) noexcept 
{
    this_t::get_mgr()->dump_to_std() ;
}

//*************************************************************************************
void_ptr_t global::create( size_t const sib, char_cptr_t purpose ) noexcept 
{
    return this_t::get_mgr()->create( sib, purpose ) ;
}

//*************************************************************************************
void_ptr_t global::create( size_t const sib ) noexcept 
{
    return this_t::get_mgr()->create( sib ) ;
}

//*************************************************************************************
void_ptr_t global::create( void_ptr_t ptr ) noexcept 
{
    return this_t::get_mgr()->create( ptr ) ;
}

//*************************************************************************************
void_ptr_t global::release( void_ptr_t ptr ) noexcept 
{
    return this_t::get_mgr()->release( ptr ) ;
}

//*************************************************************************************
void_ptr_t global::alloc( size_t const sib, char_cptr_t purpose ) noexcept 
{
    return this_t::get_mgr()->alloc( sib, purpose ) ;
}

//*************************************************************************************
void_ptr_t global::alloc( size_t const sib ) noexcept 
{
    return this_t::get_mgr()->alloc( sib ) ;
}

//*************************************************************************************
void_ptr_t global::dealloc( void_ptr_t ptr ) noexcept 
{
    if( ptr == nullptr ) return nullptr ;

    this_t::get_mgr()->dealloc( ptr ) ;

    return nullptr ;
}

//*************************************************************************************
size_t global::get_sib( void_t ) noexcept 
{
    return this_t::get_mgr()->get_sib() ;
}

//*************************************************************************************
bool_t global::get_purpose( void_ptr_t ptr, char_cptr_t & pout ) noexcept 
{
    return this_t::get_mgr()->get_purpose( ptr, pout ) ;
}
