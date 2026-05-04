

#include "global.h"

#include "node/group.h"
#include "node/decorator.h"
#include "node/trafo3d_node.h"
#include "node/render_node.h"
#include "node/render_settings.h"
#include "visitor/trafo_visitor.h"
#include "visitor/log_visitor.h"
#include "visitor/render_visitor.h"
#include "visitor/variable_update_visitor.h"

using namespace motor::scene ;

//***********************************************************************
void_t global::init( void_t ) noexcept 
{
}

//***********************************************************************
void_t global::deinit( void_t ) noexcept 
{
}