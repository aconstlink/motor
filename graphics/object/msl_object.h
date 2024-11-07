
#pragma once

#include "../object.hpp"
#include "../shader/compilation_listener.h"

#include "render_object.h"

namespace motor
{
    namespace graphics
    {
        enum class msl_api_type
        {
            none,
            msl_4_0         // gen 4 shader and library
        };

        class MOTOR_GRAPHICS_API msl_object : public object
        {
            motor_this_typedefs( msl_object ) ;

        private:

            // msl shader itself
            // render object (variables, render states)
            // shader object (generated shaders and i/o)
            // geometry object maybe

            motor::string_t _name ;

            motor::vector< motor::string_t > _geo ;
            motor::vector< motor::string_t > _soo ;

            struct data
            {
                msl_api_type t ;
                motor::string_t code ;
            };

            motor::vector< data > _datas ;
            motor::vector< motor::graphics::variable_set_mtr_t > _vars ;

            // default compilation listener
            motor::graphics::compilation_listener_mtr_t _comp_lst = 
                motor::shared( motor::graphics::compilation_listener(), "comp listener" ) ;

        private:

            motor::concurrent::mrsw_t _mtx_compilation_listeners ;
            motor::vector< motor::graphics::compilation_listener_mtr_t > _compilation_listeners ;

        public:

            msl_object( void_t ) noexcept ;
            msl_object( motor::string_in_t name ) noexcept ;
            msl_object( this_rref_t rhv ) noexcept ;
            msl_object( this_cref_t rhv ) noexcept  ;
            virtual ~msl_object( void_t ) noexcept ;

        public: // operator 

            this_ref_t operator = ( this_cref_t rhv ) noexcept ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept ;

        public:

            motor::string_cref_t name( void_t ) const noexcept;
            this_ref_t add( motor::graphics::msl_api_type const t, motor::string_in_t msl_code ) noexcept ;
            this_ref_t clear_shaders( void_t ) noexcept ;

            typedef std::function< void_t ( motor::string_in_t ) > foreach_funk_t ;
            void_t for_each_msl( motor::graphics::msl_api_type const t, foreach_funk_t funk ) const noexcept ;

            this_ref_t link_geometry( motor::string_cref_t name ) noexcept ;
            this_ref_t link_geometry( std::initializer_list< motor::string_t > const & names ) noexcept ;

            // link to stream out object so geometry can be fed from there.
            // the geometry is then mainly used for geometry layout.
            this_ref_t link_geometry( motor::string_cref_t name, motor::string_cref_t soo_name ) noexcept ;

            motor::vector< motor::string_t > const & get_geometry( void_t ) const noexcept ;
            motor::vector< motor::string_t > const & get_streamout( void_t ) const noexcept ;

        public: // variable sets

            this_ref_t add_variable_set( motor::graphics::variable_set_mtr_safe_t vs ) noexcept ;
            this_ref_t fill_variable_sets( size_t const ) noexcept ;
            motor::graphics::variable_set_mtr_safe_t get_varibale_set( size_t const id ) noexcept ;
            motor::graphics::variable_set_mtr_t borrow_varibale_set( size_t const id ) const noexcept ;
            motor::vector< motor::graphics::variable_set_mtr_safe_t > get_varibale_sets( void_t ) const noexcept ;            
            motor::vector< motor::graphics::variable_set_borrow_t::mtr_t > borrow_varibale_sets( void_t ) const noexcept ;

            typedef std::function< void_t ( size_t const i, motor::graphics::variable_set_mtr_t ) > for_each_var_funk_t ;
            void_t for_each( for_each_var_funk_t funk ) noexcept ;

        public:

            void_t register_listener( motor::graphics::compilation_listener_mtr_safe_t lst ) noexcept ;
            void_t unregister_listener( motor::graphics::compilation_listener_mtr_t lst ) noexcept ;
            
            using for_each_change_listerner_funk_t = std::function< void_t ( motor::graphics::compilation_listener_mtr_t ) > ;
            void_t for_each( for_each_change_listerner_funk_t f ) noexcept ;

            motor::graphics::compilation_listener_mtr_t borrow_compilation_listener( void_t ) const noexcept ;
            bool_t has_shader_changed( void_t ) const noexcept ;
            bool_t get_if_successful( motor::graphics::shader_bindings_out_t sb ) noexcept ;
            bool_t reset_and_successful( motor::graphics::shader_bindings_out_t sb ) noexcept ;
        };
        motor_typedef( msl_object ) ;
    }
}
