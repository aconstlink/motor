
#include "imgui_node_visitor.h"

#include <motor/scene/node/node.h>
#include <motor/scene/node/group.h>
#include <motor/scene/node/leaf.h>

#include <motor/scene/component/name_component.hpp>

#include <motor/std/string>
#include <imgui.h>

using namespace motor::tool ;


//************************************************************************
imgui_node_visitor::imgui_node_visitor( void_t ) noexcept 
{
}

//************************************************************************
imgui_node_visitor::~imgui_node_visitor( void_t ) noexcept
{
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::node_ptr_t ) noexcept
{
    return motor::scene::result::ok ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::group_ptr_t nptr ) noexcept
{
    ImGui::PushID( ++_id ) ;

    // check for name component first...
    motor::string_t name = this_t::check_for_name( "Group", nptr ) ;
    if( !ImGui::TreeNode( "", name.c_str() ) )
    {
        return motor::scene::result::no_descent ;
    }

    // maybe check for properties or components

    ++_depth ;
    return motor::scene::result::ok ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::post_visit( motor::scene::group_ptr_t, motor::scene::result const r ) noexcept
{
    // r is ok then children were traversed.
    if ( r == motor::scene::result::ok ) ImGui::TreePop();
    ImGui::PopID() ;
    --_depth ;
    return motor::scene::result::ok ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    ImGui::PushID( ++_id ) ;

    // check for name component first...
    motor::string_t name = this_t::check_for_name( "Leaf", nptr ) ;
    if ( ImGui::TreeNode( "", name.c_str() ) )
    {
        ImGui::TreePop() ;
    }
    ImGui::PopID() ;

    return motor::scene::result::ok ;
}

//************************************************************************
void_t imgui_node_visitor::on_finish( void_t ) noexcept
{
    _id = 0 ;
}

//************************************************************************
motor::string_t imgui_node_visitor::check_for_name( motor::string_rref_t sin, motor::scene::node_ptr_t nptr ) const noexcept
{
    motor::scene::name_component_ptr_t comp ;
    if ( nptr->borrow_component( comp ) )
    {
        return comp->get_name() ;
    }
    return sin ;
}