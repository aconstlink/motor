
#include "player_controller.h"

#include <imgui_internal.h>

using namespace motor::tool ;

//****************************************************************
player_controller::player_state player_controller::do_tool( motor::string_cref_t label_ ) noexcept 
{
    auto circle_button = [&]( motor::string_cref_t l, ImVec2 const & pos, float_t const r ) -> bool_t
    {
        ImGuiContext& ctx = *ImGui::GetCurrentContext() ;
        const ImGuiStyle& style = ctx.Style;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false ;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const ImGuiID id = window->GetID( l.c_str() ) ;
        
        ImVec2 const size( 2.0f * r, 2.0f * r ) ;

        const ImRect bb( pos, pos + size ) ;
        ImGui::ItemSize( size, style.FramePadding.y ) ;
        if( !ImGui::ItemAdd( bb, id ) )
            return false ;

        ImGuiButtonFlags flags = 0 ;
        if( ctx.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat )
            flags |= ImGuiButtonFlags_Repeat;

        bool hovered, held ;
        bool pressed = ImGui::ButtonBehavior( bb, id, &hovered, &held, flags );
        
        ImU32 const hover_color[] = 
        {
            IM_COL32(255, 255, 255, 255),
            IM_COL32(255, 0, 0, 255)
        } ;
        
        {
            ImVec2 const pos1 = pos + ImVec2( r, r ) ;
            draw_list->AddCircle( pos1, r, hover_color[ hovered ? 1 : 0 ], 0, 2.0f ) ;
            draw_list->AddCircleFilled( pos1, r, IM_COL32(255, 255, 255, 255) ) ;
        }

        return pressed ;
    } ;

    auto draw_prev = [&]( ImVec2 const & pos, float_t const r )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList() ;
        
        // tri
        {
            ImVec2 const p1 = pos + ImVec2( 1.5f * r, 0.5f * r ) ; // top
            ImVec2 const p2 = pos + ImVec2( 1.5f * r, 1.5f * r ) ; // bottom
            ImVec2 const p3 = pos + ImVec2( 0.6f * r, 1.0f * r ) ; // right

            ImVec2 const points[] = { p1, p2, p3 } ;
            draw_list->AddConvexPolyFilled( points, 3, IM_COL32(0, 0, 0, 255) ) ;
        }

        // stick
        {
            ImVec2 const p1 = pos + ImVec2( 0.5f * r, 1.5f * r ) ; // bottom
            ImVec2 const p2 = pos + ImVec2( 0.5f * r, 0.5f * r ) ; // top
            ImVec2 const p3 = pos + ImVec2( 0.7f * r, 0.5f * r ) ; // top
            ImVec2 const p4 = pos + ImVec2( 0.7f * r, 1.5f * r ) ; // bottom

            ImVec2 const points[] = { p1, p2, p3, p4 } ;
            draw_list->AddConvexPolyFilled( points, 4, IM_COL32(0, 0, 0, 255) ) ;
        }
    } ;

    auto draw_next = [&]( ImVec2 const & pos, float_t const r )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList() ;
        
        // tri
        {
            ImVec2 const p1 = pos + ImVec2( 0.6f * r, 0.5f * r ) ; // top
            ImVec2 const p2 = pos + ImVec2( 0.6f * r, 1.5f * r ) ; // bottom
            ImVec2 const p3 = pos + ImVec2( 1.5f * r, 1.0f * r ) ; // right

            ImVec2 const points[] = { p1, p2, p3 } ;
            draw_list->AddConvexPolyFilled( points, 3, IM_COL32(0, 0, 0, 255) ) ;
        }

        // stick
        {
            ImVec2 const p1 = pos + ImVec2( 1.3f * r, 1.5f * r ) ; // bottom
            ImVec2 const p2 = pos + ImVec2( 1.3f * r, 0.5f * r ) ; // top
            ImVec2 const p3 = pos + ImVec2( 1.5f * r, 0.5f * r ) ; // top
            ImVec2 const p4 = pos + ImVec2( 1.5f * r, 1.5f * r ) ; // bottom

            ImVec2 const points[] = { p1, p2, p3, p4 } ;
            draw_list->AddConvexPolyFilled( points, 4, IM_COL32(0, 0, 0, 255) ) ;
        }
    } ;

    auto draw_play = [&]( ImVec2 const & pos, float_t const r )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList() ;
        
        ImVec2 const p1 = pos + ImVec2( 0.6f * r, 0.5f * r ) ; // top
        ImVec2 const p2 = pos + ImVec2( 0.6f * r, 1.5f * r ) ; // bottom
        ImVec2 const p3 = pos + ImVec2( 1.5f * r, 1.0f * r ) ; // right

        draw_list->AddLine( p1, p2, IM_COL32(0, 0, 0, 255), 1.0f ) ;
        draw_list->AddLine( p1, p3, IM_COL32(0, 0, 0, 255), 1.0f ) ;
        draw_list->AddLine( p2, p3, IM_COL32(0, 0, 0, 255), 1.0f ) ;

        {
            ImVec2 const points[] = { p1, p2, p3 } ;
            draw_list->AddConvexPolyFilled( points, 3, IM_COL32(0, 0, 0, 255) ) ;
        }
    } ;
    
    auto draw_stop = [&]( ImVec2 const & pos, float_t const r )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList() ;
                
        ImVec2 const p2 = pos + ImVec2( 0.5f * r, 0.5f * r ) ; // top
        ImVec2 const p1 = pos + ImVec2( 0.5f * r, 1.5f * r ) ; // bottom
        ImVec2 const p3 = pos + ImVec2( 1.5f * r, 0.5f * r ) ; // top
        ImVec2 const p4 = pos + ImVec2( 1.5f * r, 1.5f * r ) ; // bottom

        {
            ImVec2 const points[] = { p1, p2, p3, p4 } ;
            draw_list->AddConvexPolyFilled( points, 4, IM_COL32(0, 0, 0, 255) ) ;
        }
    } ;

    auto draw_pause = [&]( ImVec2 const & pos, float_t const r )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList() ;

        // left
        {
            ImVec2 const p1 = pos + ImVec2( 0.4f * r, 1.5f * r ) ; // bottom
            ImVec2 const p2 = pos + ImVec2( 0.4f * r, 0.5f * r ) ; // top
            ImVec2 const p3 = pos + ImVec2( 0.8f * r, 0.5f * r ) ; // top
            ImVec2 const p4 = pos + ImVec2( 0.8f * r, 1.5f * r ) ; // bottom

            ImVec2 const points[] = { p1, p2, p3, p4 } ;
            draw_list->AddConvexPolyFilled( points, 4, IM_COL32(0, 0, 0, 255) ) ;
        }

        // right
        {
            ImVec2 const p1 = pos + ImVec2( 1.2f * r, 1.5f * r ) ; // bottom
            ImVec2 const p2 = pos + ImVec2( 1.2f * r, 0.5f * r ) ; // top
            ImVec2 const p3 = pos + ImVec2( 1.6f * r, 0.5f * r ) ; // top
            ImVec2 const p4 = pos + ImVec2( 1.6f * r, 1.5f * r ) ; // bottom

            ImVec2 const points[] = { p1, p2, p3, p4 } ;
            draw_list->AddConvexPolyFilled( points, 4, IM_COL32(0, 0, 0, 255) ) ;
        }
    } ;

    player_controller::player_state ret = player_controller::player_state::no_change ;
    float_t const r = ImGui::GetTextLineHeight() * 1.0f ;

    // prev button
    {
        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( "prev##" + label_, pos, r ) )
        {            
        }
        draw_prev( pos, r ) ;
    }

    ImGui::SameLine() ;

    if( !_internal_play )
    {
        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( "play##" + label_, pos, r ) )
        {
            _play = true ;
            _pause = false ;
            _internal_play = true ;
            ret = this_t::player_state::play ;
        }
        draw_play( pos, r ) ;
    }
    else
    {
        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( "pause##" + label_, pos, r ) )
        {
            _pause = true ;
            _play = false ;
            _internal_play = false ;
            ret = this_t::player_state::pause ;
        }
        draw_pause( pos, r ) ;
    }

    ImGui::SameLine() ;

    // next button
    {
        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( "next##" + label_, pos, r ) )
        { 
            //ret = this_t::player_state:: ;
        }
        draw_next( pos, r ) ;
    }

    ImGui::SameLine() ;

    // stop button
    {
        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( "stop##" + label_, pos, r ) )
        {
            _play = false ;
            _pause = false ;
            _internal_play = false ;
            ret = this_t::player_state::stop ;
        }
        draw_stop( pos, r ) ;
    }

    return ret ;
}

//****************************************************************
bool_t player_controller::is_play_clicked( void_t ) const noexcept 
{
    return _play ;
}

//****************************************************************
bool_t player_controller::is_pause_clicked( void_t ) const noexcept 
{
    return _pause ;
}


//****************************************************************