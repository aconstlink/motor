
#include "imgui_property.h"

#include <imgui.h>
#include <cstring>

using namespace motor::tool ;

//*************************************************************************
void_t imgui_property::handle( motor::string_in_t sheet_name, motor::property::property_sheet_inout_t props ) noexcept 
{
    ImGui::BeginGroup() ;
    {
        ImGui::SeparatorText( sheet_name.c_str() );

        int_t id = 0 ;
        props.for_each( [&] ( motor::string_cref_t name, motor::property::iproperty_mtr_t p )
        {
            motor::string_t label = name + "##" + sheet_name + "##property" + motor::to_string( id++ ) ;

            // handle int
            {
                auto * ptr = dynamic_cast<motor::property::int_traits_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    auto const & mm = ptr->get_min_max() ;
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::slider:
                        {
                            int_t v = ptr->get() ;
                            ImGui::SliderInt( label.c_str(), &v, mm.get_min(), mm.get_max() ) ;
                            ptr->set( v ) ;
                        }
                        break ;
                    default: break ;
                    }
                    return ;
                }
            }
            
            // handle float
            {
                auto * ptr = dynamic_cast<motor::property::float_traits_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    auto const & mm = ptr->get_min_max() ;
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::slider:
                        {
                            float_t v = ptr->get() ;
                            ImGui::SliderFloat( label.c_str(), &v, mm.get_min(), mm.get_max() ) ;
                            ptr->set( v ) ;
                        }
                        break ;
                    default: break ;
                    }
                    return ;
                }
            }

            // handle vector
            {
                auto * ptr = dynamic_cast<motor::property::vec2f_traits_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    auto mm = ptr->get_min_max() ;

                    ImGui::Text( name.c_str() ) ;
                    ImGui::BeginGroup() ;

                    // configuration
                    {
                        motor::string_t label_ = "Configuration ##" + label ;
                        if ( ImGui::CollapsingHeader( label_.c_str() ) )
                        {
                            auto max_ = mm.get_max() ;
                            auto min_ = mm.get_min() ;

                            {
                                ImGui::Text( "Max Value" ) ;
                                motor::string_t const label__ = "max ##" + label_ ;
                                {
                                    float_t x = max_.x() ;
                                    motor::string_t const label_ = "x ##" + label__ ;
                                    ImGui::SliderFloat( label_.c_str(), &x, 0.0f, 100.0f ) ;
                                    max_.x( x ) ;
                                }
                                {
                                    float_t y = max_.y() ;
                                    motor::string_t label_ = "y ##" + label__ ;
                                    ImGui::SliderFloat( label_.c_str(), &y, 0.0f, 100.0f ) ;
                                    max_.y( y ) ;
                                }
                            }
                            mm = motor::property::vec2f_traits_t::min_max_t( min_, max_ ) ;
                            ptr->set_min_max( mm ) ;
                        }
                    }

                    // values
                    {
                        motor::string_t label_ = "Values ##" + label ;
                        if ( ImGui::CollapsingHeader( label_.c_str() ) )
                        {
                            switch ( ptr->get_hint() )
                            {
                            case motor::property::editor_hint::slider:
                            {
                                auto v = ptr->get() ;

                                {
                                    float_t x = v.x() ;
                                    motor::string_t label_ = "x##" + label ;
                                    ImGui::SliderFloat( label_.c_str(), &x, mm.get_min().x(), mm.get_max().x() ) ;
                                    v.x( x ) ;
                                }
                                {
                                    float_t y = v.y() ;
                                    motor::string_t label_ = "y ##" + label ;
                                    ImGui::SliderFloat( label_.c_str(), &y, mm.get_min().y(), mm.get_max().y() ) ;
                                    v.y( y ) ;
                                }

                                ptr->set( v ) ;
                            }
                            break ;
                            default: break ;
                            }
                        }
                    }

                    ImGui::EndGroup() ;
                    return ;
                }
            }

            // handle vector
            {
                auto * ptr = dynamic_cast< motor::property::vec3f_traits_ptr_t >( p ) ;
                if( ptr != nullptr )
                {
                    auto const & mm = ptr->get_min_max() ;
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::slider:
                    {
                        auto v = ptr->get() ;

                        ImGui::BeginGroup() ;
                        ImGui::Text( name.c_str() ) ;
                        {
                            float_t x = v.x() ;
                            motor::string_t label_ = "x##" + label ;
                            ImGui::SliderFloat( label_.c_str(), &x, mm.get_min().x(), mm.get_max().x() ) ;
                            v.x( x ) ;
                        }
                        {
                            float_t y = v.y() ;
                            motor::string_t label_ = "y ##" + label ;
                            ImGui::SliderFloat( label_.c_str(), &y, mm.get_min().y(), mm.get_max().y() ) ;
                            v.y( y ) ;
                        }
                        {
                            float_t z = v.z() ;
                            motor::string_t label_ = "z ##" + label ;
                            ImGui::SliderFloat( label_.c_str(), &z, mm.get_min().z(), mm.get_max().z() ) ;
                            v.z( z ) ;
                        }
                        ImGui::EndGroup() ;
                        ptr->set( v ) ;
                    }
                    break ;
                    default: break ;
                    }
                    return ;
                }
            }


            // handle vector
            {
                auto * ptr = dynamic_cast<motor::property::vec4f_traits_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    ImGui::Text( name.c_str() ) ;
                    ImGui::BeginGroup() ;

                    auto mm = ptr->get_min_max() ;
                    {
                        motor::string_t label_ = "Configuration ##" + label ;
                        if( ImGui::CollapsingHeader( label_.c_str() ) )
                        {
                            auto max_ = mm.get_max() ;
                            auto min_ = mm.get_min() ;

                            {
                                ImGui::Text( "Max Value" ) ;
                                motor::string_t const label__ = "max ##" + label_ ;
                                {
                                    float_t x = max_.x() ;
                                    motor::string_t const label_ = "x ##" + label__ ;
                                    ImGui::SliderFloat( label_.c_str(), &x, 0.0f, 100.0f ) ;
                                    max_.x( x ) ;
                                }
                                {
                                    float_t y = max_.y() ;
                                    motor::string_t label_ = "y ##" + label__ ;
                                    ImGui::SliderFloat( label_.c_str(), &y, 0.0f, 100.0f ) ;
                                    max_.y( y ) ;
                                }
                                {
                                    float_t z = max_.z() ;
                                    motor::string_t label_ = "z ##" + label__ ;
                                    ImGui::SliderFloat( label_.c_str(), &z, 0.0f, 100.0f ) ;
                                    max_.z( z ) ;
                                }
                                {
                                    float_t w = max_.w() ;
                                    motor::string_t label_ = "w ##" + label__ ;
                                    ImGui::SliderFloat( label_.c_str(), &w, 0.0f, 100.0f ) ;
                                    max_.w( w ) ;
                                }
                            }
                            mm = motor::property::vec4f_traits_t::min_max_t( min_, max_ ) ;
                            ptr->set_min_max( mm ) ;
                        }
                    }


                    {
                        motor::string_t const label_ = "Values##"+label ;
                        if ( ImGui::CollapsingHeader( label_.c_str() ) )
                        {
                            switch ( ptr->get_hint() )
                            {
                            case motor::property::editor_hint::slider:
                            {
                                auto v = ptr->get() ;
                                {
                                    float_t x = v.x() ;
                                    motor::string_t label_ = "x ##" + label ;
                                    ImGui::SliderFloat( label_.c_str(), &x, mm.get_min().x(), mm.get_max().x() ) ;
                                    v.x( x ) ;
                                }
                                {
                                    float_t y = v.y() ;
                                    motor::string_t label_ = "y ##" + label ;
                                    ImGui::SliderFloat( label_.c_str(), &y, mm.get_min().y(), mm.get_max().y() ) ;
                                    v.y( y ) ;
                                }
                                {
                                    float_t z = v.z() ;
                                    motor::string_t label_ = "z ##" + label ;
                                    ImGui::SliderFloat( label_.c_str(), &z, mm.get_min().z(), mm.get_max().z() ) ;
                                    v.z( z ) ;
                                }
                                {
                                    float_t w = v.w() ;
                                    motor::string_t label_ = "w ##" + label ;
                                    ImGui::SliderFloat( label_.c_str(), &w, mm.get_min().w(), mm.get_max().w() ) ;
                                    v.w( w ) ;
                                }
                                ptr->set( v ) ;
                            }
                            break ;
                            default: break ;
                            }
                        }
                    }

                    ImGui::EndGroup() ;
                    return ;
                }
            }

            // handle string
            {
                auto * ptr = dynamic_cast<motor::property::string_property_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::no_edit:
                    {
                        ImGui::Text( ptr->get().c_str() ) ;
                        if ( ImGui::IsItemClicked() )
                        {
                            ptr->set_hint( motor::property::editor_hint::edit_box ) ;
                        }
                        break ;
                    }
                    case motor::property::editor_hint::edit_box:
                    {
                        char buf[ 256 ] ;
                        size_t const s = std::min( sizeof( buf ), ptr->get().size() ) ;
                        std::memcpy( buf, ptr->get().c_str(), s ) ;
                        buf[ s ] = '\0' ;
                        if ( ImGui::InputText( label.c_str(), buf, sizeof( buf ) ) )
                        {
                            ptr->set( motor::string_t( buf ) ) ;
                        }
                        if ( ImGui::IsKeyPressed( ImGuiKey_Enter ) )
                        {
                            ptr->set_hint( motor::property::editor_hint::no_edit ) ;
                        }
                        break ;
                    }
                    default: break ;
                    }
                    return ;
                }
            }
            
            // handle enum
            {
                auto * ptr = dynamic_cast<motor::property::any_enum_property_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::combo_box:
                    {
                        if ( ImGui::BeginCombo( label.c_str(), ptr->get_char_ptr(), 0 ) )
                        {
                            size_t sel = ptr->as_index() ;
                            for ( size_t n = 0; n < ptr->get_strings().second; n++ )
                            {
                                const bool is_selected = ( sel == n );
                                if ( ImGui::Selectable( ptr->get_strings().first[ n ], is_selected ) )
                                    sel = n;

                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if ( is_selected )
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo() ;
                            ptr->set_by_index( sel ) ;
                        }
                    }
                    break ;
                    default: break ;
                    }
                    return ;
                }
            }
        } ) ;
    }
    ImGui::EndGroup() ;
}