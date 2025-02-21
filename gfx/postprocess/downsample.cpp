
#include "downsample.h"

using namespace motor::gfx::postprocess ;

downsample::downsample( motor::string_in_t name ) noexcept 
{
}

downsample::downsample( this_rref_t ) noexcept 
{
}

downsample::~downsample( void_t ) noexcept 
{
}

void_t downsample::init( motor::math::vec2ui_in_t target_dims ) noexcept 
{
    // init quad
    {
        struct vertex { motor::math::vec2f_t pos ; } ;

        auto vb = motor::graphics::vertex_buffer_t()
            .add_layout_element( motor::graphics::vertex_attribute::position, motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 )
            .resize( 4 ).update<vertex>( [=] ( vertex * array, size_t const ne )
        {
            array[ 0 ].pos = motor::math::vec2f_t( -0.5f, -0.5f ) ;
            array[ 1 ].pos = motor::math::vec2f_t( -0.5f, +0.5f ) ;
            array[ 2 ].pos = motor::math::vec2f_t( +0.5f, +0.5f ) ;
            array[ 3 ].pos = motor::math::vec2f_t( +0.5f, -0.5f ) ;
        } );

        auto ib = motor::graphics::index_buffer_t().
            set_layout_element( motor::graphics::type::tuint ).resize( 6 ).
            update<uint_t>( [] ( uint_t * array, size_t const ne )
        {
            array[ 0 ] = 0 ;
            array[ 1 ] = 1 ;
            array[ 2 ] = 2 ;

            array[ 3 ] = 0 ;
            array[ 4 ] = 2 ;
            array[ 5 ] = 3 ;
        } ) ;

        _geo = motor::graphics::geometry_object_t( _name + ".quad",
            motor::graphics::primitive_type::triangles, std::move( vb ), std::move( ib ) ) ;
    }

    // init msl and shaders
    {
        motor::graphics::msl_object_t mslo( _name + ".post_quad" ) ;

        // shader
        {
            mslo.add( motor::graphics::msl_api_type::msl_4_0, R"(
            config post_quad
            {
                vertex_shader
                {
                    in vec2_t pos : position ;

                    out vec2_t tx : texcoord0 ;
                    out vec4_t pos : position ;

                    void main()
                    {
                        out.tx = sign( in.pos.xy ) *0.5 + 0.5 ;
                        out.pos = vec4_t( sign( in.pos.xy ), 0.0, 1.0 ) ; 
                    }
                }

                pixel_shader
                {
                    in vec2_t tx : texcoord0 ;
                    tex2d_t u_tex ;

                    void main()
                    {
                        out.color = vec4_t(0.5,0.5,0.5,1.0) ;
                    }
                }
            } )" ) ;

            mslo.link_geometry( _name + ".quad" ) ;
        }

        // variable sets
        {
            motor::graphics::variable_set_t vars ;

            {
                auto * var = vars.data_variable< motor::math::vec4f_t >( "color" ) ;
                var->set( motor::math::vec4f_t( 1.0f, 0.0f, 0.0f, 1.0f ) ) ;
            }

            {
                auto * var = vars.texture_variable( "u_tex" ) ;
                var->set( "some_texture_name_here...." ) ;
            }
            mslo.add_variable_set( motor::memory::create_ptr( std::move( vars ) ) ) ;
        }


        _msl = std::move( mslo ) ;
    }

    // framebuffer
    {
        motor::math::vec2ui_t dims = target_dims ;

        for ( size_t i = 0; i < 3; ++i )
        {
            dims /= 2 ;

            _fb[ i ] = motor::graphics::framebuffer_object_t( _name + ".render_target" ) ;
            _fb[ i ].set_target( motor::graphics::color_target_type::rgba_uint_8, 1 )
                .resize( target_dims.x(), target_dims.y() ) ;
        }
    }
}

void_t downsample::release( void_t ) noexcept 
{
}

void_t downsample::execute( void_t ) noexcept 
{
}