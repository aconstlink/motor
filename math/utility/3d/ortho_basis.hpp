#pragma once

namespace motor
{
    namespace math
    {
        namespace m3d
        {
            template< typename vec3_t >
            void create_orthonormal_basis( vec3_t const & vcNormal, vec3_t & outX, vec3_t & outY, vec3_t & outZ ) 
            {
                outZ = vcNormal ;

                vec3_t vcUp = vec3_t( 0.0f, 1.0f, 0.0f ) ;

                // This check must be done.
                // If the normal and the desired up vector is colinear,
                // another up vector must be choosen.
                if( vcUp.equals(vcNormal,TypeTraits<vec3_t::type_t>::epsilon()) )vcUp = vec3_t( 0.0f,0.0f,1.0f ) ;

                outY = (vcUp - ( vcNormal * (vcNormal.dot(vcUp)) )).normalize() ;
                outX = outY.crossed( vcNormal ) ;
            }

            /**
                This function creates an orthonormal basis by using a given direction 
                vector. This is the normal vector.
                This basis is than normalized and orthogonal.

                Be sure to not pass a null vector. There is no checking.

                @param vcNormal [in] A non null vector. Must be normalized.
                @param matOut [out] The created orthonormal basis.
            */
            template< typename vec3_t, typename mat3_t >
            void create_orthonormal_basis( vec3_t const & vcNormal, mat3_t & matOut ) 
            {
                if( vec3_t(0.0f,0.0f,0.0f).equals(vcNormal,TypeTraits<vec3_t::type_t>::epsilon()) ) return ;

                vec3_t vcX ;
                vec3_t vcY ;
                vec3_t vcZ ;

                CreateOrthonormalBasis( vcNormal, vcX, vcY, vcZ ) ;

                matOut.set_column_u(0, vcX ) ;
                matOut.set_column_u(1, vcY ) ;
                matOut.set_column_u(2, vcZ ) ;
            }
        } 
    }
} 