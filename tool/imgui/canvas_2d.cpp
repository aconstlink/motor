
#include "canvas_2d.h"

#include <motor/math/camera/3d/orthographic_projection.hpp>

#include <cmath>

using namespace motor::tool::imgui;

//********************************************************************
canvas_2d::canvas_2d( void_t ) noexcept {}

//********************************************************************
canvas_2d::canvas_2d( this_rref_t ) noexcept {}

//********************************************************************
canvas_2d::~canvas_2d( void_t ) noexcept {}

//********************************************************************
ImVec2 canvas_2d::world_to_screen( motor::math::vec2f_cref_t const & v ) noexcept
{
    // #1 : transform to world center and zoom
    auto const v0 = ( v - _world_center ) * _zoom;
    // #2 : flipp y coordinate
    auto const v1 = motor::math::vec2f_t( v0.x(), -v0.y() );
    // #3 : return imgui screen space coordinate
    return convert::from( v1 ) + _start_screen_pos + _available_region * 0.5f;
}

//********************************************************************
motor::math::vec2f_t canvas_2d::screen_to_world( ImVec2 const & p ) noexcept
{
    auto const d = p - ( _start_screen_pos + _available_region * 0.5f );
    return _world_center + motor::math::vec2f_t( d.x, -d.y ) / _zoom;
}

//********************************************************************
bool_t canvas_2d::begin( char const * const name, ImVec2 const & ) noexcept
{
    if( _in_section ) return true;

    _in_section = true;

    ImVec2 s = ImGui::GetContentRegionAvail();
    _available_region = s;

    motor::math::vec2f_t const vd( s.x / _zoom, s.y / _zoom );

    auto draw_list = ImGui::GetWindowDrawList();
    auto start = ImGui::GetCursorScreenPos();
    _start_screen_pos = start;

    ImGui::InvisibleButton( "##canvas", s, ImGuiButtonFlags_MouseButtonMiddle );

    const float wheel = ImGui::GetIO().MouseWheel;
    if( ImGui::IsItemHovered() && wheel != 0.0f )
    {
        // Positiv: reinzoomen. Negativ: rauszoomen.
        // zoom_at( ImGui::GetMousePos(), std::pow( 1.15f, wheel ) );

        _zoom += motor::math::fn< float_t >::sign( wheel ) * 0.1f; // std::pow( 1.0005f, wheel );
        _zoom = std::max( 0.001f, _zoom );
    }

    if( ImGui::IsMouseDown( ImGuiMouseButton_Middle ) )
    {
        if( !_middle_mouse_tracked )
        {
            _middle_mouse_pos = ImGui::GetMousePos();
            _middle_mouse_tracked = true;
        }

        auto const dif =
            motor::tool::imgui::convert::from( ImGui::GetMousePos() - _middle_mouse_pos );
        _world_center -= dif * motor::math::vec2f_t( 1.0f, -1.0f );
        _middle_mouse_pos = ImGui::GetMousePos();
    }
    else
    {
        _middle_mouse_tracked = false;
    }

    // draw outline
    {
        draw_list->AddLine( ImVec2( 0.0f, 0.0f ) + start, ImVec2( s.x, 0.0f ) + start,
            ImColor( 0.5f, 0.5f, 0.5f ) );

        draw_list->AddLine(
            ImVec2( s.x, 0.0f ) + start, ImVec2( s.x, s.y ) + start, ImColor( 0.5f, 0.5f, 0.5f ) );

        draw_list->AddLine(
            ImVec2( s.x, s.y ) + start, ImVec2( 0.0f, s.y ) + start, ImColor( 0.5f, 0.5f, 0.5f ) );

        draw_list->AddLine( ImVec2( 0.0f, s.y ) + start, ImVec2( 0.0f, 0.0f ) + start,
            ImColor( 0.5f, 0.5f, 0.5f ) );
    }

    // draw grid
    {
        float_t const line_column_width = 50.0f / _zoom;
        float_t const line_row_height = 50.0f / _zoom;

        size_t const num_vert = std::ceil( vd.x() / line_column_width );
        size_t const num_horz = std::ceil( vd.y() / line_row_height );

        float_t const width = vd.x();
        float_t const height = vd.y();

        float_t const x_start =
            std::floor( _world_center.x() / line_column_width ) *
            (line_column_width)-std::floor( ( width * 0.5f ) / line_column_width ) *
            (line_column_width)-line_column_width;

        float_t const y_start =
            std::floor( _world_center.y() / line_row_height ) * line_row_height -
            std::floor( ( height * 0.5f ) / line_row_height ) * line_row_height;

        {
            for( size_t x = 0; x <= num_vert; ++x )
            {
                motor::math::vec2f_t const v0(
                    x_start + float_t( x ) * line_column_width, y_start );
                motor::math::vec2f_t const v1( x_start + float_t( x ) * line_column_width,
                    y_start + height + 2.0f * line_row_height );

                draw_list->AddLine( this_t::world_to_screen( v0 ), this_t::world_to_screen( v1 ),
                    ImColor( 0.5f, 0.5f, 0.5f ) );
            }
        }

        {
            for( size_t y = 0; y <= num_horz; ++y )
            {
                motor::math::vec2f_t const v0( x_start, y_start + float_t( y ) * line_row_height );
                motor::math::vec2f_t const v1( x_start + width + 2.0f * line_column_width,
                    y_start + float_t( y ) * line_row_height );

                draw_list->AddLine( this_t::world_to_screen( v0 ), this_t::world_to_screen( v1 ),
                    ImColor( 0.5f, 0.5f, 0.5f ) );
            }
        }
    }
    return true;
}

//********************************************************************
void_t canvas_2d::end( void_t ) noexcept
{
    assert( _in_section );
    if( !_in_section ) return;

    // Optional: diesen Block mit _show_debug schaltbar machen.
    {
        ImVec2 const lo = _start_screen_pos;
        ImVec2 const hi = lo + _available_region;
        ImVec2 const mouse = ImGui::GetMousePos();
        bool const inside = ImGui::IsMouseHoveringRect( lo, hi );

        auto const size = convert::from( _available_region ) / _zoom;
        char cursor[ 80 ] = "outside";
        if( inside )
        {
            auto const p = this_t::screen_to_world( mouse );
            std::snprintf( cursor, sizeof( cursor ), "%.2f, %.2f", p.x(), p.y() );
        }

        char text[ 256 ];
        std::snprintf( text, sizeof( text ),
            "Zoom    %.3fx\n"
            "Center  %.2f, %.2f\n"
            "View    %.1f x %.1f world\n"
            "Grid    %.2f world / 50 px\n"
            "Mouse   %s",
            _zoom, _world_center.x(), _world_center.y(), size.x(), size.y(), 50.0f / _zoom,
            cursor );

        auto * dl = ImGui::GetWindowDrawList();
        ImVec2 const pos = lo + ImVec2( 8, 8 );
        ImVec2 const pad( 7, 5 );
        ImVec2 const label = pos + pad;

        dl->PushClipRect( lo, hi, true );
        dl->AddRectFilled(
            pos, label + ImGui::CalcTextSize( text ) + pad, IM_COL32( 16, 18, 22, 220 ), 3.0f );
        dl->AddText( label, IM_COL32( 230, 234, 240, 255 ), text );
        dl->PopClipRect();
    }

    _in_section = false;
}

//********************************************************************
void_t canvas_2d::draw_line( this_t::draw_line_info const & dl ) noexcept
{
    auto draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine( this_t::world_to_screen( motor::math::vec2f_t( dl.start ) ),
        this_t::world_to_screen( dl.end ), ImColor( 1.0f, 1.0f, 1.0f ) );
}