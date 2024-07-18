#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/wire/node/node.h>
#include <motor/math/vector/vector2.hpp>

#include <imgui.h>
#include <imnodes.h>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API imnodes_wire
        {
            motor_this_typedefs( imnodes_wire ) ;

        public:

            struct node
            {
                int_t id ;
                motor::string_t name ;
                ImVec2 pos ;

                // nodes are positioned in a matrix style,
                // so this is its matrix position
                // x, y : column, row
                motor::math::vec2ui_t matrix_pos ;
            } ;

            struct link
            {
                int_t nid ;
                int_t a ;
                int_t b ;
            };

            bool_t _inital_set = false ;
            size_t _num_columns = 0 ;

        private:

            motor::wire::node::tier_builder_t::build_result_t _tb_results ;
            motor::vector< node > _nodes ;
            motor::vector< link > _links ;

        public:

            imnodes_wire(  void_t ) noexcept ;
            ~imnodes_wire( void_t ) noexcept ;

        public:
            
            void_t begin( void_t ) noexcept ;
            void_t end( bool_t const force_mini_map = false ) noexcept ;
            void_t build( motor::wire::inode_mtr_t start ) noexcept ;

            int_t visualize( int_t id ) noexcept ;

            using for_each_link_funk_t = std::function< void_t ( int_t const link_id, this_t::link const & ) > ;
            void_t for_each( for_each_link_funk_t f ) noexcept ;
            
            
        };
        motor_typedef( imnodes_wire ) ;
    }
}