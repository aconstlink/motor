
#include "system.h"

using namespace motor::network ;

//*******************************************************************
system::system( void_t ) noexcept 
{
}

//*******************************************************************
system::system( this_rref_t rhv ) noexcept 
{
    _mods = std::move( rhv._mods ) ;
}

//*******************************************************************
system::~system( void_t ) noexcept 
{
    for( auto * mod : _mods )
    {
        motor::memory::release_ptr( mod ) ;
    }
}

//*******************************************************************
void_t system::add_module( motor::network::imodule_mtr_rref_t mod ) noexcept 
{
    _mods.emplace_back( motor::move( mod ) ) ;
}

//*******************************************************************
void_t system::modules( for_each_module_funk_t funk ) noexcept 
{
    for( auto * ptr : _mods )
    {
        if( funk( ptr ) ) break ;
    }
}