// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "graph_viewer.h"

using namespace motor::tool::imgui;

//********************************************************************
bool_t graph_viewer::begin( canvas2d_t * can ) noexcept
{
    _can = can;
    _is_in = true;
    return true;
}

//********************************************************************
void_t graph_viewer::end( void_t ) noexcept
{
    assert( _is_in );
    _is_in = false;
    _rect_cache.clear();
    _cur_rect = size_t( -1 );
}

//********************************************************************
size_t graph_viewer::draw_node( node_info const & info ) noexcept
{
    float_t const font_scale = 0.8f;
    float_t const header_h = 24.0f; //_header_height * _zoom;
    float_t const pad = 8.0f;       //* _zoom;

    auto const title_size = _can->measure_text_size( info.title, header_h * font_scale );
    auto size = info.size;

    if( info.size.x() <= title_size.x() )
    {
        size.x( title_size.x() + 2.0f * pad );
    }

    float_t const body_h = size.y() - header_h;

    float_t const radius = 6.0f;

    // draw header
    _can->draw_rect_filled( { { info.pos.x(), info.pos.y() + body_h }, { size.x(), header_h },
        info.header_color, 6.0f, ImDrawFlags_RoundTop } );

    // draw body
    _can->draw_rect_filled(
        { info.pos, { size.x(), body_h }, info.body_color, radius, ImDrawFlags_RoundBottom } );

    _can->push_clip_rect(
        { .pos = info.pos + motor::math::vec2f_t( pad, body_h ), .size = title_size } );

    _can->draw_text( { .title = info.title,
        .pos = info.pos + motor::math::vec2f_t( pad, body_h ),
        .font_height = int_t( std::floor( header_h * font_scale ) ),
        .color = info.text_color } );

    _can->pop_clip_rect();

    _can->draw_rect(
        { .pos = info.pos, .size = size, .color = info.border_color, .radius = radius } );

    // add to node cache
    if( _rect_cache.size() <= ++_cur_rect )
    {
        _rect_cache.resize( _rect_cache.size() + 10 );
    }
    _rect_cache[ _cur_rect ] = { info.pos, info.pos + size };
    return _cur_rect;
}

//********************************************************************
void_t graph_viewer::draw_link( link_info const & li ) noexcept
{
    if( li.node_id_from >= _rect_cache.size() || 
    li.node_id_to >= _rect_cache.size() ) return ;

    auto const a = this_t::exec_pin_position( { li.node_id_from, this_t::exec_pin::out } );
    auto const b = this_t::exec_pin_position( { li.node_id_to, this_t::exec_pin::in } );

    float_t const bend = (b-a).length() * 1.0f / 4.0f ;

    _can->draw_bezier_curve(
        { { a, { a.x() + bend, a.y() }, { b.x() - bend, b.y() }, b }, li.color, 2.0f } );
}

//********************************************************************
motor::math::vec2f_t graph_viewer::exec_pin_position( this_t::exec_pin_info const & ifo ) noexcept
{    
    auto const p0 = _rect_cache[ ifo.node_id ].top_right ;
    auto const p1 = _rect_cache[ ifo.node_id ].bottom_left ;

    auto const pos = ifo.pin == exec_pin::out ? p0 : motor::math::vec2f_t{ p1.x(), p0.y() };
    return pos + motor::math::vec2f_t( 0.0f, -0.5f * 24.0f /*header height*/ );
}