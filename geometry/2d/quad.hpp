
#pragma once

#include "typedefs.h"

namespace motor
{
    namespace geometry
    {
        /// constructs a quad in [-0.5,0.5]
        template< typename type_t >
        class quad_2d
        {
            typedef motor::math::vector2< type_t > vec_t ;

            typedef vec_t& vec_ref_t ;
            typedef vec_t const& vec_const_ref_t ;
            typedef vec_t* vec_ptr_t ;
            typedef vec_t const* vec_cptr_t ;

        private:

            std::vector< vec_t > _positions ;
            std::vector< uint_t > _indices ;

            size_t _tess ;

        public:

            quad( size_t tess )
            {
                _tess = tess ;

                create() ;
            }

            size_t get_num_vertices( void_t ) const { return _positions.size() ; }
            size_t get_num_indices( void_t ) const { return _indices.size() ; }

            vec_cptr_t get_positions( void_t ) const { return &( _positions[ 0 ] ); }
            uint_t const* get_indices( void_t ) const { return &( _indices[ 0 ] ) ; }

            void_t create( void_t )
            {
                size_t num_cells = 1 << _tess ;
                size_t num_points = num_cells + 1 ;
                size_t index = 0 ;

                _positions.resize( num_points * num_points ) ;
                _indices.resize( num_cells * num_cells * 6 ) ;

                type_t step = 1.0f / type_t( num_cells ) ;
                vec_t pos( type_t( -0.5 ), type_t( -0.5 ) ) ;

                for( size_t j = 0; j < num_points; ++j )
                {
                    for( size_t i = 0; i < num_points; ++i )
                    {
                        _positions[ index ] = pos ;
                        pos.x() += step ;
                        ++index ;
                    }
                    pos.y() += step ;
                    pos.x() = type_t( -0.5 ) ;
                }

                index = 0 ;
                for( size_t j = 0; j < num_cells; ++j )
                {
                    for( size_t i = 0; i < num_cells; ++i )
                    {
                        _indices[ index + 0 ] = uint_t( j * num_points + i ) ;
                        _indices[ index + 1 ] = uint_t( ( j + 1 ) * num_points + i ) ;
                        _indices[ index + 2 ] = uint_t( ( j + 1 ) * num_points + ( i + 1 ) ) ;

                        _indices[ index + 3 ] = uint_t( j * num_points + i ) ;
                        _indices[ index + 4 ] = uint_t( ( j + 1 ) * num_points + ( i + 1 ) ) ;
                        _indices[ index + 5 ] = uint_t( j * num_points + ( i + 1 ) ) ;

                        index += 6 ;
                    }
                }
            }
        } ;
    }
}