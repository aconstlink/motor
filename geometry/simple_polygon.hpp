
#pragma once

namespace motor
{
    namespace geometry
    {
        template< typename vec_type >
        class simple_polygon
        {
            motor_this_typedefs( simple_polygon ) ;

        public:
                
            struct edge
            {
                size_t a ;
                size_t b ;
            };

            motor_typedef( edge ) ;
            motor_typedefs( motor::vector< edge_t >, edges ) ;

            motor_typedefs( vec_type, point ) ;
            motor_typedefs( motor::vector< point_t >, points ) ;

        private:

            points_t _points ;
            edges_t _edges ;

        public:

            simple_polygon( void_t ) noexcept
            {
            }

            simple_polygon( this_cref_t rhv ) noexcept : 
                _edges( rhv._edges ), _points( rhv._points )
            {
            }

            simple_polygon( points_cref_t points ) noexcept : _points( points )
            {
                _edges.resize( points.size() ) ;
                for( size_t i=0; i<points.size()-1; ++i )
                {
                    _edges[i] = { i, i+1 } ;
                }

                _edges[_edges.size()-1] = { _points.size()-1, 0 } ;
            }

            simple_polygon( points_cref_t points, edges_cref_t ) noexcept : 
                _points( points ), _edges( edges ) {}

            simple_polygon( points_rref_t points, edges_rref_t edges ) noexcept : 
                _points( std::move( points ) ), _edges( std::move( edges ) ) {}

            simple_polygon( points_cref_t points, edges_rref_t edges ) noexcept : 
                _points( points ), _edges( std::move( edges ) ) {}

        public:

            size_t num_points( void_t ) const noexcept
            {
                return _points.size() ;
            }

            size_t num_edges( void_t ) const noexcept
            {
                return _edges.size() ;
            }

            this_t::point_cref_t point_at( size_t const i ) const noexcept
            {
                assert( i < _points.size() ) ;
                return _points[i] ;
            }

            this_t::edge_cref_t edge_at( size_t const i ) const noexcept
            {
                assert( i < _edges.size() ) ;
                return _edges[i] ;
            }

            std::pair< point_t, point_t > points_at( this_t::edge_cref_t e ) const noexcept
            {
                return std::make_pair( this_t::point_at( e.a ), this_t::point_at( e.b ) ) ;
            }

            this_t::edges_cref_t edges( void_t ) const noexcept
            {
                return _edges ;
            }

            this_t::points_cref_t points( void_t ) const noexcept
            {
                return _points ;
            }

            void_t clear( void_t ) noexcept
            {
                _points.clear() ;
                _edges.clear() ;
            }

        public:

            // ear-clippling triangulation
            this_t triangulate( void_t ) const noexcept
            {
                this_t::edges_t ret ;
                ret.reserve( _points.size() * 3 ) ;

                if( _points.size() < 3 ) return this_t() ;

                auto edges = _edges ;

                size_t tris = 0 ;
                size_t const tris_need = _points.size() - 2 ;

                size_t ei = size_t(-1) ;
                while( tris != tris_need )
                {
                    if( ++ei > (tris_need << 2) ) break ;

                    size_t const e0i = this_t::cur_edge( ei, edges ) ;
                    size_t const e1i = this_t::next_edge( e0i, edges ) ;

                    auto const e0 = edges[ e0i ] ;
                    auto const e1 = edges[ e1i ] ;

                    auto const [a,b] = e0 ;
                    auto const [c,d] = e1 ;

                    // test if already triangle
                    {
                        auto const e2 = edges[ this_t::next_edge( e1i, edges ) ] ;
                        if( a == e2.b && d == e2.a ) 
                        {
                            this_t::add_triangle( edges[e0i], edges[e1i], e2, ret ) ;
                            this_t::replace_edges( e0i, e1i, {size_t(-1), size_t(-1)}, edges ) ;

                            ++tris ;

                            continue ;
                        }
                    }

                    this_t::edge_t diag = { d, a } ;

                    // only convex edges can be clipped
                    {
                        if( !this_t::is_convex( this_t::dir_from_edge( e0 ), 
                            this_t::dir_from_edge( e1 ) ) ) continue ;
                    }

                    // this makes this algo O(n^2)
                    // test if another point is inside triangle
                    {
                        auto const p0 = this_t::ortho_from_edge_left( e0 ) ;
                        auto const p1 = this_t::ortho_from_edge_left( e1 ) ;
                        auto const p2 = this_t::ortho_from_edge_left( diag ) ;

                        size_t i = size_t(-1) ;
                        while( ++i < _points.size() )
                        {
                            if ( i == a || i == b || i == d ) continue ;

                            motor::math::vec3f_t const ndot(
                                p0.dot( _points[ i ] - _points[ c ] ),
                                p1.dot( _points[ i ] - _points[ c ] ),
                                p2.dot( _points[ i ] - _points[ d ] ) ) ;

                            if ( ( motor::math::vec3f_t( 0.0f ).less_than( ndot ) ).all() ) break ;
                        }

                        if( i != _points.size() ) continue ;
                    }
                
                    {
                        this_t::add_triangle( edges[e0i], edges[e1i], diag, ret ) ;
                        this_t::replace_edges( e0i, e1i, {diag.b, diag.a}, edges ) ;
                        
                        ++tris ;
                    }
                }
                return this_t( _points, std::move( ret ) ) ;
            }


        private:

            motor::math::vec2f_t ortho_from_edge_left( this_t::edge_t const & e ) const noexcept
            {
                auto const [a, b] = e ;
                auto const cur_dir = ( _points[ b ] - _points[ a ] ).normalize() ;
                return cur_dir.ortho_left() ;
            }

            motor::math::vec2f_t ortho_from_edge_left( size_t const e, this_t::edges_t const & edges ) const noexcept
            {
                return this_t::ortho_from_edge_left( edges[ e ] ) ;
            }

            this_t::point_t dir_from_edge( this_t::edge_t const & e ) const noexcept
            {
                return ( _points[ e.b ] - _points[ e.a ] ).normalize() ;
            }

            // e1 follow e0
            bool_t is_convex( this_t::point_cref_t e0_dir, this_t::point_cref_t e1_dir ) const noexcept
            {
                auto const dir = e0_dir ;
                auto const nrm = dir.ortho_left() ;

                this_t::point_t const signs (
                    e1_dir.dot( dir ),
                    e1_dir.dot( nrm )
                ) ;

                return 
                    (signs.x() >= 0.0f && signs.y() >= 0.0f) || 
                    (signs.x() < 0.0f && signs.y() > 0.0f ) ;

            }

            // replaces e0 with {-1,-1} and e1 with e
            void_t replace_edges( size_t const e0, size_t const e1, this_t::edge_t const & e, this_t::edges_t & edges ) const noexcept
            {
                edges[e0] = {size_t(-1), size_t(-1) } ;
                edges[e1] = e ;
            }

            // pass eid where this func searches for next id != -1
            size_t cur_edge( size_t eid, edges_t const & edges ) const noexcept
            {
                size_t id = eid % edges.size() ;
                while( true ) 
                {
                    if ( edges[ id ].a == size_t( -1 ) ) 
                    {
                        id = ++eid % edges.size() ;
                        continue ;
                    }
                    break ;
                }
                return id ;
            }

            // pass eid where this func searches for next id != -1
            // eid should be determined by cur_edge
            size_t next_edge( size_t eid, edges_t const & edges ) const noexcept
            {
                size_t id = ++eid % edges.size() ;
                while( true ) 
                {
                    if ( edges[ id ].a == size_t( -1 ) ) 
                    {
                        id = ++eid % edges.size() ;
                        continue ;
                    }
                    break ;
                }
                return id ;
            }

            void_t add_triangle( this_t::edge_t const & e0, this_t::edge_t const & e1, 
            this_t::edge_t const & e2, this_t::edges_t & tris ) const noexcept
            {
                tris.emplace_back( e0 ) ;
                tris.emplace_back( e1 ) ;
                tris.emplace_back( e2 ) ;
            }
        };
    }
}