
#include "animation_component.h"

using namespace motor;
using namespace motor::scene;

//********************************************************************
animation_component::animation_component( void_t ) noexcept {}

//********************************************************************
animation_component::animation_component( this_rref_t rhv ) noexcept
    : _controllers( std::move( rhv._controllers ) )
{
}

//********************************************************************
animation_component::~animation_component( void_t ) noexcept
{
    for( auto * mtr : _controllers )
    {
        motor::release( motor::move( mtr ) );
    }
}

//********************************************************************
bool_t animation_component::attach_controller( animation_controller_mtr_safe_t ctrl ) noexcept
{
    _controllers.emplace_back( motor::move( ctrl ) );
    return true;
}