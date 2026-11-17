
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "graph_viewer.h"

#include <motor/wire/node/node.h>
#include <motor/math/vector/vector2.hpp>

namespace motor::tool::imgui
{
class MOTOR_TOOL_API graph_view_wire
{
    motor_this_typedefs( graph_view_wire );

  private:

    struct node
    {
        size_t id;

        // nodes are positioned in a matrix style,
        // so this is its matrix position
        // x, y : column, row
        motor::math::vec2ui_t matrix_pos;
    };

    struct link
    {
        size_t from;
        size_t to;
    };

    bool_t _inital_set = false;
    size_t _num_columns = 0;

    motor::wire::inode::tier_builder_t::build_result_t _tb_results;
    motor::vector< node > _nodes;
    motor::vector< link > _links;

  public:

    graph_view_wire( void_t ) noexcept ;
    graph_view_wire( this_cref_t ) = delete ;
    graph_view_wire( this_rref_t ) noexcept ;
    ~graph_view_wire( void_t ) noexcept ;

    void_t build( motor::tool::imgui::graph_viewer_ref_t gv, motor::wire::inode_mtr_t start ) noexcept;
    void_t visualize( motor::tool::imgui::graph_viewer_ref_t gv ) noexcept ;
};
motor_typedef( graph_view_wire );
} // namespace motor::tool::imgui