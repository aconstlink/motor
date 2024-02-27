#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/variable/variable_set.hpp>
#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/image_object.h>

#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/device/layouts/three_mouse.hpp>
#include <motor/device/layouts/ascii_keyboard.hpp>

#include <imgui/imgui.h>

namespace motor
{
    namespace tool
    {
        class MOTOR_TOOL_API imgui
        {
            motor_this_typedefs( imgui ) ;

        private:

            ImGuiContext* _ctx = nullptr ;

            struct vertex 
            { 
                motor::math::vec2f_t pos ; 
                motor::math::vec2f_t uv ; 
                motor::math::vec4f_t color ; 
            } ;

            motor::graphics::render_object_mtr_t _rc = nullptr ;
            motor::graphics::shader_object_mtr_t _sc = nullptr ;
            motor::graphics::geometry_object_mtr_t _gc = nullptr ;
            motor::graphics::image_object_mtr_t _ic = nullptr ;

            motor::vector< motor::graphics::variable_set_mtr_t > _vars ;
            motor::graphics::state_object_mtr_t  _rs = nullptr ;

            uint_t _width = 0 ;
            uint_t _height = 0 ;

            bool_t _texture_added = false ;

            bool_t _init = false ;
            bool_t _deinit = true ;

        private:


        public:

            imgui( motor::string_cref_t ) noexcept ;
            imgui( this_cref_t ) = delete ;
            imgui( this_rref_t ) noexcept ;
            ~imgui( void_t ) noexcept ;

            void_t init( motor::string_cref_t ) noexcept ;
            
            
            void_t render( motor::graphics::gen4::frontend_mtr_t ) noexcept ;
            void_t deinit( motor::graphics::gen4::frontend_mtr_t ) noexcept ;

            typedef std::function< void_t ( void_t ) > exec_funk_t ;
            void_t execute( exec_funk_t ) noexcept ;

        public:

            struct window_data
            {
                int_t width ;
                int_t height ;
            };
            motor_typedef( window_data ) ;

            void_t update( window_data_cref_t ) noexcept ;
            void_t update( motor::device::three_device_borrow_t::mtr_t ) noexcept ;
            void_t update( motor::device::ascii_device_borrow_t::mtr_t ) noexcept ;

            // Returns an id that can be used to render the passed image.
            // the image needs to be registered before via an image_object.
            ImTextureID texture( motor::string_in_t ) noexcept ;
            

        private:

            void_t begin( void_t ) noexcept ;
            void_t end( void_t ) noexcept ;

            void_t do_default_imgui_init( void_t ) ;
        };
        motor_typedef( imgui ) ;
        
        class imgui_view
        {
            motor_this_typedefs( imgui_view ) ;

        private:

            imgui_mtr_t _imres ;

        public:

            imgui_view( imgui_mtr_t im ) noexcept : _imres( im )
            {}

            imgui_view( this_cref_t rhv ) noexcept
            {
                _imres = rhv._imres ;
            }

            imgui_view( this_rref_t rhv ) noexcept
            {
                _imres = std::move( rhv._imres ) ;
            }

            this_ref_t operator = ( this_cref_t rhv ) noexcept
            {
                _imres = rhv._imres ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _imres = std::move( rhv._imres ) ;
                return *this ;
            }

        public:

            ImTextureID texture( motor::string_in_t s ) noexcept 
            {
                return _imres->texture( s ) ;
            }

        };
        motor_typedef( imgui_view ) ;
    }
}

static ImVec2 const operator + ( ImVec2 const & lhv, ImVec2 const & rhv ) noexcept
{
    return ImVec2( lhv.x + rhv.x, lhv.y + rhv.y ) ;
}

static ImVec2 const operator - ( ImVec2 const & lhv, ImVec2 const & rhv ) noexcept
{
    return ImVec2( lhv.x - rhv.x, lhv.y - rhv.y ) ;
}
