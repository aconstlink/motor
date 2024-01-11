#include "custom_widgets.h"

using namespace motor::tool ;

// original from https://github.com/ocornut/imgui/issues/942#issuecomment-268369298
bool custom_imgui_widgets::knob( const char* label, float* p_value, float v_min, float v_max ) noexcept
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();

    float radius_outer = 20.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);
    float line_height = ImGui::GetTextLineHeight();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    ImGui::InvisibleButton(label, ImVec2(radius_outer*2, radius_outer*2 + line_height + style.ItemInnerSpacing.y));
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.x != 0.0f)
    {
        float step = (v_max - v_min) / 200.0f;
        *p_value += io.MouseDelta.x * step;
        if (*p_value < v_min) *p_value = v_min;
        if (*p_value > v_max) *p_value = v_max;
        value_changed = true;
    }

    float t = (*p_value - v_min) / (v_max - v_min);
    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * t;
    float angle_cos = cosf(angle), angle_sin = sinf(angle);
    float radius_inner = radius_outer*0.40f;
    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(ImVec2(center.x + angle_cos*radius_inner, center.y + angle_sin*radius_inner), ImVec2(center.x + angle_cos*(radius_outer-2), center.y + angle_sin*(radius_outer-2)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%.3f", *p_value);
        ImGui::EndTooltip();
    }

    return value_changed;
}

bool_t custom_imgui_widgets::direction( char const * label, motor::math::vec2f_ref_t dir ) noexcept
{
    bool_t ret = false ;

    ImGuiIO& io = ImGui::GetIO() ;
    ImGuiStyle& style = ImGui::GetStyle() ;

    float radius_outer = 30.0f ;
    float radius_inner = radius_outer * 0.3f ;

    ImVec2 pos = ImGui::GetCursorScreenPos() ;
    ImVec2 center = ImVec2( pos.x + radius_outer, pos.y + radius_outer ) ;
    float line_height = ImGui::GetTextLineHeight() ;
    ImDrawList* draw_list = ImGui::GetWindowDrawList() ;

    ImGui::InvisibleButton( label, ImVec2( 
            radius_outer * 2, radius_outer * 2 + line_height + style.ItemInnerSpacing.y ) ) ;
                
    bool_t const is_active = ImGui::IsItemActive() ;
    bool_t const is_hovered = ImGui::IsItemHovered() ;

    if( is_active )
    {
        motor::math::vec2f_t const c( center.x, center.y ) ;
        motor::math::vec2f_t const m( io.MousePos.x, io.MousePos.y ) ;

        dir = (m - c).normalize() * motor::math::vec2f_t( 1.0f, -1.0f ) ;
        
        ret = true ;
    }

    // do the drawing
    {
        // outer circle
        draw_list->AddCircleFilled( center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16 ) ;

        // the line
        {
            auto const dirn = dir.normalized() * motor::math::vec2f_t( 1.0f, -1.0f ) ;

            auto const inner = motor::math::vec2f_t( center.x, center.y ) + dirn * (radius_inner + 0 ) ;
            auto const outer = motor::math::vec2f_t( center.x, center.y ) + dirn * (radius_outer - 2 ) ;

            draw_list->AddLine(
                ImVec2( inner.x(), inner.y() ), 
                ImVec2( outer.x(), outer.y() ),
                ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
        }

        // inner circle
        draw_list->AddCircleFilled( center, radius_inner, ImGui::GetColorU32( is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg ), 16 ) ;
    }

    return ret ;
}

void_t custom_imgui_widgets::text_overlay( char const * label, motor::string_cref_t text, int corner ) noexcept 
{
    this_t::overlay_begin( label, corner ) ;
    ImGui::Text(text.c_str());
    this_t::overlay_end() ;
}

bool_t custom_imgui_widgets::overlay_begin( char const * label, int corner ) noexcept 
{
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
    {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    bool_t open = true ;
    return ImGui::Begin(label, &open, window_flags) ;
}

void_t custom_imgui_widgets::overlay_end( void_t ) noexcept 
{
    ImGui::End();
}