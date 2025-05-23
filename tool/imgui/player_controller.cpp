
#include "player_controller.h"

#include <imgui_internal.h>

using namespace motor::tool ;

//****************************************************************
player_controller::player_state player_controller::do_tool( char const * label_, bool_t const play_toggle ) noexcept 
{
    auto circle_button = [&]( char const * l, ImVec2 const & pos, float_t const r ) -> bool_t
    {
        ImGuiContext& ctx = *ImGui::GetCurrentContext() ;
        const ImGuiStyle& style = ctx.Style;

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false ;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const ImGuiID id = window->GetID( l ) ;
        
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

    bool_t click_play_pause = false ;
    bool_t click_stop = false ;

    if( play_toggle )
    {
       auto const s = this_t::get_state() ;
       switch( s )
       {
       case this_t::player_state::play:
           click_play_pause = true ;
           break ;

        case this_t::player_state::pause:
        case this_t::player_state::stop:
            click_play_pause = true ;
           break ;
       }
    }

    // prev button
    {
        char buffer[1024] ;
        std::snprintf( buffer, 1024, "prev##%s", label_  ) ;

        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( buffer, pos, r ) )
        { 
        }
        draw_prev( pos, r ) ;
    }

    ImGui::SameLine() ;

    if( !_internal_play )
    {
        char buffer[1024] ;
        std::snprintf( buffer, 1024, "play##%s", label_  ) ;

        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        bool_t const pressed = circle_button( buffer, pos, r ) ; 
        if( pressed || click_play_pause )
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
        char buffer[1024] ;
        std::snprintf( buffer, 1024, "pause##%s", label_ ) ;

        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        auto const pressed = circle_button( buffer, pos, r ) ;
        if( pressed || click_play_pause )
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
        char buffer[1024] ;
        std::snprintf( buffer, 1024, "next##%s", label_ ) ;

        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( buffer, pos, r ) )
        { 
            //ret = this_t::player_state:: ;
        }
        draw_next( pos, r ) ;
    }

    ImGui::SameLine() ;

    // stop button
    {
        char buffer[1024] ;
        std::snprintf( buffer, 1024, "stop##%s", label_ ) ;

        ImVec2 const pos = ImGui::GetCursorScreenPos() ;
        if( circle_button( buffer, pos, r ) || click_stop )
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