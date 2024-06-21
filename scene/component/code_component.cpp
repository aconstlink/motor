


#include "code_component.h"

using namespace motor::scene ;


//**********************************************************************
void_t code_component::execute( motor::scene::node_mtr_t n ) noexcept
{
    _funk( n ) ;
}