
#pragma once

#include "node.h"

namespace motor
{
    namespace wire
    {
        class node_disconnector
        {

        public:

            static void_t disconnect_everyting( motor::wire::inode_mtr_safe_t n0 ) noexcept
            {
                motor::wire::inode_t::nodes_t ns[2] ;
                motor::wire::inode_t::cleaner_accessor::move_out_all( n0, ns[0] ) ;

                size_t ridx = 0 ;
                size_t widx = 1 ;

                while ( ns[ ridx ].size() != 0 )
                {
                    for ( auto * n : ns[ ridx ] )
                    {
                        motor::wire::inode_t::cleaner_accessor::move_out_all( n, ns[ widx ] ) ;

                        n->disconnect() ;
                        motor::memory::release_ptr( n ) ;
                    }
                    ns[ ridx ].clear() ;

                    widx = ridx ;
                    ridx = ++ridx % 2 ;
                }

                n0->disconnect() ;
                motor::release( n0 ) ;
            }
        };
        motor_typedef( node_disconnector ) ;
    }
}