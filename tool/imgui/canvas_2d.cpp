
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
motor::math::vec2f_t canvas_2d::screen_to_world( ImVec2 const & ) noexcept
{
    return motor::math::vec2f_t();
}

//********************************************************************
bool_t canvas_2d::begin( char const * const name, ImVec2 const & ) noexcept
{
    if( _in_section ) return true;

    _in_section = true;

    ImVec2 s = ImGui::GetContentRegionAvail();
    _available_region = s;

    motor::math::vec2f_t const vd( s.x, s.y );

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

// draw test
#if 0
    {
        motor::math::vec2f_t draw_center( 60.0f, 10.0f );
        {
            motor::math::vec2f_t p0;
            ImVec2 c = trafo_fn( p0, draw_center, _zoom, start, s );
            draw_list->AddCircle( c, 10.0f, 0xff0000ff );
        }
        {
            motor::math::vec2f_t p0( 40.0f );
            ImVec2 c = trafo_fn( p0, draw_center, _zoom, start, s );
            draw_list->AddCircle( c, 10.0f, 0xffffffff );
        }

        {
            motor::math::vec2f_t p0( -40.0f, 40.0f );
            ImVec2 c = trafo_fn( p0, draw_center, _zoom, start, s );
            draw_list->AddCircle( c, 10.0f, 0xffffffff );
        }

        {
            motor::math::vec2f_t p0( 100.0f, 100.0f );
            ImVec2 c = trafo_fn( p0, draw_center, _zoom, start, s );
            draw_list->AddCircle( c, 10.0f, ImColor( 0.0f, 1.0f, 1.0f ) );
        }
    }
#endif

    // draw outlice
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
        float_t const line_column_width = 50.0f;
        float_t const light_row_height = 50.0f;

        size_t const num_vert = std::ceil( vd.x() / line_column_width );
        size_t const num_horz = std::ceil( vd.y() / light_row_height );

        float_t const width = vd.x();
        float_t const height = vd.y();

        {
            float_t const x_start =
                std::floor( _world_center.x() / line_column_width ) * line_column_width -
                std::floor( ( width * 0.5f ) / line_column_width ) * line_column_width;

            for( size_t x = 0; x < num_vert; ++x )
            {
                motor::math::vec2f_t const v0(
                    x_start + float_t( x ) * line_column_width, -height * 0.5f );
                motor::math::vec2f_t const v1(
                    x_start + float_t( x ) * line_column_width, +height * 0.5f );

                draw_list->AddLine( this_t::world_to_screen( v0 ), this_t::world_to_screen( v1 ),
                    ImColor( 0.5f, 0.5f, 0.5f ) );
            }
        }

        {
            for( size_t y = 0; y < num_horz; ++y )
        {
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

    _in_section = false;
}

//********************************************************************
void_t canvas_2d::draw_line( this_t::draw_line_info const & dl ) noexcept
{
    auto draw_list = ImGui::GetWindowDrawList();

    draw_list->AddLine( this_t::world_to_screen( motor::math::vec2f_t( dl.start ) ),
        this_t::world_to_screen( dl.end ), ImColor( 1.0f, 1.0f, 1.0f ) );
}