
#include "system.h"

using namespace motor::controls ;

//***************************************************************
system::system( void_t ) noexcept
{
    //_vdev = motor::controls::vdev_module_t() ;
}

//***************************************************************
system::system( this_rref_t rhv ) noexcept
{
    _modules = std::move( rhv._modules ) ;
    _vdev = std::move( rhv._vdev ) ;
}

//***************************************************************
system::~system( void_t ) noexcept
{
    this_t::release() ;
}

//***************************************************************
void_t system::add_module( motor::controls::imodule_mtr_safe_t mod ) noexcept
{
    _modules.emplace_back( mod ) ;

    // every time a new module is added, 
    // all the vdev should check  all mappings
    //_vdev->check_devices( res ) ;
}


//***************************************************************
void_t system::search( motor::controls::imodule::search_funk_t funk ) noexcept
{
    for( auto & mod : _modules )
    {
        mod->search( funk ) ;
    }
    //_vdev->search( funk ) ;
}

//***************************************************************
void_t system::update( void_t ) noexcept
{
    // update all modules which should update all physicals
    for( auto & res : _modules )
    {
        res->update() ;
    }

    // the virtual device module must update 
    // after all physical devices
    //_vdev->update() ;
}

//***************************************************************
void_t system::install( motor::controls::iobserver_borrow_t::mtr_t ptr ) noexcept 
{
    for( auto & res : _modules )
    {
        res->install( ptr ) ;
    }
}

//***************************************************************
void_t system::release( void_t ) noexcept 
{
    //_vdev->release() ;

    for( auto & res : _modules )
    {
        motor::memory::release_ptr( res ) ;
    }
    //_vdev.reset() ;
    _modules.clear() ;
}