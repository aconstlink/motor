
#include "camera_component.h"

using namespace motor::scene ;

//**************************************************************************************
camera_component::camera_component( void_t ) noexcept
{
}

//**************************************************************************************
camera_component::camera_component( motor::gfx::generic_camera_mtr_safe_t cam ) noexcept :
	_cam( motor::move(cam) )
{
}

//**************************************************************************************
camera_component::camera_component( this_rref_t rhv ) noexcept : _cam( motor::move(rhv._cam) )
{
}

//**************************************************************************************
camera_component::~camera_component( void_t ) noexcept
{
	motor::release( motor::move( _cam ) ) ;
}

//**************************************************************************************