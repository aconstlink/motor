// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "graph_viewer.h"

using namespace motor::tool::imgui;

//********************************************************************
bool_t graph_viewer::begin( canvas2d_t * can ) noexcept
{
    _can = can;
    _is_in = true;

    auto const mouse_world = can->screen_to_world( ImGui::GetMousePos() );

    if( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) && _dragging_idx != size_t( -1 ) )
    {
        _nodes[ _dragging_idx ].is_dragged = false;
        _nodes[ _dragging_idx ].is_hovered = false;
        _dragging_idx = size_t( -1 );
    }

    if( _dragging_idx == size_t( -1 ) )
    {
        for( size_t i = 0; i < _nodes.size(); ++i )
        {
            auto & n = _nodes[ i ];
            if( !n.used ) continue ;

            // keep hovered if n.is_dragged
            n.is_hovered =
                n.is_dragged || ( mouse_world.greater_equal_than( n.bottom_left ).all() &&
                                    mouse_world.less_equal_than( n.top_right ).all() );

            // only start dragging if the mouse actually clicked inside region.
            // without the click condition, the drag would start if the user clicks
            // anywhere in the canvas and drags the mouse over a node while holding the
            // left mouse button.
            if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && n.is_hovered )
                n.is_dragged = ImGui::IsItemHovered() && ImGui::IsMouseDown( ImGuiMouseButton_Left ) && n.is_hovered;

            // if one is dragged, just break out.
            if( n.is_dragged )
            {
                _mouse_pos = mouse_world ;
                _dragging_idx = i;
                break;
            }
        }
    }

    if( _dragging_idx != size_t( -1 ) )
    {
        auto & n = _nodes[ _dragging_idx ];
        n.bottom_left += mouse_world - _mouse_pos;
        n.top_right += mouse_world - _mouse_pos;
    }

    _mouse_pos = mouse_world;
    return true;
}

//********************************************************************
void_t graph_viewer::end( void_t ) noexcept
{
    assert( _is_in );
    _is_in = false;
}

//********************************************************************
size_t graph_viewer::add_node( node_info const & info ) noexcept
{
    size_t idx = size_t( -1 );
    while( ++idx < _nodes.size() && _nodes[ idx ].used );

    if( idx == _nodes.size() )
    {
        _nodes.emplace_back( this_t::node_entry{} );
    }

    auto & n = _nodes[ idx ];

    n.used = true;
    n.bottom_left = info.pos;
    n.top_right = info.pos + info.size;
    n.body_color = info.body_color;
    n.border_color = info.border_color;
    n.text_color = info.text_color;
    n.header_color = info.header_color;
    n.title = info.title;

    return idx;
}

//********************************************************************
bool_t graph_viewer::remove_node( size_t const idx ) noexcept
{
    if( idx >= _nodes.size() ) return false;

    _nodes[ idx ].used = false;
    if( _dragging_idx == idx ) 
    {
        _dragging_idx = size_t(-1) ;
    }
    return true;
}

//********************************************************************
bool_t graph_viewer::draw_node( size_t const idx ) noexcept
{
    if( idx >= _nodes.size() || !_nodes[ idx ].used ) return false;

    auto & n = _nodes[ idx ];

    float_t const font_scale = 0.8f;
    float_t const header_h = 24.0f; //_header_height * _zoom;

    auto const title_size = _can->measure_text_size( n.title.c_str(), header_h * font_scale );
    auto size = n.size();

    float_t const pad = 8.0f; //* _zoom;

    if( size.x() <= title_size.x() )
    {
        size.x( title_size.x() + 2.0f * pad );
    }

    n.top_right = n.bottom_left + size;
    n.title_size = title_size;

    auto const pos = n.bottom_left;

    // size = abs( bottom_left - top_right )
    float_t const body_h = size.y() - header_h;

    float_t const radius = 6.0f;

    // draw header
    _can->draw_rect_filled( { { pos.x(), pos.y() + body_h }, { size.x(), header_h }, n.header_color,
        6.0f, ImDrawFlags_RoundTop } );

    // draw body
    _can->draw_rect_filled(
        { pos, { size.x(), body_h }, n.body_color, radius, ImDrawFlags_RoundBottom } );

    _can->push_clip_rect(
        { .pos = pos + motor::math::vec2f_t( pad, body_h ), .size = n.title_size } );

    _can->draw_text( { .title = n.title.c_str(),
        .pos = pos + motor::math::vec2f_t( pad, body_h ),
        .font_height = int_t( std::floor( header_h * font_scale ) ),
        .color = n.text_color } );

    _can->pop_clip_rect();

    {
        motor::math::vec3f_t bc = {
            n.border_color.Value.x, n.border_color.Value.y, n.border_color.Value.z };

        if( n.is_hovered ) bc.negate();

        ImColor border_color = { bc.x(), bc.y(), bc.z(), n.border_color.Value.w };

        _can->draw_rect( { .pos = pos, .size = size, .color = border_color, .radius = radius } );
    }

    return true;
}

//********************************************************************
void_t graph_viewer::draw_link( link_info const & li ) noexcept
{
    if( li.node_id_from >= _nodes.size() || !_nodes[ li.node_id_from ].used ) return;
    if( li.node_id_to >= _nodes.size() || !_nodes[ li.node_id_to ].used ) return;

    auto const a = this_t::exec_pin_position( { li.node_id_from, this_t::exec_pin::out } );
    auto const b = this_t::exec_pin_position( { li.node_id_to, this_t::exec_pin::in } );

    float_t const bend = ( b - a ).length() * 1.0f / 4.0f;

    _can->draw_bezier_curve(
        { { a, { a.x() + bend, a.y() }, { b.x() - bend, b.y() }, b }, li.color, 2.0f } );
}

//********************************************************************
motor::math::vec2f_t graph_viewer::exec_pin_position( this_t::exec_pin_info const & ifo ) noexcept
{
    auto const p0 = _nodes[ ifo.node_id ].top_right;
    auto const p1 = _nodes[ ifo.node_id ].bottom_left;

    auto const pos = ifo.pin == exec_pin::out ? p0 : motor::math::vec2f_t{ p1.x(), p0.y() };
    return pos + motor::math::vec2f_t( 0.0f, -0.5f * 24.0f /*header height*/ );
}