
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
void_t imnodes_wire::build( motor::wire::inode_mtr_t start ) noexcept 
{
    {
        motor::wire::node::tier_builder_t tb ;

        tb.build( start, _tb_results ) ;


        if ( _tb_results.has_cylce ) motor::log::global_t::status( "graph has a cycle." ) ;
        assert( _tb_results.has_cylce == false ) ;
    }

    {
        ImVec2 cur_pos( 0.0f, 0.0f ) ;//= ImGui::GetCursorPos() ; 

        // 
        {
            motor::hash_map< motor::wire::inode_mtr_t, int_t > nodes_to_ids ;

            int_t const num_nodes = int_t( _tb_results.num_nodes ) ;

            // visualize tasks and collect info
            {
                int_t nid = 0 ;

                for ( auto & tier : _tb_results.tiers )
                {
                    for ( auto * n : tier.nodes )
                    {
                        _nodes.emplace_back( this_t::node { n->name(), cur_pos } );
                        nodes_to_ids[ n ] = nid ;

                        ImVec2 const dims = ImVec2( 100, 200 ) ;// ImNodes::GetNodeDimensions( nid ) ;

                        cur_pos.y += dims.y * 2.0f ;

                        ++nid ;
                    }

                    ImVec2 const dims( 50.0f, 1.0f ) ;
                    cur_pos.x += dims.x * 3.0f ;
                    cur_pos.y = 0.0f ;
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

                        //ImNodes::Link( link_id++, in_id, out_id ) ;
                    }
                } ) ;
            }
        }
    }
}

//*******************************************************
int_t imnodes_wire::visualize( int_t id ) noexcept 
{
    int_t nid = id ;

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

            if( !n.inital_set )
            {
                ImNodes::SetNodeGridSpacePos( nid, n.pos ) ;
                n.inital_set = true ;
            }
            else 
            {
                n.pos = ImNodes::GetNodeGridSpacePos( nid ) ;
            }
            
            
            
            ++nid ;
        }
    }

    // #2 : make links
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