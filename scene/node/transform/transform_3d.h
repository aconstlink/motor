//------------------------------------------------------------
// verzerrt (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#ifndef _VERZERRT_SCENE_NODE_TRANSFORM_TRANSFORM_3D_H_
#define _VERZERRT_SCENE_NODE_TRANSFORM_TRANSFORM_3D_H_

#include "../decorator.h"

#include <verzerrt/variable/variable.hpp>
#include <verzerrt/math/utility/3d/transformation.hpp>

namespace vzt_scene
{
    namespace vzt_node
    {
        class VZT_SCENE_API transform_3d : public decorator
        {
            typedef decorator base_t ;
            vzt_this_typedefs( transform_3d ) ;

            vzt_typedefs( vzt_math::vzt_3d::trafof_t, trafo ) ;
            vzt_typedefs( vzt_var::variable< vzt_math::mat4f_t >, var_mat4f ) ;

        private:

            trafo_t _trafo ;
            var_mat4f_ptr_t _var_trafo ;
            
        public:

            transform_3d( void_t ) ;
            transform_3d( this_rref_t ) ;
            transform_3d( this_cref_t ) ;
            transform_3d( vzt_math::vzt_3d::trafof_cref_t ) ;
            transform_3d( iparent_ptr_t ) ;
            transform_3d( node_ptr_t, iparent_ptr_t=nullptr ) ;
            virtual ~transform_3d( void_t ) ;

        private:

            void_t ctor_init( void_t ) ;

        public:

            static this_ptr_t create( vzt_memory::purpose_cref_t ) ;
            static this_ptr_t create( this_rref_t, vzt_memory::purpose_cref_t ) ;
            static void_t destroy( this_ptr_t ) ;

        public:

            this_ref_t set_decorated( node_ptr_t ) ;

        public:

            void_t set_transformation( trafo_cref_t ) ;
            trafo_cref_t get_transformation( void_t ) const ;
            trafo_t compute_trafo( void_t ) const ;

        public:

            virtual vzt_scene::result apply( vzt_scene::vzt_visitor::ivisitor_ptr_t ptr ) ;

        public:

            virtual void_t destroy( void_t ) ;
        };
        vzt_typedef( transform_3d ) ;
    }
}

#endif
