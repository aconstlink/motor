
#include "render_state_component.h"

using namespace motor::scene ;

//********************************************************************************
render_state_component::render_state_component( motor::graphics::render_state_sets_rref_t rss ) noexcept : _so( std::move( rss ) ) 
{
}

//********************************************************************************
render_state_component::render_state_component( motor::graphics::state_object_rref_t so ) noexcept : _so( std::move( so ) ) 
{
}

//********************************************************************************
render_state_component::~render_state_component( void_t ) noexcept 
{}