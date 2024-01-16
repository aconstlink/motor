
#include "timeline.h"

#include <sstream>
#include <iomanip>

using namespace motor::tool ;

static auto tri_down = []( ImVec2 const pos, float_t const height, ImU32 const color, ImDrawList * draw_list )
{
    float_t const h = motor::math::fn<float_t>::ceil( height * 0.5f ) ;

    ImVec2 const points[] = 
    {
        pos + ImVec2(0.0f, 0.0f), pos + ImVec2( -h, -h ), pos + ImVec2( h, -h) 
    } ;

    draw_list->AddConvexPolyFilled( points, 3, color ) ;
} ;

static auto tri_up = []( ImVec2 const pos, float_t const height, ImU32 const color, ImDrawList * draw_list )
{
    float_t const h = motor::math::fn<float_t>::ceil( height * 0.5f ) ;

    ImVec2 const points[] = 
    {
        pos, pos + ImVec2( h, h ), pos + ImVec2( -h, h) 
    } ;

    draw_list->AddConvexPolyFilled( points, 3, color ) ;
} ;

size_t timeline::_label_counter = 0 ;

size_t timeline::inc_label_counter( void_t ) noexcept 
{
    static motor::concurrent::mutex_t __mtx ;
    motor::concurrent::lock_guard_t lk( __mtx ) ; 
    return _label_counter++ ;
}

timeline::timeline( void_t ) noexcept 
{
    auto const c = this_t::inc_label_counter() ;
    _label = "unkown_timeline_" + motor::to_string( c ) ;
}

//***************************************************************
timeline::timeline( motor::string_in_t label ) noexcept 
{
    _label = label + "_" + motor::to_string( this_t::inc_label_counter() ) ;
}

//***************************************************************
timeline::timeline( this_rref_t rhv ) noexcept  
{
    _label = std::move( rhv._label ) ;
}

//***************************************************************
timeline::~timeline( void_t ) noexcept 
{
}

//***************************************************************
timeline::this_ref_t timeline::operator = ( this_cref_t ) noexcept 
{
    return *this ;
}

//***************************************************************
timeline::this_ref_t timeline::operator = ( this_rref_t ) noexcept 
{
    return *this ;
}

//***************************************************************
bool_t timeline::begin( motor::tool::time_info_ref_t ti ) noexcept 
{
    if( _begin ) return false ;
    _begin = true ;

    if( ti.max_milli == 0 ) ti.max_milli = 10000 ;

    _max_milli = ti.max_milli ;

    if( _zoom == 0 )
    {
        _zoom = std::max( size_t( 1 ), size_t( _max_milli / size_t( ImGui::GetContentRegionAvail().x ) ) ) ;
    }

    float_t const top_line_height = 10.0f ;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    //ImVec2 const scrolling_child_size = ImVec2( 0, ImGui::GetContentRegionAvail().y ) - ImVec2(0.0f, ImGui::GetTextLineHeight()+10) ;
    ImGui::BeginChild((_label + "##timeline").c_str(), ImVec2(0,100), true, ImGuiWindowFlags_HorizontalScrollbar) ;

    auto draw_list = ImGui::GetWindowDrawList() ;

    ImVec2 const capture_pos = ImGui::GetCursorScreenPos() ;
    float_t const height = ImGui::GetContentRegionAvail().y ;
    size_t const big_line_height = height * 0.5f ;
    size_t const small_line_height = height * 0.25f ;

    // set markers in order to have the horizontal scroll bar visible and scrollable
    {
        {
            ImVec2 pos = ImGui::GetCursorScreenPos() ;
            draw_list->AddLine( pos + ImVec2(0.0f,10.0f), pos + ImVec2( 0.0f,10.0f ), IM_COL32(255, 255, 255, 0), 1.0f ) ;
        }

        ImGui::SetCursorScreenPos( ImGui::GetCursorScreenPos() + ImVec2( this_t::milli_to_pixel(ti.max_milli), 0.0f ) ) ;
                     
        {
            ImVec2 pos = ImGui::GetCursorScreenPos() ;
            draw_list->AddLine( pos + ImVec2(0.0f,0.0f), pos + ImVec2( 0.0f,10.0f ), IM_COL32(255, 255, 255, 255), 10.0f ) ;
        }
        ImGui::Dummy(ImVec2(0, 10));

        ImGui::SetCursorScreenPos( capture_pos ) ;
    }
    
    float_t const scroll_max_x = ImGui::GetScrollMaxX() ;
    float_t const scroll_x = ImGui::GetScrollX() ;
    
    float_t const mouse_x = 
        std::max( 0.0f, std::min( ImGui::GetMousePos().x - ImGui::GetWindowPos().x + scroll_x, float_t( this_t::milli_to_pixel(ti.max_milli) ) ) ) ;

    //motor::log::global_t::status( motor::to_string(mouse_x) ) ;

    bool_t const mouse_in_cr = ImGui::IsMouseHoveringRect( 
        ImVec2( ImGui::GetScrollX(), 0.0f) + ImGui::GetCursorScreenPos(), 
        ImVec2( ImGui::GetScrollX(), 0.0f) + ImGui::GetCursorScreenPos()+ImGui::GetContentRegionAvail() ) && ImGui::IsWindowHovered() ; 

    if( ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl) && mouse_in_cr )
    {
        auto const f = ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift) ? 10.0f : 1.0f ;
        auto const mw = ImGui::GetIO().MouseWheel ;
        int_t tmp = int_t( -mw * f ) ;
        if( tmp < 0 && _zoom >= size_t( std::abs( tmp ) ) )
        {
            _zoom += tmp ;
            _zoom = std::max( size_t( 10 ), _zoom ) ;
        }
        else
        {
            _zoom += tmp ;
            _zoom = std::min( size_t( 100000 ), _zoom ) ;
        }

        if( tmp != 0 ) ImGui::SetScrollX( std::max( mouse_x - ImGui::GetContentRegionAvail().x * 0.5f, 0.0f ) ) ;
    }

    ImGui::SetCursorScreenPos( capture_pos ) ;

    // draw time line
    {
        float_t const small = 10.0f ;
        size_t const steps = 5 ;
        float_t const big = small * float_t(steps) ;

        float_t const offset_x = motor::math::fn<float_t>::fract( scroll_x / big ) * big ;
        ImVec2 const start = ImGui::GetCursorScreenPos() + ImVec2( scroll_x - offset_x, top_line_height ) ;

        // draw small lines
        {
            size_t const num_steps = size_t( ImGui::GetContentRegionAvail().x / small ) + steps ;
            for( size_t i=0; i<num_steps; ++i )
            {
                ImVec2 const p0 = start + ImVec2( small * float_t(i), 0.0f ) ;
                ImVec2 const p1 = start + ImVec2( small * float_t(i), small_line_height ) ;
                draw_list->AddLine( p0, p1, IM_COL32(150,150,150, 255), 1.0f ) ;
            }
        }

        ImGui::SetCursorScreenPos( capture_pos ) ;

        // draw big lines
        #if 1
        {
            size_t const the_big = size_t( scroll_x ) / size_t( big ) ;
            size_t const num_steps = size_t( ImGui::GetContentRegionAvail().x / big ) + 2 ;
            // need to draw one less (-1), otherwise, a infinite scroll appears
            for( size_t i=0; i<num_steps-1; ++i )
            {
                ImVec2 const p0 = start + ImVec2( big * float_t(i), 0.0f ) ;
                ImVec2 const p1 = start + ImVec2( big * float_t(i), big_line_height ) ;
                draw_list->AddLine( p0, p1, IM_COL32(200,200,200, 255), 1.0f ) ;
         
                size_t const cur_milli = this_t::pixel_to_milli( (the_big + i) * size_t(big) );

                #if 1
                ImGui::SetCursorScreenPos( p0 + ImVec2(2.0f, height * 0.25f) + ImVec2(0.0f, ((the_big+i)%2)*ImGui::GetTextLineHeight()*0.5) ) ;
                ImGui::SetWindowFontScale( 1.0f ) ;
                ImGui::Text( this_t::make_time_string2( cur_milli ).c_str() ) ;
                ImGui::SetWindowFontScale( 1.0f ) ;
                ImGui::SameLine() ;
                #endif
            }
        }
        #endif
    }

    ImGui::SetCursorScreenPos( capture_pos ) ;

    // line moving with mouse : hover
    if( mouse_in_cr )
    {
        ImGui::SetCursorScreenPos( ImGui::GetCursorScreenPos() + ImVec2( mouse_x, 0.0f ) ) ;

        ImVec2 const pos = ImGui::GetCursorScreenPos() ;

        {
            ImVec2 const p0 = pos ; 
            ImVec2 const p1 = p0 + ImVec2( 0.0f, top_line_height ) ;
            draw_list->AddLine( p0, p1, IM_COL32(255, 255, 0, 255), 1.0f ) ;

            _hover = this_t::pixel_to_milli( size_t( mouse_x ) ) ;
        }
    }
    else
    {
        _hover = size_t( -1 ) ;
    }

    ImGui::SetCursorScreenPos( capture_pos ) ;

    // current position marker
    {
        ImVec2 const pos = ImGui::GetCursorScreenPos() + ImVec2(0.0f, top_line_height) ;

        size_t cur_milli = ti.cur_milli ;

        if( ImGui::IsMouseDown(ImGuiMouseButton_Left) && mouse_in_cr )
        {
            cur_milli = _hover ;
        }

        size_t const milli_pos = cur_milli ;
        size_t const marker_pos = this_t::milli_to_pixel( milli_pos ) ;
        ImVec2 const avail = ImGui::GetContentRegionAvail() ;
        if( marker_pos > scroll_x && marker_pos < scroll_x + avail.x )
        {
            ImVec2 const p0 = pos + ImVec2( marker_pos, 0.0f ) ;
            ImVec2 const p1 = pos + ImVec2( marker_pos, big_line_height + ImGui::GetTextLineHeight() ) ;

            //tri_down( p0, top_line_height*0.5f, IM_COL32(0, 255, 0, 255), draw_list ) ;
            draw_list->AddLine( p0, p1, IM_COL32(0, 255, 0, 255), 1.0f ) ;
            tri_up( p1, top_line_height, IM_COL32(0, 255, 0, 255), draw_list ) ;
        }

        _play = cur_milli ;
        ti.cur_milli = cur_milli ;
    }

    // move timeline with player marker
    if( _lock_player )
    {
        auto const p0 = this_t::milli_to_pixel( _play ) - ImGui::GetContentRegionAvail().x*0.5f ;
        auto const p = std::max( p0, 0.0f ) ;
        ImGui::SetScrollX( p ) ;
    }

    ImGui::SetCursorScreenPos( capture_pos ) ;

    {
        if( ImGui::GetIO().MouseDown[1] && mouse_in_cr )
        {
            ImVec2 cm = ImGui::GetMousePos() ;
            float_t const d = (cm.x - _old_mouse_pos.x) ;
            float_t const x = std::min( ImGui::GetScrollX() + d, ImGui::GetScrollMaxX() ) ;
            ImGui::SetScrollX( x ) ;
        }
        _old_mouse_pos = ImGui::GetMousePos() ;
    }

    // testing a button within the timeline
    #if 0
    {
        ImVec2 const avail = ImGui::GetContentRegionAvail() ;
        ImGui::SetCursorScreenPos( ImGui::GetCursorScreenPos() + ImVec2( avail.x-100.0f, 0.0f ) ) ;

        ImGui::Button("test") ;

    }
    #endif 

    ImGui::SetCursorScreenPos( capture_pos ) ;
    
    return true ;
}

//***************************************************************
void_t timeline::end( void_t ) noexcept 
{
    ImGui::PopStyleVar() ;
    ImGui::EndChild();
    
    // play
    {
        motor::string_t const s = this_t::make_time_string( _play ) ;
        ImGui::TextColored( ImVec4(0.0f, 1.0f, 0.0f, 1.0f), s.c_str() ) ;
    }

    ImGui::SameLine() ;

    // hover
    {
        if( _hover != size_t( -1 ) )
        {
            motor::string_t const s = this_t::make_time_string( _hover ) ;
            ImGui::TextColored( ImVec4(1.0f, 1.0f, 0.0f, 1.0f), s.c_str() ) ;
        }
        else
        {
            ImGui::TextColored( ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "--:--:---" ) ;
        }
        
    }

    ImGui::SameLine() ;

    // max milli
    {
        motor::string_t const s = this_t::make_time_string( _max_milli ) ;
        ImGui::TextColored( ImVec4(1.0f, 0.0f, 0.0f, 1.0f), s.c_str() ) ;
    }
    
    ImGui::SameLine() ;

    {
        motor::string_t const s = motor::to_string( _zoom ) ;
        ImGui::Text( s.c_str() ) ;
    }

    ImGui::SameLine() ;

    {
        ImGui::Checkbox( ("lock##timeline" + _label).c_str(), &_lock_player ) ;
    }

    _begin = false ;
}

//***************************************************************
size_t timeline::milli_to_pixel( size_t const milli ) const noexcept 
{
    return milli / _zoom ;
}

//***************************************************************
size_t timeline::pixel_to_milli( size_t const pixel ) const noexcept 
{
    return pixel * _zoom ;
}

//***************************************************************
motor::string_t timeline::make_time_string( size_t const milli ) const noexcept 
{
    motor::string_t s ;

    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << milli / 60000 ;
        s += ss.str();
    }
    s += ":" ;
    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << (milli / 1000) % 60 ;
        s += ss.str();
    }
    s += ":" ;
    {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0') << milli % 1000 ;
        s += ss.str();
    }

    return s ;
}

//***************************************************************
motor::string_t timeline::make_time_string2( size_t const milli ) const noexcept 
{
    motor::string_t s ;

    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << milli / 60000 ;
        s += ss.str();
    }
    s += ":" ;
    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << (milli / 1000) % 60 ;
        s += ss.str();
    }

    return s ;
}