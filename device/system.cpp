
#include "system.h"

using namespace motor::device ;

//***************************************************************
system::system( void_t ) 
{
    //_vdev = motor::device::vdev_module_t() ;
}

//***************************************************************
system::system( this_rref_t rhv )
{
    _modules = std::move( rhv._modules ) ;
    _vdev = std::move( rhv._vdev ) ;
}

//***************************************************************
system::~system( void_t )
{
    this_t::release() ;
}

//***************************************************************
void_t system::add_module( motor::device::imodule_mtr_t mod ) noexcept
{
    _modules.emplace_back( motor::memory::copy_ptr( mod ) ) ;

    // every time a new module is added, 
    // all the vdev should check  all mappings
    //_vdev->check_devices( res ) ;
}

//***************************************************************
void_t system::add_module( motor::device::imodule_mtr_moved_t mod ) noexcept
{
    _modules.emplace_back( mod ) ;

    // every time a new module is added, 
    // all the vdev should check  all mappings
    //_vdev->check_devices( res ) ;
}

//***************************************************************
void_t system::search( motor::device::imodule::search_funk_t funk ) 
{
    for( auto & mod : _modules )
    {
        mod->search( funk ) ;
    }
    //_vdev->search( funk ) ;
}

//***************************************************************
void_t system::update( void_t )
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
void_t system::release( void_t ) noexcept 
{
    //_vdev->release() ;

    for( auto & res : _modules )
    {
        res->release() ;
    }
    //_vdev.reset() ;
    _modules.clear() ;
}