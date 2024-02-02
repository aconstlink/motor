#include "imgui.h"

#include <motor/math/camera/3d/orthographic_projection.hpp>

using namespace motor::tool ;

//***
imgui::imgui( void_t ) noexcept
{
    _vars.emplace_back( motor::memory::create_ptr( 
        motor::graphics::variable_set_t(), "imgui variable set" ) ) ;

    _ctx = ImGui::CreateContext() ;
    this_t::init() ;
}

//***
imgui::imgui( this_rref_t rhv ) noexcept
{
    motor_move_member_ptr( _ctx, rhv ) ;

    _rc = motor::move( rhv._rc ) ;
    _sc = motor::move( rhv._sc ) ;
    _gc = motor::move( rhv._gc ) ;
    _rs = motor::move( rhv._rs ) ;
    _ic = motor::move( rhv._ic ) ;

    _vars = std::move( rhv._vars ) ;
}

//***
imgui::~imgui( void_t ) noexcept
{
    if( _ctx != nullptr )
    {
        ImGui::DestroyContext(_ctx);
    }

    motor::memory::release_ptr( motor::move(_rc) ) ;
    motor::memory::release_ptr( motor::move(_sc) ) ;
    motor::memory::release_ptr( motor::move(_gc) ) ;
    motor::memory::release_ptr( motor::move(_rs) ) ;
    motor::memory::release_ptr( motor::move(_ic) ) ;
    
    for( auto * mtr : _vars )
        motor::memory::release_ptr( mtr ) ;
}

//***
void_t imgui::execute( exec_funk_t funk ) noexcept
{  
    auto * old = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;
    this_t::begin() ;
    funk() ;
    this_t::end() ;
    ImGui::SetCurrentContext( old ) ;
}

//***
void_t imgui::init( void_t ) noexcept 
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;

    // geometry
    {
        // @see struct vertex in the header
        auto vb = motor::graphics::vertex_buffer_t()
            .add_layout_element( motor::graphics::vertex_attribute::position, 
                motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 )

            .add_layout_element( motor::graphics::vertex_attribute::texcoord0, 
                motor::graphics::type::tfloat, motor::graphics::type_struct::vec2 )

            .add_layout_element( motor::graphics::vertex_attribute::color0, 
                motor::graphics::type::tfloat, motor::graphics::type_struct::vec4 ) ;

        auto ib = motor::graphics::index_buffer_t().
            set_layout_element( motor::graphics::type::tuint ) ;

        _gc = motor::memory::create_ptr( 
            motor::graphics::geometry_object_t( "motor.system.imgui",
            motor::graphics::primitive_type::triangles,
            std::move( vb ), std::move( ib ) ), "[imgui] : geometry object" ) ;
    }

    {
        _rs = motor::memory::create_ptr( 
            motor::graphics::state_object_t("motor.system.imgui"), "[imgui] : render states" ) ;
    }

    // shader config
    {
        motor::graphics::shader_object_t sc( "motor.system.imgui" ) ;

        // shaders : ogl 3.0
        {
            sc.insert( motor::graphics::shader_api_type::glsl_4_0, motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 140

                    in vec2 in_pos ;
                    in vec2 in_uv ;
                    in vec4 in_color ;

                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;

                    out vec2 var_uv ;
                    out vec4 var_color ;

                    void main()
                    {
                        var_uv = vec2( in_uv.x, 1.0 - in_uv.y ) ;
                        var_color = in_color ;
                        gl_Position = u_proj * vec4( in_pos, 0.0, 1.0 ) ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 140

                    out vec4 out_color ;
                    uniform sampler2D u_tex ;

                    in vec2 var_uv ;
                    in vec4 var_color ;

                    void main()
                    {    
                        out_color = var_color * texture( u_tex, var_uv ) ;
                    } )" ) ) 
            ) ;
        }

        // shaders : es 3.0
        {
            sc.insert( motor::graphics::shader_api_type::glsles_3_0, motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    layout( location = 0 ) in vec2 in_pos ;
                    layout( location = 1 ) in vec2 in_uv ;
                    layout( location = 2 ) in vec4 in_color ;

                    uniform mat4 u_proj ;
                    uniform mat4 u_view ;

                    out vec2 var_uv ;
                    out vec4 var_color ;

                    void main()
                    {
                        var_uv = vec2( in_uv.x, 1.0 - in_uv.y ) ;
                        var_color = in_color ;
                        gl_Position = u_proj * u_view * vec4( in_pos, 0.0, 1.0 ) ;
                    } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                    #version 300 es
                    precision mediump float ;
                    layout( location = 0 ) out vec4 out_color ;

                    uniform sampler2D u_tex ;

                    in vec2 var_uv ;
                    in vec4 var_color ;

                    void main()
                    {    
                        out_color = var_color * texture( u_tex, var_uv ) ;
                    } )" ) ) 
            ) ;
        }

        // shaders : hlsl 11
        {
            sc.insert( motor::graphics::shader_api_type::hlsl_5_0, motor::graphics::shader_set_t().

                set_vertex_shader( motor::graphics::shader_t( R"(
                            cbuffer Camera : register( b0 ) 
                            {
                                matrix u_proj ;
                                matrix u_view ;
                            }

                            struct VS_OUTPUT
                            {
                                float4 pos : SV_POSITION ;
                                float2 tx : TEXCOORD0 ;
                                float4 color : COLOR0 ;
                            };

                            VS_OUTPUT VS( float2 in_pos : POSITION, float2 in_uv : TEXCOORD0, 
                                        float4 in_color : COLOR0 )
                            {
                                float4 pos = float4( in_pos.xy, 0.0f, 1.0f ) ;
                                VS_OUTPUT output = (VS_OUTPUT)0;
                                output.pos = mul( pos, u_view );
                                output.pos = mul( output.pos, u_proj );
                                output.tx = float2( in_uv.x, 1.0 - in_uv.y ) ;
                                output.color = in_color ;
                                return output;
                            } )" ) ).

                set_pixel_shader( motor::graphics::shader_t( R"(
                            Texture2D u_tex : register( t0 );
                            SamplerState smp_u_tex : register( s0 );
                            float sys_flipv_u_tex ;

                            struct VS_OUTPUT
                            {
                                float4 pos : SV_POSITION ;
                                float2 tx : TEXCOORD0 ;
                                float4 color : COLOR0 ;
                            };

                            float4 PS( VS_OUTPUT input ) : SV_Target
                            {
                                float2 uv = input.tx ;
                                uv.y = lerp( uv.y, 1.0 - uv.y, sys_flipv_u_tex ) ;
                                return u_tex.Sample( smp_u_tex, uv ) * input.color ;
                            } )" 
                ) ) ) ;
        }

        // configure more details
        {
            sc
                .add_vertex_input_binding( motor::graphics::vertex_attribute::position, "in_pos" )
                .add_vertex_input_binding( motor::graphics::vertex_attribute::texcoord0, "in_uv" )
                .add_vertex_input_binding( motor::graphics::vertex_attribute::color0, "in_color" )
                .add_input_binding( motor::graphics::binding_point::view_matrix, "u_view" )
                .add_input_binding( motor::graphics::binding_point::projection_matrix, "u_proj" ) ;
        }

        _sc = motor::memory::create_ptr( std::move( sc ), "[imgui] : shader object" ) ;
    }

    // image configuration
    {
        ImGuiIO& io = ImGui::GetIO();

        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32( &pixels, &width, &height ) ;
        io.Fonts->TexID = (ImTextureID) 0 ;
        motor::graphics::image_t img = motor::graphics::image_t( motor::graphics::image_t::dims_t( width, height, 1 ) )
            .update( [&] ( motor::graphics::image_ptr_t, motor::graphics::image_t::dims_in_t dims, void_ptr_t data_in )
        {
            typedef motor::math::vector4< uint8_t > rgba_t ;
            auto* dst = reinterpret_cast< rgba_t* >( data_in ) ;
            auto* src = reinterpret_cast< rgba_t* >( pixels ) ;

            size_t const ne = dims.x() * dims.y() * dims.z() ;
            for( size_t i = 0; i < ne; ++i )
            {
                size_t const start = ne - width * ( ( i / width ) + 1 ) ;
                dst[ i ] = rgba_t( src[ start + i % width ] );
            }
        } ) ;

        auto ic = motor::graphics::image_object_t( "motor.system.imgui.font", std::move( img ) )
            .set_wrap( motor::graphics::texture_wrap_mode::wrap_s, motor::graphics::texture_wrap_type::clamp )
            .set_wrap( motor::graphics::texture_wrap_mode::wrap_t, motor::graphics::texture_wrap_type::clamp )
            .set_filter( motor::graphics::texture_filter_mode::min_filter, motor::graphics::texture_filter_type::linear )
            .set_filter( motor::graphics::texture_filter_mode::mag_filter, motor::graphics::texture_filter_type::linear ) ;

        _ic = motor::memory::create_ptr( std::move( ic ), "[imgui] : image object" );

    }

    // render configuration
    {
        motor::graphics::render_object_t rc( "motor.system.imgui" ) ;

        rc.link_geometry( "motor.system.imgui" ) ;
        rc.link_shader( "motor.system.imgui" ) ;
        
        // the variable set with id == 0 is the default
        // imgui variable set for rendering default widgets
        {
            auto* var = _vars[ 0 ]->texture_variable( "u_tex" ) ;
            var->set( "motor.system.imgui.font" ) ;
        }

        rc.add_variable_set( motor::share( _vars[0] ) ) ;

        _rc = motor::memory::create_ptr( std::move( rc ), "[imgui] : render object" )  ;
     }

    {
        this_t::do_default_imgui_init() ;

        ImGui::StyleColorsDark() ;
    }

    ImGui::SetCurrentContext( old_ctx ) ;
}

//***
void_t imgui::begin( void_t ) noexcept
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;
    ImGui::NewFrame() ;
    ImGui::SetCurrentContext( old_ctx ) ;
}

void_t imgui::end( void_t ) noexcept
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;
    ImGui::EndFrame() ;
    ImGui::SetCurrentContext( old_ctx ) ;
}

//***
void_t imgui::render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;

    if( !_init )
    {
        fe->configure<motor::graphics::state_object_t>( _rs ) ;
        fe->configure<motor::graphics::geometry_object_t>( _gc ) ;
        fe->configure<motor::graphics::shader_object_t>( _sc ) ;
        fe->configure<motor::graphics::image_object_t>( _ic ) ;
        fe->configure<motor::graphics::render_object_t>( _rc ) ;

        _init = true ;
    }

    motor::math::mat4f_t const proj = motor::math::m3d::orthographic<float_t>::create(
        float_t( _width ), float_t(_height), 1.0f, 10.0f ) ;

    for( auto & vars : _vars )
    {
        auto* var = vars->data_variable< motor::math::mat4f_t >( "u_proj" ) ;
        var->set( proj ) ;
    }
    
    ImGui::Render() ;
    ImDrawData* draw_data = ImGui::GetDrawData() ;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int_t const fb_width = ( int_t ) ( draw_data->DisplaySize.x * draw_data->FramebufferScale.x );
    int_t const fb_height = ( int_t ) ( draw_data->DisplaySize.y * draw_data->FramebufferScale.y );
    if( fb_width <= 0 || fb_height <= 0 )
        return;

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    {
        size_t size = 0 ;
        for( int n = 0; n < draw_data->CmdListsCount; n++ )
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[ n ];
            size += cmd_list->CmdBuffer.Size ;
        }

        {
            _rs->resize( size ) ;
            motor::graphics::render_state_sets_t rss ;

            rss.depth_s.do_change = true ;
            rss.depth_s.ss.do_activate = false ;
            rss.depth_s.ss.do_depth_write = false ;
            
            rss.polygon_s.do_change = true ;
            rss.polygon_s.ss.do_activate = false ;
            rss.polygon_s.ss.ff = motor::graphics::front_face::counter_clock_wise ;
            rss.polygon_s.ss.cm = motor::graphics::cull_mode::back ;
            rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill ;
            
            rss.scissor_s.do_change = true ;
            rss.scissor_s.ss.do_activate = true ;
            
            rss.blend_s.do_change = true ;
            rss.blend_s.ss.do_activate = true ;
            rss.blend_s.ss.src_blend_factor = motor::graphics::blend_factor::src_alpha ;
            rss.blend_s.ss.dst_blend_factor = motor::graphics::blend_factor::one_minus_src_alpha ;

            for( size_t i=0; i<_rs->size(); ++i )
            {
                _rs->set_render_states_set( i, rss ) ;
            }

            fe->configure<motor::graphics::state_object_t>( _rs ) ;
        }
    }

    // update geometry
    #if 1
    {
        _gc->vertex_buffer().resize( draw_data->TotalVtxCount ) ;
        _gc->index_buffer().resize( draw_data->TotalIdxCount ) ;

        size_t vb_off = 0 ;
        size_t ib_off = 0 ;

        // Update geometry: Everything is packed into a single vb/ib combo
        for( int n = 0; n < draw_data->CmdListsCount; n++ )
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[ n ];

            _gc->vertex_buffer().update<this_t::vertex>( [&] ( vertex* array, size_t const /*ne*/ )
            {
                for( int_t i = 0; i < cmd_list->VtxBuffer.Size; ++i )
                {
                    ImDrawVert const& imvert = cmd_list->VtxBuffer.Data[ i ] ;
                    auto const c = ImGui::ColorConvertU32ToFloat4( imvert.col ) ;

                    array[ vb_off + i ].pos = motor::math::vec2f_t(
                        imvert.pos.x - _width * 0.5f, -imvert.pos.y + _height * 0.5f ) ;
                    array[ vb_off + i ].uv = motor::math::vec2f_t( imvert.uv.x, imvert.uv.y ) ;
                    array[ vb_off + i ].color = motor::math::vec4f_t( c.x, c.y, c.z, c.w ) ;
                }
            } );

            _gc->index_buffer().update<uint_t>( [&] ( uint_t* array, size_t const /*ne*/ )
            {
                for( int_t i = 0 ; i < cmd_list->IdxBuffer.Size; ++i )
                {
                    array[ ib_off + i ] = uint_t( vb_off ) + cmd_list->IdxBuffer.Data[ i ] ;
                }
            } ) ;

            vb_off += cmd_list->VtxBuffer.Size ;
            ib_off += cmd_list->IdxBuffer.Size ;

            #if 0 // test. Let in until tested extensively
            cmd_list->VtxBuffer.Size = 4 ;
            cmd_list->IdxBuffer.Size = 6 ;
            ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ 0 ];
            pcmd->ElemCount = 6 ;
            #endif
        }
        fe->update( _gc ) ;
    }
    #else // test. Let in until tested extensively
    {
        _gc->vertex_buffer().resize( 4 ) ;
        _gc->index_buffer().resize( 6 ) ;

        _gc->vertex_buffer().update<this_t::vertex>( [&] ( vertex* array, size_t const /*ne*/ )
        {
            array[ 0 ].pos = motor::math::vec2f_t( -0.5f, -0.5f ) *300.0f;
            array[ 1 ].pos = motor::math::vec2f_t( -0.5f, +0.5f ) *300.0f;
            array[ 2 ].pos = motor::math::vec2f_t( +0.5f, +0.5f ) *300.0f;
            array[ 3 ].pos = motor::math::vec2f_t( +0.5f, -0.5f ) *300.0f;

            array[ 0 ].uv = motor::math::vec2f_t( -0.0f, -0.0f ) ;
            array[ 1 ].uv = motor::math::vec2f_t( -0.0f, +1.0f ) ;
            array[ 2 ].uv = motor::math::vec2f_t( +1.0f, +1.0f ) ;
            array[ 3 ].uv = motor::math::vec2f_t( +1.0f, -0.0f ) ;

            array[ 0 ].color = motor::math::vec4f_t( 1.0f,1.0f,1.0f,1.0f ) ;
            array[ 1 ].color = motor::math::vec4f_t( 1.0f,1.0f,1.0f,1.0f ) ;
            array[ 2 ].color = motor::math::vec4f_t( 1.0f,1.0f,1.0f,1.0f ) ;
            array[ 3 ].color = motor::math::vec4f_t( 1.0f,1.0f,1.0f,1.0f ) ;

        } );

        _gc->index_buffer().update<uint_t>( [&] ( uint_t* array, size_t const /*ne*/ )
        {
            array[ 0 ] = 0 ;
            array[ 1 ] = 1 ;
            array[ 2 ] = 2 ;

            array[ 3 ] = 0 ;
            array[ 4 ] = 2 ;
            array[ 5 ] = 3 ;
        } ) ;
       
        fe->update( motor::delay( _gc ) ) ;

        if( draw_data->CmdListsCount > 0 )
        {

        draw_data->CmdListsCount = 1 ;
        ImDrawList* cmd_list = draw_data->CmdLists[ 0 ];
        cmd_list->CmdBuffer.Size = 1 ;
        ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ 0 ];
        pcmd->ElemCount = 6 ;
        }
    }
    #endif

    size_t rs_id = 0 ;
    size_t offset = 0 ;
    

    // Render command lists
    for( int n = 0; n < draw_data->CmdListsCount; n++ )
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[ n ];

        for( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++ )
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[ cmd_i ];

            // Project scissor/clipping rectangles into framebuffer space
            ImVec4 clip_rect;
            clip_rect.x = ( pcmd->ClipRect.x - clip_off.x ) * clip_scale.x;
            clip_rect.y = ( pcmd->ClipRect.y - clip_off.y ) * clip_scale.y;
            clip_rect.z = ( pcmd->ClipRect.z - clip_off.x ) * clip_scale.x;
            clip_rect.w = ( pcmd->ClipRect.w - clip_off.y ) * clip_scale.y;

            if( _texture_added )
            {
                fe->configure<motor::graphics::render_object_t>( _rc ) ;
                _texture_added = false ;
            }

            if( clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f )
            {
                // change scissor
                {
                    _rs->access_render_state( rs_id,
                        [&] ( motor::graphics::render_state_sets_ref_t sets )
                    {
                        sets.scissor_s.ss.rect = motor::math::vec4ui_t( ( uint_t ) clip_rect.x, ( uint_t ) ( fb_height - clip_rect.w ), ( uint_t ) ( clip_rect.z - clip_rect.x ), ( uint_t ) ( clip_rect.w - clip_rect.y ) ) ;
                    } ) ;
                }

                {
                    fe->push( _rs, rs_id, false ) ;
                }

                // do rendering
                {
                    motor::graphics::gen4::backend_t::render_detail_t rd ;
                    rd.num_elems = pcmd->ElemCount ;
                    rd.start = offset ;
                    rd.varset = size_t( pcmd->TextureId ) ;
                    fe->render(_rc, rd ) ;
                }

                {
                    fe->pop( motor::graphics::gen4::backend::pop_type::render_state ) ;
                }
            }

            offset += pcmd->ElemCount ;
            rs_id++ ;
        }
    }

    ImGui::SetCurrentContext( old_ctx ) ;
}

//*****************************************************************************
void_t imgui::deinit( motor::graphics::gen4::frontend_mtr_t fe ) noexcept 
{
    fe->release( motor::move( _gc ) ) ;
    fe->release( motor::move( _rc ) ) ;
    fe->release( motor::move( _sc ) ) ;
    fe->release( motor::move( _ic ) ) ;
    fe->release( motor::move( _rs ) ) ;
}

//*****************************************************************************
void_t imgui::do_default_imgui_init( void_t ) 
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_motor";

    io.DisplaySize = ImVec2( float_t( _width ), float_t( _height ) ) ;

    io.ConfigWindowsMoveFromTitleBarOnly = true ;

    using key_t = motor::device::layouts::ascii_keyboard_t::ascii_key ;

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
#if 0
    io.KeyMap[ ImGuiKey_Tab ] = size_t( key_t::tab ) ;
    io.KeyMap[ ImGuiKey_LeftArrow ] = size_t( key_t::arrow_left );
    io.KeyMap[ ImGuiKey_RightArrow ] = size_t( key_t::arrow_right );
    io.KeyMap[ ImGuiKey_UpArrow ] = size_t( key_t::arrow_up );
    io.KeyMap[ ImGuiKey_DownArrow ] = size_t( key_t::arrow_down );
    io.KeyMap[ ImGuiKey_PageUp ] = size_t( key_t::one_up );
    io.KeyMap[ ImGuiKey_PageDown ] = size_t( key_t::one_down );
    //io.KeyMap[ ImGuiKey_Home ] = size_t( key_t::ho );
    io.KeyMap[ ImGuiKey_End ] = size_t( key_t::sk_end );
    io.KeyMap[ ImGuiKey_Insert ] = size_t( key_t::insert );
    io.KeyMap[ ImGuiKey_Delete ] = size_t( key_t::k_delete );
    io.KeyMap[ ImGuiKey_Backspace ] = size_t( key_t::back_space );
    io.KeyMap[ ImGuiKey_Enter ] = size_t( key_t::k_return );
    io.KeyMap[ ImGuiKey_Escape ] = size_t( key_t::escape );
    io.KeyMap[ ImGuiKey_Space ] = size_t( key_t::space );
    io.KeyMap[ ImGuiKey_A ] = size_t( key_t::a );
    io.KeyMap[ ImGuiKey_C ] = size_t( key_t::c );
    io.KeyMap[ ImGuiKey_V ] = size_t( key_t::v );
    io.KeyMap[ ImGuiKey_X ] = size_t( key_t::x );
    io.KeyMap[ ImGuiKey_Y ] = size_t( key_t::y );
    io.KeyMap[ ImGuiKey_Z ] = size_t( key_t::z );
#endif

    /*
    io.SetClipboardTextFn = ;
    io.GetClipboardTextFn = ;
    io.ClipboardUserData = ;
    */

    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return NULL and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
    /*GLFWerrorfun prev_error_callback = glfwSetErrorCallback( NULL );
    g_MouseCursors[ ImGuiMouseCursor_Arrow ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_TextInput ] = glfwCreateStandardCursor( GLFW_IBEAM_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_ResizeNS ] = glfwCreateStandardCursor( GLFW_VRESIZE_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_ResizeEW ] = glfwCreateStandardCursor( GLFW_HRESIZE_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_Hand ] = glfwCreateStandardCursor( GLFW_HAND_CURSOR );
    #if GLFW_HAS_NEW_CURSORS
    g_MouseCursors[ ImGuiMouseCursor_ResizeAll ] = glfwCreateStandardCursor( GLFW_RESIZE_ALL_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_ResizeNESW ] = glfwCreateStandardCursor( GLFW_RESIZE_NESW_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_ResizeNWSE ] = glfwCreateStandardCursor( GLFW_RESIZE_NWSE_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_NotAllowed ] = glfwCreateStandardCursor( GLFW_NOT_ALLOWED_CURSOR );
    #else
    g_MouseCursors[ ImGuiMouseCursor_ResizeAll ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_ResizeNESW ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_ResizeNWSE ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    g_MouseCursors[ ImGuiMouseCursor_NotAllowed ] = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    #endif*/

    ImGui::SetCurrentContext( old_ctx ) ;
}

//****
void_t imgui::update( window_data_cref_t data ) noexcept
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2( ( float_t ) data.width, ( float_t ) data.height );
    _width = data.width ;
    _height = data.height ;

    ImGui::SetCurrentContext( old_ctx ) ;
}

//****
void_t imgui::update( motor::device::three_device_borrow_t::mtr_t dev ) noexcept
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;

    ImGuiIO& io = ImGui::GetIO();
    
    // handle mouse
    if( dev != nullptr )
    {
        motor::device::three_device_t::layout_t mouse( dev ) ;
        // 1. mouse position
        {
            if( io.WantSetMousePos )
            {
                // @todo set mouse cursor
            }
            else
            {
                // transforms motor coords to imgui coords.
                // get_local origin : bottom left upwards
                // imgui needs origin : top left downwards
                motor::math::vec2f_t pos = ( mouse.get_local() -
                    motor::math::vec2f_t( 0.0f, 1.0f ) ) * motor::math::vec2f_t( 1.0f, -1.0f ) ;

                pos = pos * motor::math::vec2f_t(
                    float_t( io.DisplaySize.x ),
                    float_t( io.DisplaySize.y ) ) ;

                io.AddMousePosEvent( pos.x(), pos.y() ) ;
            }
        }

        // 2. buttons
        {
            using layout_t = motor::device::three_device_t::layout_t ;

            for( size_t i = 0; i < 3; ++i )
            {
                //io.MouseDown[ i ] = false ;
                
            }

            if( mouse.is_pressing( layout_t::button::left ) )
            {
                //io.MouseDown[ 0 ] = true ;
                io.AddMouseButtonEvent( 0, true ) ;
            }
            else if( mouse.is_released( layout_t::button::left ) )
            {
                //io.MouseDown[ 0 ] = true ;
                io.AddMouseButtonEvent( 0, false ) ;
            }

            if( mouse.is_pressing( layout_t::button::right ) )
            {
                //io.MouseDown[ 1 ] = true ;
                io.AddMouseButtonEvent( 1, true ) ;
            }
            if( mouse.is_pressing( layout_t::button::middle ) )
            {
                //io.MouseDown[ 2 ] = true ;
                io.AddMouseButtonEvent( 2, true ) ;
            }
        }

        // 3. wheel
        {
            //using layout_t = motor::device::three_device_t::layout_t ;

            float_t const m = 1.0f ; // io.KeyCtrl ? 1.0f : 2.5f ;
            float_t s ;
            if( mouse.get_scroll( s ) )
            {
                s = s / m ;
                if( io.KeyShift ) io.MouseWheelH = s ;
                else io.MouseWheel = s ;
            }
        }
    }

    ImGui::SetCurrentContext( old_ctx ) ;
}

//****
void_t imgui::update( motor::device::ascii_device_borrow_t::mtr_t dev ) noexcept
{
    auto * old_ctx = ImGui::GetCurrentContext() ;
    ImGui::SetCurrentContext( _ctx ) ;

    using ks_t = motor::device::components::key_state ;
    using layout_t = motor::device::layouts::ascii_keyboard_t ;
    using key_t = layout_t::ascii_key ;

    // handle keyboard
    if( dev == nullptr ) return ;

    ImGuiIO& io = ImGui::GetIO();
    //io.ClearInputKeys() ;

    motor::device::layouts::ascii_keyboard_t keyboard( dev ) ;

    bool_t shift = false ;
    bool_t alt = false ;
    bool_t ctrl = false ;

    for( size_t i = 0; i < size_t( key_t::num_keys ); ++i )
    {
        auto const ks = keyboard.get_state( key_t( i ) ) ;

        if( ks == ks_t::none ) continue ;

        if( key_t(i) == key_t::shift_left )
        {
            io.AddKeyEvent( ImGuiKey_LeftShift, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            shift = ks == ks_t::pressed || ks == ks_t::pressing ;
            //io.KeyShift = ks == ks_t::pressed || ks == ks_t::pressing ;
        }
        else if( key_t(i) == key_t::shift_right )
        {
            io.AddKeyEvent( ImGuiKey_RightShift, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            shift = ks == ks_t::pressed || ks == ks_t::pressing ;
            io.KeyShift = ks == ks_t::pressed || ks == ks_t::pressing ;
        }
        else if( key_t(i) == key_t::ctrl_left )
        {
            io.AddKeyEvent( ImGuiKey_LeftCtrl, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            ctrl = ks == ks_t::pressed || ks == ks_t::pressing ;
            io.KeyCtrl = ks == ks_t::pressed || ks == ks_t::pressing ;
        }
        else if( key_t(i) == key_t::ctrl_right )
        {
            io.AddKeyEvent( ImGuiKey_RightCtrl, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            ctrl = ks == ks_t::pressed || ks == ks_t::pressing ;
            io.KeyCtrl = ks == ks_t::pressed || ks == ks_t::pressing ;
        }
        else if( key_t(i) == key_t::alt_left )
        {
            io.AddKeyEvent( ImGuiKey_LeftAlt, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            alt = ks == ks_t::pressed || ks == ks_t::pressing ;
            io.KeyAlt = ks == ks_t::pressed || ks == ks_t::pressing ;
        }
        else if( key_t(i) == key_t::alt_right )
        {
            io.AddKeyEvent( ImGuiKey_RightAlt, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            alt = ks == ks_t::pressed || ks == ks_t::pressing ;
            io.KeyAlt = ks == ks_t::pressed || ks == ks_t::pressing ;
        }
    }

    for( size_t i = 0; i < size_t( key_t::num_keys ); ++i )
    {
        auto const ks = keyboard.get_state( key_t( i ) ) ;

        if( ks == ks_t::none ) continue ;
        
        if( layout_t::is_key_character( key_t(i) ) )
        {
            auto const ik = (ImGuiKey)( size_t( ImGuiKey_A ) + size_t( key_t::z ) - i ) ;
            io.AddKeyEvent( ik, ks == ks_t::pressed || ks == ks_t::pressing ) ;
            
            if( ks == ks_t::pressed )
            {
                char_t c ;
                layout_t::convert_key_to_ascii_char( shift, key_t( i ), c ) ;
                io.AddInputCharacter( c ) ;
            }
        }
        else if( layout_t::is_key_number( key_t(i) ) )
        {
            auto const ik = (ImGuiKey)( size_t( ImGuiKey_0 ) + size_t( key_t::k_9 ) - i ) ;
            io.AddKeyEvent( ik, ks == ks_t::pressed || ks == ks_t::pressing ) ;

            if( ks == ks_t::pressed )
            {
                char_t c ;
                layout_t::convert_key_to_ascii_number( alt, shift, key_t( i ), c ) ;
                io.AddInputCharacter( c ) ;
            }
        }        
        else if( layout_t::is_key_num_number( key_t(i) ) )
        {
            auto const ik = (ImGuiKey)( size_t( ImGuiKey_0 ) + size_t( key_t::num_9 ) - i ) ;
            io.AddKeyEvent( ik, ks == ks_t::pressed || ks == ks_t::pressing ) ;

            if( ks == ks_t::pressed )
            {
                char_t c ;
                layout_t::convert_key_to_ascii_num_number( key_t( i ), c ) ;
                io.AddInputCharacter( c ) ;
            }
        }
        else if( ks != ks_t::none && ks != ks_t::pressing )
        {
            size_t ii = ImGuiKey_None ;
            if( key_t(i) == key_t::space  ) 
            {
                ii = ImGuiKey_Space ;
                if( ks == ks_t::pressed )
                    io.AddInputCharacter( ' ' ) ;
            }
            else if( key_t(i) == key_t::k_return ) 
            {
                ii = ImGuiKey_Enter ;
            }
            else if( key_t(i) == key_t::escape ) 
            {
                ii = ImGuiKey_Escape ;
            }
            else if( key_t(i) == key_t::back_space ) 
            {
                ii = ImGuiKey_Backspace ;
            }
            else if( key_t(i) == key_t::minus && shift) 
            {
                ii = ImGuiKey_Minus ;
                if( ks == ks_t::pressed )
                    io.AddInputCharacter( '_' ) ;
            }
            else if( key_t(i) == key_t::minus) 
            {
                ii = ImGuiKey_Minus ;
                if( ks == ks_t::pressed )
                    io.AddInputCharacter( '-' ) ;
            }
            else if( key_t(i) == key_t::plus) 
            {
                if( ks == ks_t::pressed )
                    io.AddInputCharacter( '+' ) ;
            }

            io.AddKeyEvent( (ImGuiKey)ii, ks == ks_t::pressed ) ;
        }
    }

    ImGui::SetCurrentContext( old_ctx ) ;
}

//****
ImTextureID imgui::texture( motor::string_in_t name ) noexcept 
{
    size_t i = 0 ;

    for( auto& vars : _vars )
    {
        auto* var = vars->texture_variable( "u_tex" ) ;
        if( var->get() == name ) break ;

        i++ ;
    }

    // @note
    // places a new variable_set to the render configuration because
    // there is no other data that needs to be changed per variable_set,
    // except the data variable that are changed anyway.
    if( i == _vars.size() )
    {
        _vars.emplace_back( motor::memory::create_ptr( 
            motor::graphics::variable_set_t(), "[imgui] : variable set " ) ) ;
        _rc->add_variable_set( motor::share( _vars[ i ] ) ) ;
        auto * var = _vars[ i ]->texture_variable( "u_tex" ) ;
        var->set( name ) ;

        _texture_added = true ;
    }

    return ImTextureID(i) ;
}
