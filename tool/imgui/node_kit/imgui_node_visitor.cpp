
#include "imgui_node_visitor.h"

#include <motor/scene/global.h>
#include <motor/scene/node/node.h>
#include <motor/scene/node/group.h>
#include <motor/scene/node/decorator.h>
#include <motor/scene/node/leaf.h>
#include <motor/scene/node/trafo3d_node.h>
#include <motor/scene/node/camera_node.h>
#include <motor/scene/node/render_node.h>
#include <motor/scene/node/render_settings.h>

#include <motor/scene/component/name_component.hpp>

#include <motor/std/string>
#include <imgui.h>

using namespace motor::tool ;

motor_core_dd_id_init( imgui_node_visitor ) ;

//************************************************************************
imgui_node_visitor::imgui_node_visitor( void_t ) noexcept 
{
}

//************************************************************************
imgui_node_visitor::~imgui_node_visitor( void_t ) noexcept
{
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::ivisitable_ptr_t vptr ) noexcept 
{
    /*if( dynamic_cast< motor::scene::group_ptr_t>( vptr ) != nullptr )
    {
        return this->visit( static_cast< motor::scene::group_ptr_t>( vptr ) ) ;
    }
    else */if ( dynamic_cast<motor::scene::decorator_ptr_t>( vptr ) != nullptr )
    {
        return this->visit( static_cast< motor::scene::decorator_ptr_t>( vptr ) ) ;
    }
    else if( dynamic_cast<motor::scene::leaf_ptr_t>( vptr ) != nullptr )
    {
        return this->visit( static_cast< motor::scene::leaf_ptr_t>( vptr ) ) ;
    }

    return motor::scene::result::not_implemented ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::post_visit( motor::scene::ivisitable_ptr_t vptr, motor::scene::result const r ) noexcept 
{
    /*if ( dynamic_cast<motor::scene::group_ptr_t>( vptr ) != nullptr )
    {
        return this->post_visit( static_cast<motor::scene::group_ptr_t>( vptr ), r ) ;
    }
    else */if ( dynamic_cast<motor::scene::decorator_ptr_t>( vptr ) != nullptr )
    {
        return this->post_visit( static_cast<motor::scene::decorator_ptr_t>( vptr ), r ) ;
    }

    return motor::scene::result::not_implemented ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::decorator_ptr_t nptr ) noexcept
{
    ImGui::PushID( ++_id ) ;

    // check for name component first...
    motor::string_t name = this_t::check_for_name( "Decorator", nptr ) ;
    if ( !ImGui::TreeNode( "", name.c_str() ) )
    {
        return motor::scene::result::no_descent ;
    }

    //this_t::list_components( nptr ) ;

    ++_depth ;
    return motor::scene::result::ok ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::post_visit( motor::scene::decorator_ptr_t, motor::scene::result const r ) noexcept
{
    // r is ok then children were traversed.
    if ( r == motor::scene::result::ok ) ImGui::TreePop();
    ImGui::PopID() ;
    --_depth ;
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

    //this_t::list_components( nptr ) ;

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
        //this_t::list_components( nptr ) ;
        ImGui::TreePop() ;
    }

    ImGui::PopID() ;

    return motor::scene::result::ok ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::render_settings_ptr_t nptr ) noexcept 
{
    auto const res = this_t::visit( static_cast< motor::scene::decorator_ptr_t >( nptr ) ) ;

    {
        auto * ptr = this_t::check_and_borrow_imgui_component( nptr ) ;

    }
    return res ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::post_visit( motor::scene::render_settings_ptr_t nptr, motor::scene::result const r ) noexcept
{
    auto const res = this_t::post_visit( static_cast< motor::scene::decorator_ptr_t >( nptr ), r ) ;

    return res ;
}

//************************************************************************
motor::scene::result imgui_node_visitor::visit( motor::scene::render_node_ptr_t nptr ) noexcept
{
    auto const res = this_t::visit( static_cast< motor::scene::leaf_ptr_t >( nptr ) ) ;

    return motor::scene::result::ok ;
}

//************************************************************************
void_t imgui_node_visitor::on_finish( void_t ) noexcept
{
    _id = 0 ;
}

//************************************************************************
void_t imgui_node_visitor::init_function_callbacks( void_t ) noexcept 
{
    motor::scene::global::register_default_callbacks<this_t, motor::scene::group>() ;
    motor::scene::global::register_default_callbacks<this_t, motor::scene::leaf>() ;
    motor::scene::global::register_default_callbacks<this_t, motor::scene::trafo3d_node>() ;
    motor::scene::global::register_default_callbacks<this_t, motor::scene::render_node>() ;
    motor::scene::global::register_default_callbacks<this_t, motor::scene::render_settings>() ;
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

//************************************************************************
void_t imgui_node_visitor::list_components( motor::scene::node_ptr_t nptr ) noexcept 
{
    ImGui::PushID( ++_id ) ;

    if ( ImGui::TreeNode( "", "components" ) )
    {
        nptr->for_each_component( [&]( motor::scene::icomponent_ptr_t comp )
        {
            ImGui::PushID( ++_id ) ;
            if( ImGui::TreeNode("a component" ) )
            {
                ImGui::TreePop() ;
            }
            ImGui::PopID() ;
        } ) ;
        ImGui::TreePop() ;
    }

    ImGui::PopID() ;
}

//************************************************************************
motor::tool::imgui_node_component_mtr_t imgui_node_visitor::check_and_borrow_imgui_component( motor::scene::node_ptr_t nptr ) noexcept 
{
    motor::tool::imgui_node_component_mtr_t comp = nullptr ;

    if( !nptr->borrow_component( comp ) )
    {
        comp = motor::shared( motor::tool::imgui_node_component_t() ) ;
        auto const b = nptr->add_component( motor::share_unsafe( comp ) ) ;
        
        assert( b == true ) ;
    }

    return comp ;
}
