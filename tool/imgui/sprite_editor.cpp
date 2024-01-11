
#include "sprite_editor.h"
#include "imgui_custom.h"

#include <motor/gfx/camera/pinhole_camera.h>
#include <motor/gfx/sprite/sprite_render_2d.h>
#include <motor/graphics/object/framebuffer_object.h>

#include <motor/format/future_items.hpp>
#include <motor/format/global.h>
#include <motor/format/motor/motor_structs.h>
#include <motor/format/motor/motor_module.h>

#include <motor/math/vector/vector2.hpp>

using namespace motor::tool ;

struct sprite_editor::sprite_render_pimpl
{
    bool_t initialized = false ;
    motor::graphics::framebuffer_object_res_t fb ;
    motor::gfx::sprite_render_2d_res_t sr ;
    motor::gfx::pinhole_camera_t cam ;
    motor::graphics::image_object_res_t sheets ;

    motor::graphics::image_t::dims_t dims ;

    motor::graphics::state_object_res_t so ;
};

sprite_editor::sprite_editor( void_t ) noexcept 
{
    _srp = motor::memory::global_t::alloc< sprite_editor::sprite_render_pimpl >() ;
}

sprite_editor::sprite_editor( motor::io::database_res_t db ) noexcept 
{
    _db = db ;
    _srp = motor::memory::global_t::alloc< sprite_editor::sprite_render_pimpl >() ;
}

sprite_editor::sprite_editor( this_rref_t rhv ) noexcept 
{
    _db = std::move( rhv._db ) ;
    _sprite_sheets = std::move( rhv._sprite_sheets ) ;
    _loads = std::move( rhv._loads ) ;
    motor_move_member_ptr( _srp, rhv ) ;
}

sprite_editor::~sprite_editor( void_t ) noexcept 
{
    motor::memory::global_t::dealloc( _srp ) ;
}

sprite_editor::this_ref_t sprite_editor::operator = ( this_rref_t rhv ) noexcept 
{
    _db = std::move( rhv._db ) ;
    _sprite_sheets = std::move( rhv._sprite_sheets ) ;
    _loads = std::move( rhv._loads ) ;

    return *this ;
}

// ****
void_t sprite_editor::add_sprite_sheet( motor::ntd::string_cref_t name, 
                motor::io::location_cref_t loc ) noexcept 
{
    this_t::load_item_t ss ;
    ss.disp_name = name ;
    ss.name = "motor.tool.sprite_editor." + name ;
    ss.loc = loc ;

    motor::format::module_registry_res_t mod_reg = motor::format::global_t::registry() ;
    ss.fitem = mod_reg->import_from( loc, _db ) ;

    _loads.emplace_back( std::move( ss ) ) ;
}

// ****
void_t sprite_editor::store( motor::io::database_res_t db ) noexcept 
{
    motor::format::module_registry_res_t mod_reg = motor::format::global_t::registry() ;

    motor::format::motor_document doc ;
        
    for( auto const & tss : _sprite_sheets )
    {
        motor::format::motor_document_t::sprite_sheet_t ss ;
        ss.name = tss.name ;

        {
                motor::format::motor_document_t::sprite_sheet_t::image_t img ;
                img.src = tss.img_loc.as_string() ;
                ss.image = img ;
        }

        ss.sprites.reserve( tss.sprites.size() ) ;
        for( auto const & ts : tss.sprites )
        {
            motor::format::motor_document_t::sprite_sheet_t::sprite_t sp ;
            sp.name = ts.name ;
            sp.animation.rect = tss.bounds[ ts.bound_idx ] ;
            sp.animation.pivot = tss.anim_pivots[ ts.pivot_idx ] ;

            ss.sprites.emplace_back( sp ) ;
        }
        
        for( auto const & ta : tss.animations )
        {
            motor::format::motor_document_t::sprite_sheet_t::animation_t ani ;
            ani.name = ta.name ;

            for( auto const & tf : ta.frames )
            {
                motor::format::motor_document_t::sprite_sheet_t::animation_t::frame_t fr ;
                fr.sprite = tss.sprites[ tf.sidx ].name ;
                fr.duration = tf.duration ;
                ani.frames.emplace_back( std::move( fr ) ) ;
            }

            ss.animations.emplace_back( std::move( ani ) ) ;
        }

        doc.sprite_sheets.emplace_back( ss ) ;
    }
        
    auto item = mod_reg->export_to( ss_loc, db, 
        motor::format::motor_item_res_t( motor::format::motor_item_t( std::move( doc ) ) ) ) ;

    motor::format::status_item_res_t r =  item.get() ;
}

// ****
void_t sprite_editor::do_tool( motor::tool::imgui_view_t imgui ) noexcept 
{
    // checking future items
    {
        auto loads = std::move(_loads) ;
        for( auto & item : loads )
        {
            auto const res = item.fitem.wait_for( std::chrono::milliseconds(0) ) ;
            if( res != std::future_status::ready )
            {
                _loads.emplace_back( std::move( item ) ) ;
                continue ;
            }

            motor::format::item_res_t ir = item.fitem.get() ;
            if( motor::format::item_res_t::castable<motor::format::image_item_res_t>(ir) )
            {
                motor::format::image_item_res_t ii = ir ;

                motor::graphics::image_t img = *ii->img ;
                
                auto iter = std::find_if( _sprite_sheets.begin(), _sprite_sheets.end(), [&]( this_t::sprite_sheet_cref_t sh )
                {
                    return sh.name == item.name ;
                } ) ;

                if( iter != _sprite_sheets.end() )
                {
                    iter->dims = motor::math::vec2ui_t( img.get_dims() ) ;

                    iter->img = motor::graphics::image_object_t( iter->name, std::move( img ) )
                        .set_wrap( motor::graphics::texture_wrap_mode::wrap_s, motor::graphics::texture_wrap_type::clamp_border )
                        .set_wrap( motor::graphics::texture_wrap_mode::wrap_t, motor::graphics::texture_wrap_type::clamp_border )
                        .set_filter( motor::graphics::texture_filter_mode::min_filter, motor::graphics::texture_filter_type::nearest )
                        .set_filter( motor::graphics::texture_filter_mode::mag_filter, motor::graphics::texture_filter_type::nearest );

                    iter->zoom = -0.5f ;
                    imgui.async().configure( iter->img ) ;
                }
            }
            else if( motor::format::item_res_t::castable<motor::format::motor_item_res_t>(ir) )
            {
                motor::format::motor_item_res_t ni = ir ;
                ss_loc = item.loc ;

                motor::format::motor_document_t doc = ni->doc ;
                
                for( auto const & ss : doc.sprite_sheets )
                {
                    this_t::sprite_sheet_t tss ;
                    tss.name = ss.name ;
                    tss.dname = ss.name ;
                    tss.dims = motor::math::vec2ui_t( 1 ) ;

                    // load image
                    {
                        this_t::load_item_t li ;
                        li.disp_name = ss.name ;
                        li.name = ss.name ;
                        li.loc = ss.image.src ; //motor::io::location_t::from_path( motor::io::path_t( ss.image.src ) ) ; 
                        
                        motor::format::module_registry_res_t mod_reg = motor::format::global_t::registry() ;
                        li.fitem = mod_reg->import_from( li.loc, _db ) ;

                        _loads.emplace_back( std::move( li ) ) ;

                        tss.img_loc = ss.image.src ;
                    }

                    for( auto const & s : ss.sprites )
                    {
                        this_t::sprite_sheet_t::sprite_t ts ;
                        ts.name = s.name ;
                        ts.bound_idx = tss.bounds.size() ;
                        ts.pivot_idx = tss.anim_pivots.size() ;

                        tss.sprites.emplace_back( std::move( ts ) ) ;
                        tss.bounds.emplace_back( s.animation.rect ) ;
                        tss.anim_pivots.emplace_back( s.animation.pivot ) ;
                    }

                    for( auto const & a : ss.animations )
                    {
                        this_t::sprite_sheet::animation_t ta ;
                        ta.name = a.name ;
                        for( auto const & f : a.frames )
                        {
                            this_t::sprite_sheet_t::animation_frame_t taf ;
                            taf.duration = f.duration ;
                            auto const iter = std::find_if( tss.sprites.begin(), tss.sprites.end(), [&]( this_t::sprite_sheet_t::sprite_cref_t s )
                            {
                                return s.name == f.sprite ;
                            }) ;
                            taf.sidx = std::distance( tss.sprites.begin(), iter ) ;

                            if( taf.sidx < tss.sprites.size() ) ta.frames.emplace_back( std::move( taf ) ) ;
                        }
                        tss.animations.emplace_back( std::move( ta ) ) ;
                    }

                    if( ss.animations.size() == 0 )
                    {
                        this_t::sprite_sheet::animation_t ta ;
                        ta.name = "empty" ;
                        tss.animations.emplace_back( std::move( ta ) ) ;
                    }

                    _sprite_sheets.emplace_back( std::move( tss ) ) ;
                }
            }
        }
    }

    if( !_srp->initialized )
    {
        _srp->cam = motor::gfx::pinhole_camera_t() ;
        _srp->cam.set_dims( 200.0f, 200.0f, 1.0f, 100.0f ) ;
        _srp->cam.orthographic() ;
        _srp->cam.look_at( motor::math::vec3f_t( 0.0f, 0.0f, -50.0f ),
                        motor::math::vec3f_t( 0.0f, 1.0f, 0.0f ), motor::math::vec3f_t( 0.0f, 0.0f, 0.0f )) ;
        
        // init framebuffer
        {
            motor::graphics::framebuffer_object_t fb( "sprite_editor.framebuffer" ) ;
            fb.set_target( motor::graphics::color_target_type::rgba_uint_8 ).resize( 512, 512 ) ;
            _srp->fb = motor::graphics::framebuffer_object_res_t( std::move( fb ) ) ;
            imgui.async().configure( _srp->fb ) ;
        }

        // make image with all sprite sheet layers
        {
            // taking all slices
            motor::graphics::image_t img ;

            // load each slice into the image
            for( auto & ss : _sprite_sheets )
            {
                if( ss.img.is_valid() ) img.append( ss.img->image() ) ;
            }

            motor::graphics::image_object_res_t ires = motor::graphics::image_object_t( 
                "sprite_editor.sheets", std::move( img ) )
                .set_type( motor::graphics::texture_type::texture_2d_array )
                .set_wrap( motor::graphics::texture_wrap_mode::wrap_s, motor::graphics::texture_wrap_type::repeat )
                .set_wrap( motor::graphics::texture_wrap_mode::wrap_t, motor::graphics::texture_wrap_type::repeat )
                .set_filter( motor::graphics::texture_filter_mode::min_filter, motor::graphics::texture_filter_type::nearest )
                .set_filter( motor::graphics::texture_filter_mode::mag_filter, motor::graphics::texture_filter_type::nearest );

            imgui.async().configure( ires ) ;

            _srp->sheets = std::move( ires ) ;
        }

        // init sprite_render
        {
            motor::graphics::async_views_t asyncs( {imgui.async()} ) ;
            motor::gfx::sprite_render_2d_t spr ;
            
            spr.init( "sprite_editor.sprite_render_2d", "sprite_editor.sheets",  asyncs ) ;
            //spr.set_texture( "sprite_editor.sheets" ) ;

            _srp->sr = motor::gfx::sprite_render_2d_res_t( std::move( spr ) ) ;
        }
        
        {
            motor::graphics::state_object_t so = motor::graphics::state_object_t(
                "sprite_editor.root_render_state" ) ;

            {
                motor::graphics::render_state_sets_t rss ;

                rss.depth_s.do_change = true ;
                rss.depth_s.ss.do_activate = false ;
                rss.depth_s.ss.do_depth_write = false ;

                //rss.polygon_s.do_change = true ;
                rss.polygon_s.ss.do_activate = true ;
                rss.polygon_s.ss.ff = motor::graphics::front_face::counter_clock_wise ;
                rss.polygon_s.ss.cm = motor::graphics::cull_mode::back ;
                rss.polygon_s.ss.fm = motor::graphics::fill_mode::fill ;
                   
                rss.clear_s.do_change = true ;
                rss.clear_s.ss.do_activate = true ;
                rss.clear_s.ss.do_depth_clear = true ;
                rss.clear_s.ss.do_color_clear = true ;

                rss.view_s.do_change = true ;
                rss.view_s.ss.do_activate = true ;
                rss.view_s.ss.vp = motor::math::vec4ui_t( 0,0, 512,512) ;
                so.add_render_state_set( rss ) ;
            }
            _srp->so = motor::graphics::state_object_res_t( std::move( so ) ) ;
            imgui.async().configure( _srp->so ) ;
        }

        _srp->initialized = true ;
    }

    // the order of the sprite sheets need to be preserved, so the 
    // data in the image needs to be cleared before any other
    // new image layer is appended 
    if( _srp->sheets->image().get_dims().z() < _sprite_sheets.size() )
    {
        auto & image = _srp->sheets->image() ;
        image.clear_data() ;
        for( size_t i=0; i<_sprite_sheets.size(); ++i )
        {
            auto const & ss = _sprite_sheets[i] ;
            if( !ss.img.is_valid() ) continue ;

            image.append( ss.img->image() ) ;
        }

        if( image.get_dims().z() > 0 )
        {
            imgui.async().configure( _srp->sheets ) ;
            _srp->dims = image.get_dims() ;
        }
    }

    ImGui::Begin( "Sprite Editor" ) ;

    if( _sprite_sheets.size() == 0 )
    {
        ImGui::Text( "No Sprite Sheets" ) ;
        ImGui::End() ;
        return ;
    }

    // if width or height is 1, the image is not loaded yet
    if( _sprite_sheets[_cur_item].dims.equal( motor::math::vec2ui_t(1) ).any() )
    {
        ImGui::Text( "Not ready yet" ) ;
        ImGui::End() ;
        return ;
    }

    {
        ImGui::BeginGroup() ;
        
        {
            if( ImGui::Button( "Export" ) )
            {
                this_t::store( _db ) ;
            }

            if( _cur_mode == this_t::mode::pivot )
            {
                ImGui::SameLine() ;
                if( ImGui::Button( "Reset Pivots" ) )
                {
                    for( auto const & s : _sprite_sheets[_cur_item].sprites )
                    {
                        auto const & b = _sprite_sheets[_cur_item].bounds[s.bound_idx] ;
                        _sprite_sheets[_cur_item].anim_pivots[s.pivot_idx] = (b.zw() + b.xy())/2 ;
                    }
                }
            }
            if( ImGui::Checkbox( "Play Animation", &_play_animation ) )
            {
            }
        }

        // sprite sheets list box
        {
            ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;
            ImGui::LabelText( "", "Sprite Sheets" ) ;

            motor::ntd::vector< const char * > names( _sprite_sheets.size() ) ;
            for( size_t i=0; i<_sprite_sheets.size(); ++i ) names[i] = _sprite_sheets[i].dname.c_str() ;
        
            ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;
            if( ImGui::ListBox( "##sprite_sheets", &_cur_item, names.data(), names.size() ) )
            {}

            {
                auto const & cur_sheet = _sprite_sheets[ _cur_item ] ;
                if( cur_sheet.animations.size() == 0 ) _cur_sel_ani = size_t( -1 ) ;
                _cur_sel_ani = std::min( _cur_sel_ani, cur_sheet.animations.size() - 1 ) ;
            }
        }

        _cur_hovered = size_t(-1) ;

        if( _cur_mode == this_t::mode::bounds )
        {
            ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;
            ImGui::LabelText( "", "Boxes" ) ;

            // bounding boxes in the list
            {
                size_t i = 0 ;
                motor::ntd::vector< motor::ntd::string_t > names( _sprite_sheets[_cur_item].sprites.size() ) ;
                for( auto const & s : _sprite_sheets[_cur_item].sprites )
                {
                    auto const & b = _sprite_sheets[_cur_item].bounds[s.bound_idx] ;
                    names[i] = std::to_string(i) + "( " + std::to_string(b.x()) + ", " + std::to_string(b.y()) + ", "
                        + std::to_string(b.z()) + ", " + std::to_string(b.w()) + " )";
                    names[i] = s.name ;
                    ++i ;
                }

                int sel = _cur_sel_box ;
                static size_t double_clicked = size_t(-1) ;
                ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;
                int hovered = -1 ;
                size_t item_edited = size_t(-1);

                {
                    if( motor::tool::imgui_custom::ListBox( "##bounds", &sel, &hovered, double_clicked, names, item_edited ) )
                    {
                    }
                    _cur_sel_box = sel ;
                }

                if( item_edited != size_t(-1) )
                {
                    auto & sprites = _sprite_sheets[_cur_item].sprites ;
                    auto const iter = std::find_if( sprites.begin(), sprites.end(), [&]( motor::tool::sprite_editor_t::sprite_sheet_t::sprite_cref_t s )
                    {
                        return s.name == names[item_edited] ;
                    } ) ;

                    // only empty or unique names
                    if( !names[item_edited].empty() && iter == sprites.end() )
                    {
                        _sprite_sheets[_cur_item].sprites[item_edited].name = names[item_edited] ;
                    }
                    sel = 0 ;
                    double_clicked = size_t(-1) ;
                }
                if( hovered != -1 )
                {
                    //motor::log::global_t::status( std::to_string(hovered) ) ;
                    _cur_hovered = size_t(hovered) ;
                }
            }
        }
        else if( _cur_mode == this_t::mode::pivot )
        {
            auto & cur_sheet = _sprite_sheets[_cur_item] ;

            // animations in the list
            {
                size_t i = 0 ;
                motor::ntd::vector< motor::ntd::string_t > names( _sprite_sheets[_cur_item].animations.size() ) ;
                for( auto const & a : cur_sheet.animations )
                {
                    names[i] = a.name + "##" + std::to_string(i) ;
                    ++i ;
                }

                int sel = int_t(_cur_sel_ani) ;
                static size_t double_clicked = size_t(-1) ;
                int hovered = -1 ;
                size_t item_edited = size_t(-1);

                {
                    ImGui::Text("Animations") ;
                    ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;

                    if( motor::tool::imgui_custom::ListBox( "##animations", &sel, &hovered, double_clicked, names, item_edited ) )
                    {
                    }
                    _cur_sel_ani = size_t( sel ) ; //std::min( size_t(  ), cur_sheet.animations.size()-1 ) ;
                }
            }

            // bounding boxes in the list for selected animation
            {
                _cur_sel_ani = std::min( _cur_sel_ani, cur_sheet.animations.size()-1 ) ;
                auto & animations = cur_sheet.animations ;
                auto & cur_ani = animations[_cur_sel_ani] ;

                size_t i = 0 ;
                motor::ntd::vector< std::pair< motor::ntd::string_t, bool_t > > names( cur_ani.frames.size() ) ;
                motor::ntd::vector< size_t > ids( cur_ani.frames.size() ) ;
                for( auto const & f : cur_ani.frames )
                {
                    if( f.sidx == size_t(-1) ) continue ;

                    auto const & s = cur_sheet.sprites[ f.sidx ] ;
                    names[i].first = s.name + "##frame_" + std::to_string(i) ;
                    names[i].second = false ;
                    ids[i] = f.sidx ;
                    ++i ;
                }

                for( auto & s : _pivot_ani_frame_sel )
                {
                    names[s].second = true ;
                }

                int sel = int_t( std::min( _cur_sel_frame, cur_ani.frames.size()-1) ) ; 
                static size_t double_clicked = size_t(-1) ;
                
                int hovered = int( _cur_hovered_frame_rel < cur_ani.frames.size() ? _cur_hovered_frame_rel :size_t(-1) ) ;
                size_t item_edited = size_t(-1);

                {
                    ImGui::Text("Frames") ;
                    
                    ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;

                    if( motor::tool::imgui_custom::ListBox( "##frames", &sel, &hovered, double_clicked, names, item_edited ) )
                    {}

                    {
                        _pivot_ani_frame_sel.clear() ;
                        for( size_t i=0; i<names.size(); ++i ) 
                        {
                            if( names[i].second ) _pivot_ani_frame_sel.emplace_back( i ) ;
                        }
                    }

                    // needs to be reset if mouse is not over the list
                    {
                        bool_t const list_hovered = ImGui::IsItemHovered() ;
                        if( !list_hovered ) 
                        {
                            hovered = -1 ;
                        }
                    }

                    _cur_sel_frame = std::min( size_t(sel), cur_ani.frames.size()-1) ;
                    _cur_hovered_frame_rel = hovered ;
                    _cur_hovered_frame = hovered != size_t(-1) ? ids[hovered] : size_t(-1) ;
                }
            }
        }
        else if( _cur_mode == this_t::mode::hit )
        {
        }
        else if( _cur_mode == this_t::mode::damage )
        {
        }
        else if( _cur_mode == this_t::mode::animation )
        {
            auto & cur_sheet = _sprite_sheets[_cur_item] ;

            // animations in the list
            {
                size_t i = 0 ;
                motor::ntd::vector< motor::ntd::string_t > names( _sprite_sheets[_cur_item].animations.size() ) ;
                for( auto const & a : cur_sheet.animations )
                {
                    names[i] = a.name + "##" + std::to_string(i) ;
                    ++i ;
                }

                int sel = int_t(_cur_sel_ani) ;
                static size_t double_clicked = size_t(-1) ;
                int hovered = -1 ;
                size_t item_edited = size_t(-1);

                {
                    ImGui::Text("Animations") ;
                    ImGui::SameLine() ;
                    if( ImGui::Button("+##addanimation") )
                    {
                        auto & animations = cur_sheet.animations ;
                        motor::tool::sprite_editor_t::sprite_sheet_t::animation_t a ;

                        size_t number = 0 ;
                        auto iter = animations.begin() ;
                        while( iter != animations.end() )
                        {
                            a.name = "new animation " + std::to_string( number++ ) ;

                            iter = std::find_if( animations.begin(), animations.end(), [&]( motor::tool::sprite_editor_t::sprite_sheet_t::animation_cref_t a_ )
                            {
                                return a_.name == a.name ;
                            } ) ;
                        }
                        animations.emplace_back( std::move( a ) ) ;
                    }
                    ImGui::SameLine() ;
                    if( ImGui::Button("-##removeanimation") )
                    {
                        // remove animation
                    }
                    ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;

                    if( motor::tool::imgui_custom::ListBox( "##animations", &sel, &hovered, double_clicked, names, item_edited ) )
                    {
                    }
                    _cur_sel_ani = size_t( sel ) ; //std::min( size_t(  ), cur_sheet.animations.size()-1 ) ;
                }

                if( item_edited != size_t(-1) )
                {
                    auto & animations = cur_sheet.animations ;
                    auto const iter = std::find_if( animations.begin(), animations.end(), [&]( motor::tool::sprite_editor_t::sprite_sheet_t::animation_cref_t a )
                    {
                        return a.name == names[item_edited] ;
                    } ) ;

                    // only empty or unique names
                    if( !names[item_edited].empty() && iter == animations.end() )
                    {
                        animations[item_edited].name = names[item_edited] ;
                    }
                    double_clicked = size_t(-1) ;
                }
                if( hovered != -1 )
                {
                    // need hover for animations
                    //_cur_hovered = size_t(hovered) ;
                }
            }

            // bounding boxes in the list for selected animation
            {
                _cur_sel_ani = std::min( _cur_sel_ani, cur_sheet.animations.size()-1 ) ;
                auto & animations = cur_sheet.animations ;
                auto & cur_ani = animations[_cur_sel_ani] ;

                size_t i = 0 ;
                motor::ntd::vector< std::pair< motor::ntd::string_t, int_t > > names_values( cur_ani.frames.size() ) ;
                motor::ntd::vector< size_t > ids( cur_ani.frames.size() ) ;
                for( auto const & f : cur_ani.frames )
                {
                    if( f.sidx == size_t(-1) ) continue ;

                    auto const & s = cur_sheet.sprites[ f.sidx ] ;
                    names_values[i].first = s.name + "##frame_" + std::to_string(i) ;
                    names_values[i].second = f.duration ;
                    ids[i] = f.sidx ;
                    ++i ;
                }

                int sel = int_t( std::min( _cur_sel_frame, cur_ani.frames.size()-1) ) ; 
                static size_t double_clicked = size_t(-1) ;
                
                int hovered = int( _cur_hovered_frame_rel < cur_ani.frames.size() ? _cur_hovered_frame_rel :size_t(-1) ) ;
                size_t item_edited = size_t(-1);

                {
                    ImGui::Text("Frames") ;
                    ImGui::SameLine() ;
                    if( ImGui::Button("+##addframe") )
                    {
                        if( cur_sheet.sprites.size() > 0 )
                        {
                            // add frame
                            motor::tool::sprite_editor_t::sprite_sheet_t::animation_frame_t f ;
                            f.duration = 50 ;
                            f.sidx = size_t(0) ;
                            cur_ani.frames.emplace_back( f ) ;
                        }
                    }
                    ImGui::SameLine() ;
                    if( ImGui::Button("-##removeframe") )
                    {
                        if( _cur_sel_frame != size_t(-1) )
                        {
                            auto const iter = cur_ani.frames.begin() + _cur_sel_frame ;
                            cur_ani.frames.erase( iter ) ;
                        }
                    }
                    ImGui::SetNextItemWidth( ImGui::GetWindowWidth() * 0.3f ) ;

                    //if( motor::tool::imgui_custom::ListBox( "##frames", &sel, &hovered, double_clicked, names, item_edited ) )
                    {
                        bool_t value_changed = false ;
                        if( motor::tool::imgui_custom::ListBoxWithInputInt( "##frames", &sel, &hovered, 
                            double_clicked, names_values, item_edited, value_changed ) )
                        {}

                        if( value_changed )
                        {
                            cur_ani.frames[sel].duration = names_values[sel].second ;
                        }
                    }

                    // needs to be reset if mouse is not over the list
                    {
                        bool_t const list_hovered = ImGui::IsItemHovered() ;
                        if( !list_hovered ) 
                        {
                            hovered = -1 ;
                        }
                    }

                    _cur_sel_frame = std::min( size_t(sel), cur_ani.frames.size()-1) ;

                    if( item_edited != size_t(-1) )
                    {
                        auto & frames = cur_ani.frames ;
                        auto & sprites = cur_sheet.sprites;
                        auto const iter = std::find_if( sprites.begin(), sprites.end(), 
                            [&]( motor::tool::sprite_editor_t::sprite_sheet_t::sprite_cref_t s )
                        {
                            return s.name == names_values[item_edited].first ;
                        } ) ;

                        // need to be a name from the sprites
                        if( iter != sprites.end() )
                        {
                            frames[item_edited].sidx = std::distance( sprites.begin(), iter ) ;
                        }
                        double_clicked = size_t(-1) ;
                    }
                    _cur_hovered_frame_rel = hovered ;
                    _cur_hovered_frame = hovered != size_t(-1) ? ids[hovered] : size_t(-1) ;
                }
            }
        }

        ImGui::EndGroup() ;
    }

    
    ImGui::SameLine() ; 

    // tabs
    {
        auto & ss = _sprite_sheets[_cur_item] ;

        ImGui::BeginGroup() ;

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            _croff = motor::math::vec2f_t( ImGui::GetCursorScreenPos().x,
                        ImGui::GetCursorScreenPos().y ) ;
            
            _crdims = motor::math::vec2f_t( 
                ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y ) * 
                    motor::math::vec2f_t( 0.99f, 0.99f ) ;
            
            this_t::handle_mouse( imgui, _cur_item ) ;

            if( ImGui::BeginTabItem("Bounds") )
            {
                this_t::handle_mouse_drag_for_bounds( _bounds_drag_info, ss.bounds ) ;
                _cur_mode = this_t::mode::bounds ;
                this_t::handle_mouse_drag_for_anim_pivot( _cur_item ) ;

                motor::math::vec4ui_t res ;
                if( this_t::handle_rect( _bounds_drag_info, res ) )
                {
                    this_t::sprite_sheet_t::sprite_t ts ;
                    ts.name = std::to_string( ss.sprites.size() ) ;
                    ts.bound_idx = ss.bounds.size() ;
                    ts.pivot_idx = ss.anim_pivots.size() ;
                    ss.sprites.emplace_back( std::move( ts ) ) ;

                    ss.bounds.emplace_back( res ) ;
                    ss.anim_pivots.emplace_back( res.xy() + (res.zw() - res.xy()) / motor::math::vec2ui_t(2) ) ;
                }

                this_t::show_image( imgui, _cur_item ) ;

                // draw rect for current mouse position
                {
                    motor::math::vec4f_t r( _cur_pixel, _cur_pixel ) ;
                    auto const rect = this_t::image_rect_to_window_rect( _cur_item, r ) ;
                    this_t::draw_rect_for_scale( rect ) ;
                }

                // draw rects
                {
                    auto const idx = this_t::draw_rects( ss.bounds, _cur_sel_box, _cur_hovered, motor::math::vec4ui_t(255, 255, 255, 150), 
                        motor::math::vec4ui_t(0, 0, 255, 150) ) ;

                    if( idx != size_t(-1) )
                    {
                        auto const r = this_t::image_rect_to_window_rect( _cur_item, ss.bounds[idx] ) ;
                        this_t::draw_scales( r, ss.bounds[idx] ) ;
                    }
                }

                // draw currently building rect when user
                // presses the mouse button
                if( _bounds_drag_info.mouse_down_rect )
                {
                    auto const rect0 = this_t::rearrange_mouse_rect( _bounds_drag_info.cur_rect ) ;
                    auto const rect = this_t::image_rect_to_window_rect( _cur_item, rect0 ) ;
                    this_t::draw_rect( rect ) ;
                }

                ImGui::EndTabItem() ;
            }

            if( ImGui::BeginTabItem("Pivot") )
            {
                _cur_mode = this_t::mode::pivot ;

                this_t::handle_mouse_drag_for_anim_pivot( _cur_item ) ;

                this_t::show_image( imgui, _cur_item ) ;
                bool_t const is_image_hovered = ImGui::IsItemHovered() ;

                // draw rect for current mouse position
                {
                    motor::math::vec4f_t r( _cur_pixel, _cur_pixel ) ;
                    auto const rect = this_t::image_rect_to_window_rect( _cur_item, r ) ;
                    this_t::draw_rect_for_scale( rect ) ;
                }

                // bounds in animation
                motor::ntd::vector< motor::math::vec4ui_t > bounds ;
                if( _cur_sel_ani != size_t(-1) && _sprite_sheets[_cur_item].animations.size() > 0 )
                {
                    auto const & sheet = _sprite_sheets[_cur_item] ;
                    auto const & animation = sheet.animations[_cur_sel_ani] ;
                    auto const & frames = animation.frames ;
                    bounds.reserve( frames.size() ) ;
                    for( auto const & f : frames )
                    {
                        auto const & bb = sheet.bounds[ sheet.sprites[ f.sidx ].bound_idx ] ;
                        bounds.emplace_back( bb ) ;
                    }
                }

                // draw rects
                {
                    auto const idx = this_t::draw_rects( ss.bounds, size_t(-1), _cur_hovered_frame, motor::math::vec4ui_t(150, 150, 150, 150), 
                        motor::math::vec4ui_t(0, 0, 255, 150) ) ;

                    if( idx != size_t(-1) )
                    {
                        if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && 
                            ImGui::IsKeyDown( ImGuiKey_LeftCtrl) )
                        {
                            // assign the currently clicked to the currently selected
                            int bp = 0 ;
                        }
                        else if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
                        {
                            _cur_sel_frame = idx ;
                        }

                        _cur_hovered_frame = idx ;
                    }
                    else if( is_image_hovered )
                    {
                        _cur_hovered_frame = size_t(-1) ;
                    }
                }

                // draw animation rects
                {
                    size_t const idx = this_t::draw_rects( bounds, _cur_sel_frame, _cur_hovered_frame_rel, motor::math::vec4ui_t(255, 255, 255, 150), 
                        motor::math::vec4ui_t(0, 255, 0, 150) ) ;
                    if( idx != size_t(-1) )
                    {
                        _cur_hovered_frame_rel = idx ;
                    }
                    else if( is_image_hovered )
                    {
                        _cur_hovered_frame_rel = size_t(-1) ;
                    }
                }

                // draw pivots
                {
                    this_t::draw_points( ss.anim_pivots ) ;
                }
                

                ImGui::EndTabItem() ;
            }

            if( ImGui::BeginTabItem("Hit") )
            {
                _cur_mode = this_t::mode::hit ;

                this_t::handle_mouse_drag_for_bounds( _hits_drag_info, ss.hits ) ;

                motor::math::vec4ui_t res ;
                if( this_t::handle_rect( _hits_drag_info, res ) )
                {
                    res = res ;
                    ss.hits.emplace_back( res ) ;
                }

                this_t::show_image( imgui, _cur_item ) ;
                
                // draw bound rects
                {
                    this_t::draw_rects( ss.bounds ) ;
                }

                // draw bound rects
                {
                    this_t::draw_rects( ss.hits, _cur_sel_box, _cur_hovered, motor::math::vec4ui_t(255, 0, 0, 150), 
                        motor::math::vec4ui_t(0, 255, 255, 150) ) ;
                }

                // draw currently building rect when user
                // presses the mouse button
                if( _hits_drag_info.mouse_down_rect )
                {
                    auto const rect0 = this_t::rearrange_mouse_rect( _hits_drag_info.cur_rect ) ;
                    auto const rect = this_t::image_rect_to_window_rect( _cur_item, rect0 ) ;
                    this_t::draw_rect( rect ) ;
                }

                ImGui::EndTabItem() ;
            }

            if( ImGui::BeginTabItem("Damage") )
            {
                _cur_mode = this_t::mode::damage ;

                this_t::handle_mouse_drag_for_bounds( _damages_drag_info, ss.damages ) ;

                motor::math::vec4ui_t res ;
                if( this_t::handle_rect( _damages_drag_info, res ) )
                {
                    res = res ;
                    ss.damages.emplace_back( res ) ;
                }

                this_t::show_image( imgui, _cur_item ) ;
                
                // draw bound rects
                {
                    this_t::draw_rects( ss.bounds ) ;
                }

                // draw bound rects
                {
                    this_t::draw_rects( ss.damages, _cur_sel_box, _cur_hovered, motor::math::vec4ui_t(255, 0, 0, 150), 
                        motor::math::vec4ui_t(0, 255, 255, 150) ) ;
                }

                // draw currently building rect when user
                // presses the mouse button
                if( _damages_drag_info.mouse_down_rect )
                {
                    auto const rect0 = this_t::rearrange_mouse_rect( _damages_drag_info.cur_rect ) ;
                    auto const rect = this_t::image_rect_to_window_rect( _cur_item, rect0 ) ;
                    this_t::draw_rect( rect ) ;
                }

                ImGui::EndTabItem() ;
            }

            if( ImGui::BeginTabItem("Animation") )
            {
                _cur_mode = this_t::mode::animation ;

                this_t::show_image( imgui, _cur_item ) ;

                bool_t const is_image_hovered = ImGui::IsItemHovered() ;

                // draw rect for current mouse position
                {
                    motor::math::vec4f_t r( _cur_pixel, _cur_pixel ) ;
                    auto const rect = this_t::image_rect_to_window_rect( _cur_item, r ) ;
                    this_t::draw_rect_for_scale( rect ) ;
                }

                // bounds in animation
                motor::ntd::vector< motor::math::vec4ui_t > bounds ;
                if( _cur_sel_ani != size_t(-1) && _sprite_sheets[_cur_item].animations.size() > 0 )
                {
                    auto const & sheet = _sprite_sheets[_cur_item] ;
                    auto const & animation = sheet.animations[_cur_sel_ani] ;
                    auto const & frames = animation.frames ;
                    bounds.reserve( frames.size() ) ;
                    for( auto const & f : frames )
                    {
                        auto const & bb = sheet.bounds[ sheet.sprites[ f.sidx ].bound_idx ] ;
                        bounds.emplace_back( bb ) ;
                    }
                }

                // draw rects
                {
                    auto const idx = this_t::draw_rects( ss.bounds, size_t(-1), _cur_hovered_frame, motor::math::vec4ui_t(150, 150, 150, 150), 
                        motor::math::vec4ui_t(0, 0, 255, 150) ) ;

                    if( idx != size_t(-1) )
                    {
                        if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) && 
                            ImGui::IsKeyDown( ImGuiKey_LeftCtrl) && 
                            _cur_sel_ani != size_t(-1) &&
                            _cur_sel_frame != size_t(-1) )
                        {
                            auto & cur_sheet = _sprite_sheets[_cur_item] ;
                            auto & cur_sprites = cur_sheet.sprites ;

                            auto const iter = std::find_if( cur_sprites.begin(), cur_sprites.end(), 
                                [&]( motor::tool::sprite_editor_t::sprite_sheet_t::sprite_cref_t s )
                                {
                                    return s.bound_idx == idx ;
                                } ) ;

                            if( iter == cur_sprites.end() )
                            {
                                motor::log::global_t::error("Upsy. Sprite not found for selected rect.") ;
                            }
                            else
                            {
                                cur_sheet.animations[_cur_sel_ani].frames[_cur_sel_frame].sidx = std::distance( cur_sprites.begin(), iter ) ;
                            }
                            
                        }
                        else if( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
                        {
                            _cur_sel_frame = idx ;
                        }

                        _cur_hovered_frame = idx ;
                    }
                    else if( is_image_hovered )
                    {
                        _cur_hovered_frame = size_t(-1) ;
                    }
                }

                // draw animation rects
                {
                    size_t const idx = this_t::draw_rects( bounds, _cur_sel_frame, _cur_hovered_frame_rel, motor::math::vec4ui_t(255, 255, 255, 150), 
                        motor::math::vec4ui_t(0, 255, 0, 150) ) ;
                    if( idx != size_t(-1) )
                    {
                        _cur_hovered_frame_rel = idx ;
                    }
                    else if( is_image_hovered )
                    {
                        _cur_hovered_frame_rel = size_t(-1) ;
                    }
                }

                ImGui::EndTabItem() ;
            }

            ImGui::EndTabBar() ;
        }

        ImGui::EndGroup() ;
    }


    
    #if 0
    {
        ImGui::BeginTooltip() ;
            
        //ImGui::Text("rect: (%.2f, %.2f), (%.2f, %.2f)", a.x, a.y, b.x, b.y ) ;
        //ImGui::Text("cur mouse: (%.2f, %.2f)", _cur_mouse.x(), _cur_mouse.y() ) ;
        ImGui::Text("image pixel: (%d, %d)", _cur_pixel.x(), _cur_pixel.y() ) ;
        //ImGui::Text("pixel ratio: 1 : %d", ratio ) ;

        ImGui::EndTooltip() ;
    }
    #endif

    ImGui::End() ;

    // second window for sprite observation based on
    // current mode
    {
        char buf[128];
        if( _cur_mode == this_t::mode::pivot )
        {
            sprintf( buf, "Pivot###ObservationWindow" );
        }
        else if( _cur_mode == this_t::mode::animation )
        {
            sprintf( buf, "Animation###ObservationWindow" );
        }
        else
        {
            sprintf( buf, "Observation Window###ObservationWindow" );
        }
        
        ImGui::Begin( buf ) ;
        if( _cur_sel_ani != size_t(-1) )
        {
            ImGui::Image( imgui.texture( "sprite_editor.framebuffer.0" ),
                ImGui::GetContentRegionAvail() ) ;
        }
        else
        {
            ImGui::Text("No animation selected") ;
        }
        
        ImGui::End() ;
    }

    if( _cur_mode == this_t::mode::pivot && !_play_animation)
    {
        auto & cur_sheet = _sprite_sheets[_cur_item] ;
        auto & cur_ani = cur_sheet.animations[_cur_sel_ani] ;

        for( auto const & idx : _pivot_ani_frame_sel )
        {
            if( idx >= cur_ani.frames.size() ) continue ;
            
            auto const b = cur_sheet.bounds[ cur_sheet.sprites[ cur_ani.frames[idx].sidx ].bound_idx ] ;
            auto const p = cur_sheet.anim_pivots[ cur_sheet.sprites[ cur_ani.frames[idx].sidx ].pivot_idx ] ;

            auto const p2 = ((motor::math::vec2f_t( p ) - motor::math::vec2f_t(b.xy())) - 
                            (motor::math::vec2f_t( b.zw() ) - motor::math::vec2f_t( b.xy() ) ) / motor::math::vec2f_t( 2 )).floor() ;

            motor::math::vec4f_t const rect = motor::math::vec4f_t( b ) / motor::math::vec4f_t( _srp->dims.xy(), _srp->dims.xy() ) ;
            motor::math::vec2f_t const pivot = motor::math::vec2f_t( p2 ) / motor::math::vec2f_t( _srp->dims.xy() ) ;

            _srp->sr->draw( 0, motor::math::vec2f_t(), motor::math::mat2f_t().identity(), 
                    motor::math::vec2f_t(3000.0f), rect, _cur_item, pivot, 
                motor::math::vec4f_t(1.0f,1.0f,1.0f,0.3f) ) ;
        }

        imgui.async().use( _srp->fb ) ;
        imgui.async().push( _srp->so ) ;
        _srp->sr->set_view_proj( _srp->cam.mat_view(), _srp->cam.mat_proj() ) ;
        _srp->sr->prepare_for_rendering() ;
        _srp->sr->render( 0 ) ;
        imgui.async().pop( motor::graphics::backend::pop_type::render_state ) ;

        imgui.async().unuse( motor::graphics::backend::unuse_type::framebuffer ) ;
    }
    else if( _cur_mode == this_t::mode::animation || _play_animation )
    {
        auto const & cur_sheet = _sprite_sheets[ _cur_item ] ;
        _cur_sel_ani = std::min( _cur_sel_ani, cur_sheet.animations.size() - 1 ) ;

        if( _cur_sel_ani != size_t(-1) )
        {
            auto & cur_sheet = _sprite_sheets[_cur_item] ;
            auto & cur_ani = cur_sheet.animations[_cur_sel_ani] ;
        
            static size_t milli = 0 ;
            static double time = ImGui::GetTime() ;
            double const dif = ImGui::GetTime() - time ;
            milli += size_t( dif * 1000.0 ) ;
            time = ImGui::GetTime() ;

            {
                size_t accum = 0 ;
                for( auto & f : cur_ani.frames ) accum += f.duration ;
                if( milli > accum ) milli = 0 ;
            }

            size_t idx = 0 ;
            if( cur_ani.frames.size() > idx )
            {
                if( idx >= cur_ani.frames.size() ) idx = 0 ;
                size_t accum = 0 ;
             
                for( size_t i=0; i<cur_ani.frames.size(); ++i ) 
                {
                    accum += cur_ani.frames[idx].duration ;

                    // if in animation mode, play whole animation
                    if( _cur_mode == this_t::mode::animation )
                    {
                        idx = i ;
                    }
                    else
                    {
                        // if pivot selection <= 1, play whole animation
                        // else just play multi-selected frames from selected animation
                        if( _pivot_ani_frame_sel.size() <= 1 )
                            idx = i ;
                        else
                            idx = _pivot_ani_frame_sel[i%_pivot_ani_frame_sel.size()] ;
                    }

                    if( milli < accum ) break ;
                }
            }

            if( cur_ani.frames.size() > idx )
            {
                auto const b = cur_sheet.bounds[ cur_sheet.sprites[ cur_ani.frames[idx].sidx ].bound_idx ] ;
                auto const p = cur_sheet.anim_pivots[ cur_sheet.sprites[ cur_ani.frames[idx].sidx ].pivot_idx ] ;

                auto const p2 = ((motor::math::vec2f_t( p ) - motor::math::vec2f_t(b.xy())) - 
                                (motor::math::vec2f_t( b.zw() ) - motor::math::vec2f_t( b.xy() ) ) / motor::math::vec2f_t( 2 )).floor() ;

                motor::math::vec4f_t const rect = motor::math::vec4f_t( b ) / motor::math::vec4f_t( _srp->dims.xy(), _srp->dims.xy() ) ;
                motor::math::vec2f_t const pivot = motor::math::vec2f_t( p2 ) / motor::math::vec2f_t( _srp->dims.xy() ) ;
            

                _srp->sr->draw( 0, motor::math::vec2f_t(), motor::math::mat2f_t().identity(), 
                    motor::math::vec2f_t(3000.0f), rect, _cur_item, pivot, motor::math::vec4f_t(1.0f) ) ;

            
                imgui.async().use( _srp->fb ) ;
                imgui.async().push( _srp->so ) ;
                _srp->sr->set_view_proj( _srp->cam.mat_view(), _srp->cam.mat_proj() ) ;
                _srp->sr->prepare_for_rendering() ;
                _srp->sr->render( 0 ) ;
                imgui.async().pop( motor::graphics::backend::pop_type::render_state ) ;

                imgui.async().unuse( motor::graphics::backend::unuse_type::framebuffer ) ;

            }
        }
    }
    
}

void_t sprite_editor::handle_mouse( motor::tool::imgui_view_t imgui, int_t const selected ) 
{
    ImGuiIO& io = ImGui::GetIO() ;
    auto & ss = _sprite_sheets[_cur_item] ;
        
    motor::math::vec2f_t const crdims = motor::math::vec2f_t( 
        ImGui::GetContentRegionAvail().x, 
        ImGui::GetContentRegionAvail().y ) * motor::math::vec2f_t( 0.99f, 0.99f ) ;

    motor::math::vec2f_t const idims( ss.dims ) ;

    // compute initial zoom
    // so the image exactly fits the content region
    if( ss.zoom < 0.0f )
    {
        auto const tmp = idims / crdims ;
        ss.zoom = std::max( tmp.x(), tmp.y() ) ;
    }
    
    motor::math::vec2f_t const mouse_in_window(
        io.MousePos.x - ImGui::GetCursorScreenPos().x,
        io.MousePos.y - ImGui::GetCursorScreenPos().y ) ;

    motor::math::vec2f_t const mouse_centered = 
            (mouse_in_window - crdims * motor::math::vec2f_t( 0.5f )) * motor::math::vec2f_t(1.0f,-1.0f) ;

    auto cur_mouse = ss.origin + mouse_centered * ss.zoom ;
            
    // keep the new origin in the image area
    {
        cur_mouse = cur_mouse.max_ed( idims * motor::math::vec2f_t( -0.5f ) ) ;
        cur_mouse = cur_mouse.min_ed( idims * motor::math::vec2f_t( +0.5f ) ) ;
    }

    bool_t const in_cr = 
        mouse_in_window.greater_than( motor::math::vec2f_t() ).all() &&
        mouse_in_window.less_than( crdims ).all() ;

    if( (io.MouseWheel > 0.0f) && in_cr )
    {
        ss.origin = cur_mouse ;
        ss.zoom *= io.KeyCtrl ? 0.5f : 0.9f ;
        ss.zoom = std::max( ss.zoom, 1.0f/200.0f ) ;
    }
    else if( (io.MouseWheel < 0.0f) && in_cr )
    {
        ss.origin = cur_mouse ;
        ss.zoom *= io.KeyCtrl ? 2.0f : 1.1f ;

        // recompute the initial zoom(izoom) 
        auto const tmp = idims / crdims ;
        float_t const izoom = std::max( tmp.x(), tmp.y() ) ;
                
        // allow out-zooming by only izoom * 2.0f
        // so it is not going too far out
        ss.zoom = std::min( ss.zoom, izoom * 2.0f ) ;

        // reset origin by pixel ratio
        int_t const ratio = std::floor( 1.0f / ss.zoom ) ;
        if( ratio <= std::floor( 1.0f / izoom ) )
        {
            ss.origin = motor::math::vec2f_t() ;
        }
    }
    
    if( io.MouseDown[2] && in_cr)
    {
        ss.origin += (cur_mouse - _cur_mouse) * motor::math::vec2f_t( -1.0f, -1.0f ) * 0.5f ;
    }

    // image pixel coord under mouse pos + transform y coord
    motor::math::vec2i_t const ip = (cur_mouse + idims * motor::math::vec2f_t( 0.5f, 0.5f )).floored() * motor::math::vec2f_t( 1.0f, 1.0f ) ;
    
    

    // tool tip infos
    // and remember values
    if( in_cr )
    {
        // 0.5f because dimensions are multiplided by 0.5f
        int_t const ratio = std::floor( 1.0f / ss.zoom ) ;
            
        _cur_pixel = ip ;
        _pixel_ratio = ratio ;
        _cur_mouse = cur_mouse ; //(cur_mouse + crdims * motor::math::vec2f_t( 0.5f, 0.5f ) )* motor::math::vec2f_t( 1.0f, 1.0f ) ;
        _cr_mouse = mouse_in_window ;
    }
    
}

void_t sprite_editor::handle_mouse_drag_for_bounds( this_t::rect_drag_info_ref_t info, motor::ntd::vector< motor::math::vec4ui_t > & rects ) 
{
    ImGuiIO& io = ImGui::GetIO() ;
    auto & ss = _sprite_sheets[_cur_item] ;

    motor::math::vec2f_t const crdims = motor::math::vec2f_t( 
        ImGui::GetContentRegionAvail().x, 
        ImGui::GetContentRegionAvail().y ) * motor::math::vec2f_t( 0.99f, 0.99f ) ;
    
    motor::math::vec2f_t const mouse_in_window(
        io.MousePos.x - ImGui::GetCursorScreenPos().x,
        io.MousePos.y - ImGui::GetCursorScreenPos().y ) ;

    bool_t const in_cr = 
        mouse_in_window.greater_than( motor::math::vec2f_t() ).all() &&
        mouse_in_window.less_than( crdims ).all() ;

    // handle mouse dragging
    {
        if( in_cr && !info.mouse_down_rect )
        {
            if( io.MouseDown[0] )
            {
                // start dragging
                if( !info.mouse_down_drag )
                {
                    size_t idx = size_t(-1) ;
                    for( size_t i=0; i<rects.size(); ++i )
                    {
                        if( this_t::is_ip_mouse_in_bound( rects[i] ) )
                        {
                            idx = i ;
                            break ;
                        }
                    }

                    if( idx != size_t(-1) ) 
                    {
                        info.drag_idx = idx ;
                        info.mouse_down_drag = true ;
                        info.drag_begin = _cur_pixel ;
                    }
                }
                // do dragging
                else if( info.drag_idx != size_t(-1) )
                {
                    std::array< bool_t, 8 > corners = {false,false,false,false,false,false,false,false} ;
                    bool_t drag_rect = !this_t::intersect_bound_location( info.drag_begin, rects[info.drag_idx], corners ) ;

                    auto dif = motor::math::vec2i_t( _cur_pixel ) - motor::math::vec2i_t( info.drag_begin ) ;

                    auto rect = motor::math::vec4ui_t( rects[info.drag_idx] ) ;

                    if( drag_rect )
                    {
                        auto xy0 = motor::math::vec2i_t( rect.xy() ) + dif ;
                        if( xy0.x() < 0 ) dif.x( dif.x() + -xy0.x() ) ;
                        if( xy0.y() < 0 ) dif.y( dif.y() + -xy0.y() ) ;
                        xy0 = motor::math::vec2i_t( rect.xy() ) + dif ;

                        auto const xy1 = rect.zw() + dif ;

                        rect = motor::math::vec4ui_t( xy0, xy1 ) ;
                    }
                    else if( corners[0] )
                    {
                        auto const xy0 = motor::math::vec2i_t( rect.xy() ) + dif ;
                        auto const xy1 = rect.zw() ;
                        rect = motor::math::vec4ui_t( xy0, xy1 ) ;
                    }
                    else if( corners[2] )
                    {
                        auto const xy = motor::math::vec2i_t( rect.xw() ) + dif ;
                        auto const xy0 = motor::math::vec2ui_t( xy.x(), rect.y() ) ;
                        auto const xy1 = motor::math::vec2ui_t( rect.z(), xy.y() ) ;
                        rect = motor::math::vec4ui_t( xy0, xy1 ) ;
                    }
                    else if( corners[4] )
                    {
                        auto const xy0 = rect.xy() ;
                        auto const xy1 = motor::math::vec2i_t( rect.zw() ) + dif ;
                        rect = motor::math::vec4ui_t( xy0, xy1 ) ;
                    }
                    else if( corners[6] )
                    {
                        auto const xy = motor::math::vec2i_t( rect.zy() ) + dif ;
                        auto const xy0 = motor::math::vec2ui_t( rect.x(), xy.y() ) ;
                        auto const xy1 = motor::math::vec2ui_t( xy.x(), rect.w() ) ;
                        rect = motor::math::vec4ui_t( xy0, xy1 ) ;
                    }

                    if( (motor::math::vec2i_t(rect.zw()) - motor::math::vec2i_t(rect.xy())).less_than( motor::math::vec2ui_t( 1, 1 ) ).any() ) 
                    {
                        return ;
                    }

                    rects[ info.drag_idx ] = rect ;

                    info.drag_begin = _cur_pixel ;
                }
            }
            else if( io.MouseReleased[0] && info.mouse_down_drag )
            {
                info.drag_idx = size_t(-1 ) ;
                info.mouse_down_drag = false ;
            }
        }
        else if( io.MouseReleased[0] && info.mouse_down_drag ) 
        {
            info.drag_idx = size_t(-1 ) ;
            info.mouse_down_drag = false ;
        }
    }
}

void_t sprite_editor::handle_mouse_drag_for_anim_pivot( int_t const selection ) 
{
    ImGuiIO& io = ImGui::GetIO() ;
    auto & ss = _sprite_sheets[selection] ;

    motor::math::vec2f_t const crdims = motor::math::vec2f_t( 
        ImGui::GetContentRegionAvail().x, 
        ImGui::GetContentRegionAvail().y ) * motor::math::vec2f_t( 0.99f, 0.99f ) ;
    
    motor::math::vec2f_t const mouse_in_window(
        io.MousePos.x - ImGui::GetCursorScreenPos().x,
        io.MousePos.y - ImGui::GetCursorScreenPos().y ) ;

    bool_t const in_cr = 
        mouse_in_window.greater_than( motor::math::vec2f_t() ).all() &&
        mouse_in_window.less_than( crdims ).all() ;

    if( !in_cr ) return ;

    if( io.MouseDown[0] )
    {
        // start dragging
        if( !_mouse_down_drag_anim )
        {
            size_t idx = size_t(-1) ;
            for( size_t i=0; i<ss.anim_pivots.size(); ++i )
            {
                //if( this_t::is_ip_mouse_in_bound( motor::math::vec4ui_t( ss.anim_pivots[i], ss.anim_pivots[i]) ) )
                if( this_t::is_ip_mouse_in_bound( ss.bounds[i] ) )
                {
                    idx = i ;
                    break ;
                }
            }

            if( idx != size_t(-1) ) 
            {
                _drag_idx_anim = idx ;
                _mouse_down_drag_anim = true ;
                _drag_begin_anim = _cur_pixel ;
            }
        }
        // do dragging
        else
        {
            ss.anim_pivots[_drag_idx_anim] += motor::math::vec2i_t(_cur_pixel) - motor::math::vec2i_t(_drag_begin_anim) ;
            _drag_begin_anim = _cur_pixel ;
        }
    }
    else if( _mouse_down_drag_anim )
    {
        _drag_idx_anim = size_t(-1) ;
        _mouse_down_drag_anim = false ;
    }
}

void_t sprite_editor::show_image( motor::tool::imgui_view_t imgui, int_t const selected ) 
{
    _screen_pos_image = motor::math::vec2f_t(
        ImGui::GetCursorScreenPos().x,ImGui::GetCursorScreenPos().y ) ;

    auto & ss = _sprite_sheets[ selected ] ;

    motor::math::vec2f_t const crdims = motor::math::vec2f_t( 
        ImGui::GetContentRegionAvail().x, 
        ImGui::GetContentRegionAvail().y ) * motor::math::vec2f_t( 0.99f, 0.99f ) ;

    motor::math::vec2f_t const idims( ss.dims ) ;

    motor::math::vec2f_t origin( ss.origin.x(), -ss.origin.y() ) ;

    // This is what we want to see!
    // world space window rect bottom left, top right
    motor::math::vec4f_t const wrect = motor::math::vec4f_t( crdims, crdims ) * 
        motor::math::vec4f_t( ss.zoom ) * motor::math::vec4f_t( -0.5f, -0.5f, 0.5f, 0.5f ) +
        motor::math::vec4f_t( origin, origin ) ;

    // world space image rect bottom left, top right
    motor::math::vec4f_t const irect = motor::math::vec4f_t( idims, idims ) * 
        motor::math::vec4f_t( -0.5f, -0.5f, 0.5f, 0.5f ) ;

    // uv rect bottom left, top right
    // one vector from bottom left image to bottom left window
    // one vector from top right image to top right window
    auto const uv_rect = motor::math::vec4f_t( 0.0f,0.0f,1.0f,1.0f ) + 
        (wrect - irect) / motor::math::vec4f_t( idims, idims ) ;

    ImGui::Image( imgui.texture( ss.name ), 
        ImVec2( crdims.x(), crdims.y() ), 
        ImVec2( uv_rect.x(), uv_rect.y()), 
        ImVec2( uv_rect.z(), uv_rect.w()),
        ImVec4( 1, 1, 1, 1),
        ImVec4( 1, 1, 1, 1) ) ;
}


bool_t sprite_editor::handle_rect( this_t::rect_drag_info_ref_t info, motor::math::vec4ui_ref_t res  ) 
{
    if( _bounds_drag_info.mouse_down_drag ) return false;

    ImGuiIO& io = ImGui::GetIO() ;
    auto & ss = _sprite_sheets[_cur_item] ;

    if( !ImGui::IsWindowHovered() ) return false ;
    
    motor::math::vec2f_t const crdims = motor::math::vec2f_t( 
        ImGui::GetContentRegionAvail().x, 
        ImGui::GetContentRegionAvail().y ) * motor::math::vec2f_t( 0.99f, 0.99f ) ;
    
    motor::math::vec2f_t const mouse_in_window(
        io.MousePos.x - ImGui::GetCursorScreenPos().x,
        io.MousePos.y - ImGui::GetCursorScreenPos().y ) ;

    bool_t const in_cr = 
        mouse_in_window.greater_than( motor::math::vec2f_t() ).all() &&
        mouse_in_window.less_than( crdims ).all() ;
        
    if( in_cr && io.MouseDown[0] && !info.mouse_down_rect )
    {
        info.mouse_down_rect = true ;
        info.cur_rect = motor::math::vec4ui_t( _cur_pixel, _cur_pixel ) ;
    }

    else if( io.MouseDown[0] )
    {        
        info.cur_rect.zw( _cur_pixel ) ;
        
        return false ;
    }

    else if( io.MouseReleased[0] && info.mouse_down_rect )
    {
        info.mouse_down_rect = false ;
        info.cur_rect.zw( _cur_pixel ) ;

        if( (info.cur_rect.zw() - info.cur_rect.xy()).less_than( motor::math::vec2ui_t(1,1) ).any() ) 
            return false ;

        res = this_t::rearrange_mouse_rect( info.cur_rect ) ;
        info.cur_rect = motor::math::vec4ui_t() ;

        return true ;
    }
    return false ;
}

motor::math::vec4ui_t sprite_editor::rearrange_mouse_rect( motor::math::vec4ui_cref_t vin ) const 
{
    uint_t const x0 = std::min( vin.x(), vin.z() ) ;
    uint_t const x1 = std::max( vin.x(), vin.z() ) ;

    uint_t const y0 = std::min( vin.y(), vin.w() ) ;
    uint_t const y1 = std::max( vin.y(), vin.w() ) ;

    return motor::math::vec4ui_t( x0, y0, x1, y1 ) ;
}

motor::math::vec4f_t sprite_editor::compute_cur_view_rect( int_t const selection ) const 
{
    auto & ss = _sprite_sheets[ selection ] ;

    // This is what we want to see!
    // world space window rect bottom left, top right
    return motor::math::vec4f_t( _crdims, _crdims ) * 
        motor::math::vec4f_t( ss.zoom ) * motor::math::vec4f_t( -0.5f, -0.5f, 0.5f, 0.5f ) +
        motor::math::vec4f_t( ss.origin, ss.origin ) ;
}

motor::math::vec4f_t sprite_editor::image_rect_to_window_rect( int_t const selection, 
                motor::math::vec4f_cref_t image_rect ) const
{
    auto & ss = _sprite_sheets[selection] ;

    auto const a = image_rect.xy() + motor::math::vec2f_t( ss.dims ) * motor::math::vec2f_t( -0.5f ) ;

    auto const b = a + (image_rect.zw() - image_rect.xy()) + 1 ;

    motor::math::vec4f_t const wrect = this_t::compute_cur_view_rect( selection ) ;
                                        
    auto const y = motor::math::vec2f_t( 0.0f, (wrect.zw() - wrect.xy()).y() ) ;

    motor::math::vec2f_t const xy = (a - wrect.xy() - y ) / ss.zoom ;
    motor::math::vec2f_t const wh = (b - a ).absed() / ss.zoom ;
                   
    motor::math::vec2f_t const xy_ = xy * motor::math::vec2f_t( 1.0f, -1.0f ) + _croff ;

    return motor::math::vec4f_t( xy_, wh ) ;
}

bool_t sprite_editor::is_window_rect_inside_content_region( motor::math::vec4f_ref_t rect, bool_t const fix_coords ) const 
{
    // remember: y grows downwards in imgui window space

    motor::math::vec2f_t const off = _croff ;

    // right/bottom completely outside
    if( rect.x() > (off + _crdims).x() ) return false ;
    if( rect.y() - rect.w() > (off + _crdims).y() ) return false ;
    
    // left/top completly outside
    if( rect.x() + rect.z() < off.x() ) return false ;
    if( rect.y() < off.y() ) return false ;
    
    if( !fix_coords ) return true ;

    float_t const x = std::max( rect.x(), off.x() ) ;
    float_t const y = std::min( rect.y(), off.y() + _crdims.y() ) ;

    float_t const w0 = rect.z() - ( x - rect.x() ) ;
    float_t const w1 = std::min( rect.x() + rect.z(), off.x() + _crdims.x() ) - rect.x() ;

    float_t const h0 = rect.w() - ( rect.y() - y ) ;
    float_t const h1 = rect.y() - std::max( rect.y() - rect.w(), off.y() ) ;

    rect = motor::math::vec4f_t( x, y, std::min( w0, w1 ), std::min( h0, h1 ) ) ;

    return true ;
}

void_t sprite_editor::draw_rect( motor::math::vec4f_cref_t rect, motor::math::vec4ui_cref_t color) 
{
    ImVec2 const a_( rect.x(), rect.y() + 1 ) ;
    ImVec2 const b_( rect.x() + rect.z() + 1, rect.y() - rect.w() + 1 ) ;

    ImGui::GetWindowDrawList()->AddRect( a_, b_, IM_COL32( color.x(), color.y(), color.z(), color.w() ) ) ;
    ImGui::GetWindowDrawList()->AddRectFilled( a_, b_, IM_COL32( color.x(), color.y(), color.z(), color.w() ) ) ;
}

void_t sprite_editor::draw_rect_info( motor::math::vec4f_cref_t rect, motor::math::vec4ui_cref_t img_rect ) noexcept
{
    ImGui::BeginTooltip() ;
    ImGui::Text( "Img Rect: %d %d %d %d", img_rect.x(), img_rect.y(), img_rect.z(), img_rect.w() ) ;
    ImGui::EndTooltip() ;
}

size_t sprite_editor::draw_rects( motor::ntd::vector< motor::math::vec4ui_t > const & rects, size_t const selected, size_t const hovered,
        motor::math::vec4ui_cref_t color, motor::math::vec4ui_cref_t over_color) 
{
    size_t ret = size_t( -1 ) ;

    for( size_t i=0; i<rects.size(); ++i )
    {
        auto rect = this_t::image_rect_to_window_rect( _cur_item, rects[i] ) ;

        if( !this_t::is_window_rect_inside_content_region( rect ) ) continue ;

        // if mouse on a rect, draw scales
        if( this_t::is_ip_mouse_in_bound( rects[i] ) )
        {
            this_t::draw_rect( rect, over_color ) ;
            this_t::draw_rect_info( rect, rects[i] ) ;
            ret = i ;
        }
        else if( hovered == i || selected == i )
            this_t::draw_rect( rect, over_color ) ;
        else
        {
            this_t::draw_rect( rect, color ) ;
        }
    }
    return ret ;
}

void_t sprite_editor::draw_points( motor::ntd::vector< motor::math::vec2ui_t > const & points, 
    motor::math::vec4ui_cref_t color ) 
{
    for( size_t i=0; i<points.size(); ++i )
    {
        motor::math::vec4f_t rect( points[i], points[i] ) ;
        rect = this_t::image_rect_to_window_rect( _cur_item, rect  ) ;

        if( !this_t::is_window_rect_inside_content_region( rect ) ) continue ;

        // if mouse on a rect, draw scales
        if( this_t::is_ip_mouse_in_bound( rect ) )
        {
            this_t::draw_rect( rect, motor::math::vec4ui_t(255,255,0,255) ) ;

            auto r = this_t::image_rect_to_window_rect( _cur_item, rect ) ;
            this_t::draw_scales( r, rect ) ;
        }
        else
        {
            this_t::draw_rect( rect, color ) ;
        }
    }
}

void_t sprite_editor::draw_rect_for_scale( motor::math::vec4f_cref_t rect, motor::math::vec4ui_cref_t color ) 
{
    ImVec2 const a_( rect.x(), rect.y() + 1 ) ;
    ImVec2 const b_( rect.x() + rect.z() + 1, rect.y() - rect.w() + 1 ) ;

    ImGui::GetWindowDrawList()->AddRectFilled( a_, b_, IM_COL32( color.x(), color.y(), color.z(), color.w() ) ) ;
}

void_t sprite_editor::draw_scales( motor::math::vec4f_cref_t rect, motor::math::vec4ui_cref_t prect, motor::math::vec4ui_t color ) 
{
    float_t const wh = float_t( _pixel_ratio ) * 1.0f ;

    std::array< bool_t, 8 > corners = {false,false,false,false,false,false,false,false} ;
    this_t::intersect_bound_location( _cur_pixel, prect, corners ) ;

    std::array< motor::math::vec4ui_t, 8 > colors = 
    {
        corners[0] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[1] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[2] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[3] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[4] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[5] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[6] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
        corners[7] ? motor::math::vec4ui_t( 255, 0, 0, 255  ) : color,
    } ;

    // bottom left
    {
        motor::math::vec4f_t r( rect.x(), rect.y(), wh, wh ) ;
        if( this_t::is_window_rect_inside_content_region( r, false ) )
        {
            this_t::draw_rect_for_scale( r, colors[0] ) ;
        }
    }
    // top left
    {
        motor::math::vec4f_t r( rect.x(), rect.y()-rect.w()+wh, wh, wh ) ;
        if( this_t::is_window_rect_inside_content_region( r, false ) )
        {
            this_t::draw_rect_for_scale( r, colors[2] ) ;
        }
    }

    // top right
    {
        motor::math::vec4f_t r( rect.x()+rect.z()-wh, rect.y()-rect.w()+wh, wh, wh ) ;
        if( this_t::is_window_rect_inside_content_region( r, false ) )
        {
            this_t::draw_rect_for_scale( r, colors[4] ) ;
        }
    }

    // bottom right
    {
        motor::math::vec4f_t r( rect.x()+rect.z()-wh, rect.y(), wh, wh ) ;
        if( this_t::is_window_rect_inside_content_region( r, false ) )
        {
            this_t::draw_rect_for_scale( r, colors[6] ) ;
        }
    }
}

bool_t sprite_editor::is_ip_mouse_in_bound( motor::math::vec4ui_cref_t rect ) const 
{
    if( uint_t( _cur_pixel.x() ) < rect.x() || uint_t( _cur_pixel.x() ) > rect.z() ) return false ;
    if( uint_t( _cur_pixel.y() ) < rect.y() || uint_t( _cur_pixel.y() ) > rect.w() ) return false ;
    return true ;
}

bool_t sprite_editor::intersect_bound_location( motor::math::vec2ui_cref_t cur_pixel, motor::math::vec4ui_cref_t rect, std::array< bool_t, 8 > & hit ) const 
{
    motor::math::vec2ui_t const cp = cur_pixel ;

    // bottom lect
    if( cp.equal( rect.xy() ).all() )
    {
        hit[0] = true ;
        return true ;
    }
    // top left
    else if( cp.equal( rect.xw() ).all() ) 
    {
        hit[2] = true ;
        return true ;
    }
    // top right
    else if( cp.equal( rect.zw() ).all() ) 
    {
        hit[4] = true ;
        return true ;
    }
    // bottom right
    else if( cp.equal( rect.zy() ).all() ) 
    {
        hit[6] = true ;
        return true ;
    }
    // left
    else if( cp.x() == rect.x() ) 
    {
        hit[1] = true ;
        return true ;
    }
    // top
    else if( cp.y() == rect.w() ) 
    {
        hit[3] = true ;
        return true ;
    }
    // right
    else if( cp.x() == rect.z() ) 
    {
        hit[5] = true ;
        return true ;
    }
    // bottom
    else if( cp.y() == rect.y() ) 
    {
        hit[7] = true ;
        return true ;
    }

    return false ;
}

void_t sprite_editor::display_animations_and_frames( void_t ) noexcept 
{
}