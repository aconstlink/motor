
#include "default_visitor.h"

#include "../node/group.h"
#include "../node/leaf.h"

using namespace motor::scene;

default_visitor::~default_visitor( void_t ) noexcept {}

//**********************************************************************************
motor::scene::result default_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    return this->visit( static_cast< motor::scene::node_ptr_t >( nptr ) );
}

//**********************************************************************************
motor::scene::result default_visitor::post_visit( motor::scene::group_ptr_t nptr,
                                                  motor::scene::result const res ) noexcept
{
    return this->post_visit( static_cast< motor::scene::node_ptr_t >( nptr ), res );
}

//**********************************************************************************
motor::scene::result default_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    auto const res = this->visit( static_cast< motor::scene::node_ptr_t >( nptr ) );
    return this->post_visit( static_cast< motor::scene::node_ptr_t >( nptr ), res );
}