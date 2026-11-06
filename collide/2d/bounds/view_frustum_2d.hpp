//------------------------------------------------------------
// snakeoil (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#ifndef _SNAKEOIL_MATH_VIEW_FRUSTUM_HPP
#define _SNAKEOIL_MATH_VIEW_FRUSTUM_HPP

#include "frustum.hpp"
#include "../planar/plane/hesse_plane3.hpp"
#include "../matrix/matrix4.hpp"
#include "../vector/vector4.hpp"

namespace so_math
{
	namespace so_volume
	{
		template< typename type_t >
		class view_frustum
		{
			typedef frustum< type_t > frustum_t ;
			typedef frustum_t const & frustum_cref_t ;

			typedef matrix< type_t, 4, 4 > mat4_t ;
			typedef mat4_t const & mat4_cref_t ;

			typedef vector< type_t, 4 > vec4_t ;
			typedef vector< type_t, 3 > vec3_t ;
			typedef vec3_t const & vec3_cref_t ;

		public:

			typedef hesse_plane< type_t, 3 > plane_t ;
			typedef plane_t & plane_ref_t ;

		private:

			plane_t _planes[6] ;

	
		public:
		
			enum view_frustum_plane{
				p_left,
				p_right,
				p_bottom,
				p_top,
				p_near,
				p_far,
				num_view_frustum_planes
			} ;

		public:

			view_frustum( void_t )
			{
			
			}
		
		public:

			/// constructs this view frustum's planes from passed parameters.
			void_t contruct_planes_from( mat4_cref_t view, mat4_cref_t proj ) 
			{
				mat4_t comb = proj * view ;

				vec4_t r0, r1, r2, r3 ;
			
				comb.get_row(0, r0) ;
				comb.get_row(1, r1) ;
				comb.get_row(2, r2) ;
				comb.get_row(3, r3) ;
	
				vec4_t p0 = (r3 + r0).normalized() ;
				vec4_t p1 = (r3 - r0).normalized() ;
				vec4_t p2 = (r3 + r1).normalized() ;
				vec4_t p3 = (r3 - r1).normalized() ;
				vec4_t p4 = (r3 + r2).normalized() ;
				vec4_t p5 = (r3 - r2).normalized() ;

			
				_planes[p_left]	= plane_t( p0.negate() ) ;
				_planes[p_right]	= plane_t( p1.negate() ) ;
				_planes[p_bottom] = plane_t( p2.negate() ) ;
				_planes[p_top]	= plane_t( p3.negate() ) ;
				_planes[p_near]	= plane_t( p4.negate() ) ;
				_planes[p_far]	= plane_t( p5.negate() );

			}
		
			void_t get_plane( size_t i, plane_ref_t plane ) const
			{
				SNAKEOIL_ASSERT( i < num_view_frustum_planes )
				plane = _planes[i] ;
			}
		};

        typedef view_frustum< float > view_frustumf_t ;
	}
}

#endif

