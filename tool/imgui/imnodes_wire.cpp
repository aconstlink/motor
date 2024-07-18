
#include "imnodes_wire.h"

#include <motor/log/global.h>

using namespace motor::tool ;

//*******************************************************
imnodes_wire::imnodes_wire( void_t ) noexcept 
{
}

//*******************************************************
imnodes_wire::~imnodes_wire( void_t ) noexcept
{
}

//*******************************************************
void_t imnodes_wire::begin( void_t ) noexcept 
{
    ImNodes::BeginNodeEditor();
}

//*******************************************************
void_t imnodes_wire::end( bool_t const force_mini_map ) noexcept 
{
    if( force_mini_map ) ImNodes::MiniMap();
    ImNodes::EndNodeEditor();

    {
        int_t link_id = 0 ;
        if ( ImNodes::IsLinkDestroyed( &link_id ) )
        {
            int bp = 0 ;
        }

    }
}

//*******************************************************
void_t imnodes_wire::build( motor::wire::inode_mtr_t start ) noexcept 
{
    {
        motor::wire::node::tier_builder_t tb ;

        tb.build( start, _tb_results ) ;


        if ( _tb_results.has_cylce ) motor::log::global_t::status( "graph has a cycle." ) ;
        assert( _tb_results.has_cylce == false ) ;
    }

    {
        _num_columns = _tb_results.tiers.size() ;
    }

    {
        // 
        {
            motor::hash_map< motor::wire::inode_mtr_t, int_t > nodes_to_ids ;

            int_t const num_nodes = int_t( _tb_results.num_nodes ) ;

            // visualize tasks and collect info
            {
                int_t nid = 0 ;

                uint_t column = 0 ;
                for ( auto & tier : _tb_results.tiers )
                {
                    uint_t row = 0 ;
                    for ( auto * n : tier.nodes )
                    {
                        _nodes.emplace_back( this_t::node 
                            { 
                                nid, n->name(), ImVec2( 0.0f, 0.0f ), 
                                motor::math::vec2ui_t( column, row ) } ) ;

                        nodes_to_ids[ n ] = nid ;

                        ImVec2 const dims = ImVec2( 100, 200 ) ;// ImNodes::GetNodeDimensions( nid ) ;

                        ++nid ;
                        ++row ;
                    }

                    ++column ;
                }
            }

            // link all tasks
            {
                int_t link_id = 0 ;
                motor::wire::node::tier_builder_t::output_slot_walk( _tb_results,
                    [&] ( motor::wire::inode_mtr_t n_in, motor::wire::inode::nodes_in_t outputs )
                {
                    int_t const tid = nodes_to_ids[ n_in ] ;

                    for ( auto * n : outputs )
                    {
                        int_t const oid = nodes_to_ids[ n ] ;

                        int_t const out_id = num_nodes + tid * 2 + 1 ;
                        int_t const in_id = num_nodes + oid * 2 + 0 ;

                        _links.emplace_back( this_t::link {tid, in_id, out_id} ) ;
                    }
                } ) ;
            }
        }
    }

    _inital_set = false ;
}

//*******************************************************
int_t imnodes_wire::visualize( int_t id ) noexcept 
{
    int_t nid = id ;

    ImVec2 cur_pos( 0, 0 ) ;

    // #1 : place nodes
    {
        for ( auto & n : _nodes )
        {
            ImNodes::BeginNode( nid );

            ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted( n.name.c_str() );
            ImNodes::EndNodeTitleBar();

            #if 0
            ImGui::Dummy( ImVec2() ) ;
            #else
            {
                int_t const base_attr_id = int_t( _nodes.size() ) + nid * 2 ;
                ImNodes::BeginInputAttribute( base_attr_id + 0 );
                ImGui::Text( "input" );
                ImNodes::EndInputAttribute();


                ImNodes::BeginOutputAttribute( base_attr_id + 1 );
                ImGui::Indent( 40 );
                ImGui::Text( "output" );
                ImNodes::EndOutputAttribute();
            }
            #endif

            ImNodes::EndNode();

            n.pos = ImNodes::GetNodeGridSpacePos( nid ) ;
            n.id = nid ;
            
            ++nid ;
        }
    }

    // #2 : do initial placement
    {
        if( !_inital_set )
        {
            motor::vector< float_t > max_width_per_column( _num_columns ) ;
            motor::vector< size_t > num_nodes_per_column( _num_columns ) ;
            for( size_t i=0; i<_num_columns; ++i ) num_nodes_per_column[i] = 0 ;

            for ( auto & n : _nodes )
            {
                ImVec2 const dims = ImNodes::GetNodeDimensions( n.id ) ;
                auto const old_width = max_width_per_column[n.matrix_pos.x()]  ;
                max_width_per_column[n.matrix_pos.x()] = std::max( old_width, dims.x * 1.5f ) ;
                ++num_nodes_per_column[n.matrix_pos.x()] ;
            }

            float_t y = 0.0f ;

            for( auto & n : _nodes )
            {
                float_t x = 0.0f ;
                for( size_t i=0; i<n.matrix_pos.x(); ++i  ) x += max_width_per_column[i] ;

                if( n.matrix_pos.y() == 0 ) y = 0.0f ;                

                ImNodes::SetNodeGridSpacePos( n.id, ImVec2( x, y ) ) ;
                y+= ImNodes::GetNodeDimensions( n.id ).y * 1.5f ;
            }

            _inital_set = true ;
        }
    }

    // #3 : make links
    {
        int_t link_id = -1 ;
        for( auto & link : _links)
        {
            ImNodes::Link( ++link_id, id+link.a, id+link.b ) ;
        }
    }

    return int_t( _nodes.size() ) + int_t( _nodes.size() ) * 2 ;
}

//*******************************************************
void_t imnodes_wire::for_each( for_each_link_funk_t f ) noexcept 
{
    int_t link_id = -1  ;
    for( auto const & l : _links )
    {
        f( ++link_id, l ) ;
    }
}