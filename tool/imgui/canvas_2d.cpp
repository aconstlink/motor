
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "canvas_2d.h"

#include <motor/math/camera/3d/orthographic_projection.hpp>

#include <cmath>
#include <algorithm>

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
    assert( _in_section );

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
    assert( _in_section );

    auto const d = p - ( _start_screen_pos + _available_region * 0.5f );
    return _world_center + motor::math::vec2f_t( d.x, -d.y ) / _zoom;
}

//********************************************************************
motor::math::vec2f_t canvas_2d::measure_text_size(
    char const * text, float_t const font_height ) noexcept
{
    // Layout is in canvas units; only rendering depends on the zoom.
    ImVec2 const size = ImGui::GetFont()->CalcTextSizeA( font_height, FLT_MAX, 0.0f, text );
    return { size.x, size.y };
}

//********************************************************************
bool_t canvas_2d::begin( char const * const name, ImVec2 const & pos_in ) noexcept
{
    return this_t::begin( name, pos_in, {} );
}

//********************************************************************
bool_t canvas_2d::begin(
    char const * const name, ImVec2 const & pos_in, draw_options const & do_ ) noexcept
{
    if( _in_section ) return true;

    ImVec2 s = ImGui::GetContentRegionAvail();

    if( s.x <= 1.0f || s.y <= 1.0f ) return false;
    _in_section = true;

    _available_region = s;

    auto draw_list = ImGui::GetWindowDrawList();
    auto start = ImGui::GetCursorScreenPos();
    _start_screen_pos = start;

    ImGui::InvisibleButton( "##canvas", s, ImGuiButtonFlags_MouseButtonMiddle );
    // handle zoom and zoom to mouse
    {
        const float wheel = ImGui::GetIO().MouseWheel;
        if( ImGui::IsItemHovered() && wheel != 0.0f )
        {
            ImVec2 const mouse = ImGui::GetMousePos();
            auto const anchor = this_t::screen_to_world( mouse );
            _zoom = std::clamp( _zoom * std::pow( 1.15f, wheel ), 0.001f, 1000.0f );
            auto const after = this_t::screen_to_world( mouse );
            _world_center += anchor - after;
        }
    }

    // handle panning
    {
        if( ImGui::IsItemHovered() && ImGui::IsMouseDown( ImGuiMouseButton_Middle ) )
        {
            auto const mouse_world = this_t::screen_to_world( ImGui::GetMousePos() );
            auto const dif = mouse_world - _middle_mouse_pos;
            _world_center -= dif;
        }
        _middle_mouse_pos = this_t::screen_to_world( ImGui::GetMousePos() );
    }

    motor::math::vec2f_t const vd( s.x / _zoom, s.y / _zoom );
    ImVec4 outline_rect = ImVec4( start.x, start.y, s.x, s.y );

    // draw outline
    if( do_.draw_outline )
    {
        draw_list->AddLine(
            ImVec2( 0.0f, 0.0f ) + start, ImVec2( s.x, 0.0f ) + start, do_.grid_color );

        draw_list->AddLine(
            ImVec2( s.x, 0.0f ) + start, ImVec2( s.x, s.y ) + start, do_.grid_color );

        draw_list->AddLine(
            ImVec2( s.x, s.y ) + start, ImVec2( 0.0f, s.y ) + start, do_.grid_color );

        draw_list->AddLine(
            ImVec2( 0.0f, s.y ) + start, ImVec2( 0.0f, 0.0f ) + start, do_.grid_color );
    }

    // draw grid
    if( do_.draw_grid )
    {
        // smallest rounded world step with min. 50 pixel distance
        // between lines. step in the end determines how much apart
        // the lines are drawn. this is now adaptive based on zoom level.
        float_t const wanted = 50.0f / _zoom;
        float_t const decade = std::pow( 10.0f, std::floor( std::log10( wanted ) ) );
        float_t const rel = wanted / decade;
        float_t const step = decade * ( rel <= 1 ? 1 : rel <= 2 ? 2 : rel <= 5 ? 5 : 10 );

        float_t const line_column_width = step;
        float_t const line_row_height = step;

        size_t const num_vert = std::ceil( vd.x() / line_column_width );
        size_t const num_horz = std::ceil( vd.y() / line_row_height );

        float_t const width = vd.x();
        float_t const height = vd.y();

        float_t const x_start =
            line_column_width * ( std::floor( _world_center.x() / line_column_width ) -
                                    std::floor( width * 0.5f / line_column_width ) - 1.0f );

        float_t const y_start =
            std::floor( _world_center.y() / line_row_height ) * line_row_height -
            std::floor( ( height * 0.5f ) / line_row_height ) * line_row_height - line_row_height;

        // draw vertical lines
        {
            for( size_t x = 0; x <= num_vert; ++x )
            {
                motor::math::vec2f_t const v0(
                    x_start + float_t( x ) * line_column_width, y_start );
                motor::math::vec2f_t const v1( x_start + float_t( x ) * line_column_width,
                    y_start + height + 2.0f * line_row_height );

                auto const s_v0 = this_t::world_to_screen( v0 );
                auto const s_v1 = this_t::world_to_screen( v1 );

                draw_list->AddLine( s_v0, s_v1, do_.grid_color );

                int_t const gx = std::floor( v0.x() / line_column_width );
                if( do_.draw_grid_coords && ( std::abs( gx ) % do_.draw_grid_coords_every ) == 0 )
                {
                    char text[ 256 ];
                    std::snprintf( text, sizeof( text ), "%.0f", v0.x() );
                    auto const ts = ImGui::CalcTextSize( text );

                    ImVec2 p = this_t::world_to_screen( v0 );
                    // p.x = std::min( std::max( p.x, start.x ), s.x );
                    p.x -= ts.x * 0.5f;
                    p.y = s.y - ts.y + start.y;

                    draw_list->AddText( p, do_.grid_color, text );
                }
            }
        }

        // draw horizontal lines
        {
            for( size_t y = 0; y <= num_horz; ++y )
            {
                motor::math::vec2f_t const v0( x_start, y_start + float_t( y ) * line_row_height );
                motor::math::vec2f_t const v1( x_start + width + 2.0f * line_column_width,
                    y_start + float_t( y ) * line_row_height );

                auto const s_v0 = this_t::world_to_screen( v0 );
                auto const s_v1 = this_t::world_to_screen( v1 );

                draw_list->AddLine( s_v0, s_v1, do_.grid_color );

                int_t const gx = std::floor( v0.y() / line_row_height );
                if( do_.draw_grid_coords && ( std::abs( gx ) % do_.draw_grid_coords_every ) == 0 )
                {
                    char text[ 256 ];
                    std::snprintf( text, sizeof( text ), "%.0f", v0.y() );
                    auto const ts = ImGui::CalcTextSize( text );

                    ImVec2 p = this_t::world_to_screen( v0 );

                    p.x = start.x;
                    p.y -= ts.y;

                    draw_list->AddText( p, do_.grid_color, text );
                }
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
void_t canvas_2d::focus_via_dims(
    motor::math::vec2f_in_t pos, motor::math::vec2f_in_t dims ) noexcept
{
    auto const vr = motor::tool::imgui::convert::from( _available_region );
    if( vr.x() <= 0.0f || vr.y() <= 0.0f ) return;

    _world_center = pos;
    auto const zoom_x = vr.x() / std::max( dims.x() * 1.15f, 0.001f );
    auto const zoom_y = vr.y() / std::max( dims.y() * 1.15f, 0.001f );
    _zoom = std::clamp( std::min( zoom_x, zoom_y ), 0.001f, 1000.0f );
}

//********************************************************************
void_t canvas_2d::focus_via_rect(
    motor::math::vec2f_in_t min, motor::math::vec2f_in_t max ) noexcept
{
    auto const pos = ( max + min ) * 0.5f;
    auto const size = ( max - min ).abs();

    this_t::focus_via_dims( pos, size );
}

//********************************************************************
void_t canvas_2d::draw_line( this_t::draw_line_info const & dl ) noexcept
{
    auto draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine( this_t::world_to_screen( motor::math::vec2f_t( dl.start ) ),
        this_t::world_to_screen( dl.end ), dl.color );
}

//********************************************************************
void_t canvas_2d::draw_point( draw_point_info const & info ) noexcept
{
    auto dl = ImGui::GetWindowDrawList();
    dl->AddCircleFilled( this_t::world_to_screen( info.point ), 2.0f, info.color );
}

//********************************************************************
void_t canvas_2d::draw_circle( draw_circle_info const & info ) noexcept
{
    auto dl = ImGui::GetWindowDrawList();
    if( info.filled )
        dl->AddCircleFilled(
            this_t::world_to_screen( info.point ), info.radius * _zoom, info.color );
    else
        dl->AddCircle( this_t::world_to_screen( info.point ), info.radius * _zoom, info.color );
}

//********************************************************************
void_t canvas_2d::draw_poly_line( poly_line_info const & info ) noexcept
{

    auto dl = ImGui::GetWindowDrawList();

    if( _point_cache.size() < info.points.size() ) _point_cache.resize( info.points.size() );

    size_t i = size_t( -1 );
    for( auto const & p : info.points )
    {
        _point_cache[ ++i ] = this_t::world_to_screen( p );
    }

    dl->AddPolyline( _point_cache.data(), info.points.size(), info.color, info.thickness );
}

//********************************************************************
void_t canvas_2d::draw_polygon( polygon_info const & info ) noexcept
{
    auto const n = int( info.points.size() );
    if( n < 3 ) return;

    _point_cache.resize( n );
    for( int i = 0; i < n; ++i ) _point_cache[ i ] = world_to_screen( info.points[ i ] );

    auto * dl = ImGui::GetWindowDrawList();
    dl->AddConvexPolyFilled( _point_cache.data(), n, info.fill_color );
    if( info.outline_width > 0.0f )
        dl->AddPolyline(
            _point_cache.data(), n, info.outline_color, info.outline_width, ImDrawFlags_Closed );
}

//********************************************************************
void_t canvas_2d::draw_rect_filled( rect_filled_info const & info ) noexcept
{
    auto * dl = ImGui::GetWindowDrawList();

    auto const p0 = this_t::world_to_screen( info.pos );
    auto const p1 = this_t::world_to_screen( info.pos + info.size );

    ImVec2 const pmin = { p0.x, p1.y };
    ImVec2 const pmax = { p1.x, p0.y };

    dl->AddRectFilled( pmin, pmax, info.color, info.radius * _zoom, info.flags );
}

//********************************************************************
void_t canvas_2d::draw_rect( rect_info const & info ) noexcept
{
    auto * dl = ImGui::GetWindowDrawList();

    auto const bl = this_t::world_to_screen( info.pos );
    auto const tr = this_t::world_to_screen( info.pos + info.size );

    dl->AddRect( { bl.x, tr.y }, { tr.x, bl.y }, info.color, info.radius * _zoom,
        info.thickness * _zoom, info.flags );
}

//********************************************************************
void_t canvas_2d::draw_text( text_info const & info ) noexcept
{
    auto * dl = ImGui::GetWindowDrawList();

    auto const p0 = this_t::world_to_screen( info.pos );
    float_t const pad = 8.0f * _zoom;
    float_t const height = info.font_height * _zoom;

    dl->AddText( ImGui::GetFont(), height, { p0.x, p0.y - height }, info.color, info.title );
}

//********************************************************************
void_t canvas_2d::draw_bezier_curve( bezier_curve_info const & info )
{
    auto * dl = ImGui::GetWindowDrawList();

    auto const p0 = this_t::world_to_screen( info.cp[ 0 ] );
    auto const p1 = this_t::world_to_screen( info.cp[ 1 ] );
    auto const p2 = this_t::world_to_screen( info.cp[ 2 ] );
    auto const p3 = this_t::world_to_screen( info.cp[ 3 ] );

    dl->AddBezierCubic( p0, p1, p2, p3, info.color, info.thickness * _zoom, info.num_segments );
}

//********************************************************************
void_t canvas_2d::push_clip_rect( clip_rect_info const & info ) noexcept
{
    auto * dl = ImGui::GetWindowDrawList();

    auto const bottom_left = this_t::world_to_screen( info.pos );
    auto const top_right = this_t::world_to_screen( info.pos + info.size );

    dl->PushClipRect( { bottom_left.x, top_right.y }, { top_right.x, bottom_left.y }, true );
}

//********************************************************************
void_t canvas_2d::pop_clip_rect( void_t ) noexcept
{
    auto * dl = ImGui::GetWindowDrawList();
    dl->PopClipRect();
}