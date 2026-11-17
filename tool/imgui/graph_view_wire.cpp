

#include "graph_view_wire.h"

#include <motor/log/global.h>

using namespace motor::tool::imgui;

namespace internal
{
static const ImColor node_colors[] = {
    ImColor( 76, 145, 133 ),  // Jade
    ImColor( 94, 139, 196 ),  // Kornblumenblau
    ImColor( 218, 164, 104 ), // Aprikose
    ImColor( 169, 132, 194 ), // Lavendel
    ImColor( 204, 115, 125 )  // Koralle
};
static const size_t num_colors = sizeof( node_colors ) / sizeof( ImColor );

static ImColor get_color( size_t const idx ) noexcept
{
    return node_colors[ idx % num_colors ];
}
} // namespace internal

//******************************************************************
graph_view_wire::graph_view_wire( void_t ) noexcept {}

//******************************************************************
graph_view_wire::graph_view_wire( this_rref_t rhv ) noexcept
    : _tb_results( std::move( rhv._tb_results ) ), _nodes( std::move( rhv._nodes ) ),
      _links( std::move( rhv._links ) )
{
}

//******************************************************************
graph_view_wire::~graph_view_wire( void_t ) noexcept {}

//******************************************************************
void_t graph_view_wire::build(
    motor::tool::imgui::graph_viewer_ref_t gv, motor::wire::inode_mtr_t start ) noexcept
{
    {
        motor::wire::inode::tier_builder_t tb;

        tb.build( start, _tb_results );

        if( _tb_results.has_cylce ) motor::log::global_t::status( "graph has a cycle." );
        assert( _tb_results.has_cylce == false );
    }

    {
        _num_columns = _tb_results.tiers.size();
    }

    {
        motor::hash_map< motor::wire::inode_mtr_t, size_t > nodes_to_ids;

        size_t const num_nodes = _tb_results.num_nodes;

        auto const start = motor::math::vec2f_t();
        auto const size = motor::math::vec2f_t( 100.0f, 50.0f );

        // visualize tasks and collect info
        {
            uint_t column = 0;
            for( auto & tier : _tb_results.tiers )
            {
                uint_t row = 0;
                for( auto * n : tier.nodes )
                {
                    size_t const nid = gv.add_node( { .title = n->name(),
                        .pos = start + motor::math::vec2f_t{ float_t( column ), float_t( row ) } *
                                           size * 2.0f,
                        .size = size,
                        .header_color = internal::get_color( column ) } );

                    _nodes.emplace_back( this_t::node{
                        .id = nid, .matrix_pos = motor::math::vec2ui_t( column, row ) } );

                    nodes_to_ids[ n ] = nid;
                    ++row;
                }
                ++column;
            }
        }

        // link all tasks
        {
            int_t link_id = 0;
            motor::wire::inode::tier_builder_t::output_slot_walk( _tb_results,
                [ & ]( motor::wire::inode_mtr_t n_in, motor::wire::inode::nodes_in_t outputs )
            {
                size_t const tid = nodes_to_ids[ n_in ];

                for( auto * n : outputs )
                {
                    size_t const oid = nodes_to_ids[ n ];

                    int_t const out_id = num_nodes + tid * 2 + 1;
                    int_t const in_id = num_nodes + oid * 2 + 0;

                    _links.emplace_back( this_t::link{ tid, oid } );
                }
            } );
        }
    }

    _inital_set = false;
}

//*******************************************************
void_t graph_view_wire::visualize( motor::tool::imgui::graph_viewer_ref_t gv ) noexcept
{
    // draw links here
    for( auto & l : _links )
    {

        motor::tool::imgui::graph_viewer_t::link_info li{
            .node_id_from = l.from, .node_id_to = l.to };

        gv.draw_link( li );
    }
    gv.draw_all_nodes();
}