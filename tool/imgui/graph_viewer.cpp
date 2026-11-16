// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "graph_viewer.h"

using namespace motor::tool::imgui;

//********************************************************************
void_t graph_viewer::draw_node( canvas2d_t & can, node_info const & info ) noexcept
{
    float_t const font_scale = 0.8f ;
    float_t const header_h = 24.0f; //_header_height * _zoom;
    float_t const pad = 8.0f;       //* _zoom;

    auto const title_size = can.measure_text_size( info.title, header_h * font_scale );
    auto size = info.size;

    if( info.size.x() <= title_size.x() )
    {
        size.x( title_size.x() + 2.0f * pad );
    }

    float_t const body_h = size.y() - header_h;

    float_t const radius = 6.0f;

    // draw header
    can.draw_rect_filled( { { info.pos.x(), info.pos.y() + body_h }, { size.x(), header_h },
        info.header_color, 6.0f, ImDrawFlags_RoundTop } );

    // draw body
    can.draw_rect_filled(
        { info.pos, { size.x(), body_h }, info.body_color, radius, ImDrawFlags_RoundBottom } );

    can.push_clip_rect(
        { .pos = info.pos + motor::math::vec2f_t( pad, body_h ), .size = title_size } );

    can.draw_text( { .title = info.title,
        .pos = info.pos + motor::math::vec2f_t( pad, body_h ),
        .font_height = int_t( std::floor( header_h * font_scale ) ),
        .color = info.text_color } );

    can.pop_clip_rect();

    can.draw_rect(
        { .pos = info.pos, .size = size, .color = info.border_color, .radius = radius } );
#if 0
   

    float_t const font_h = ImGui::GetFontSize() * _zoom;
    dl->PushClipRect( screen_top_left, ImVec2( screen_bottom_right.x - pad, split_y ), true );
    
    dl->PopClipRect();

    dl->AddRect( screen_top_left, screen_bottom_right, info.border_color, radius, 1.0f );

#endif
}

//********************************************************************
void_t graph_viewer::draw_link( canvas2d_t * can, link_info const & li ) noexcept
{
#if 0
    auto const a = can->world_to_screen( this_t::exec_pin_position( {li.from, li.from_size, this_t::exec_pin::out} ) );
    auto const b = can->world_to_screen( this_t::exec_pin_position( {li.to, li.to_size, this_t::exec_pin::in} ) );

    float_t const bend =
        std::clamp( 0.5f * std::abs( li.to.x() - li.from.x() ), 24.0f, 100.0f ) * _zoom;

    ImGui::GetWindowDrawList()->AddBezierCubic(
        a, ImVec2( a.x + bend, a.y ), ImVec2( b.x - bend, b.y ), b, li.color, 2.0f );
#endif
}

//********************************************************************
motor::math::vec2f_t graph_viewer::exec_pin_position(
    canvas2d_t * can, this_t::exec_pin_info const & ifo ) noexcept
{
#if 1
    return motor::math::vec2f_t();
#else
    float_t const x = ifo.pin == exec_pin::out ? ifo.node_size.x() : 0.0f;
    return ifo.node_pos + motor::math::vec2f_t( x, -0.5f * _header_height );
#endif
}