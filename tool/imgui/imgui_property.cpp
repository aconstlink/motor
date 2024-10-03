
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
            motor::string_t lable = name + "##" + sheet_name + "##property" + motor::to_string( id++ ) ;

            // handle int
            {
                auto * ptr = dynamic_cast<motor::property::int_property_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    auto const & mm = ptr->get_min_max() ;
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::slider:
                        ImGui::SliderInt( lable.c_str(), ptr->ptr(), mm.get_min(), mm.get_max() ) ;
                        break ;
                    default: break ;
                    }
                    return ;
                }
            }

            // handle float
            {
                auto * ptr = dynamic_cast<motor::property::float_property_ptr_t>( p ) ;
                if ( ptr != nullptr )
                {
                    auto const & mm = ptr->get_min_max() ;
                    switch ( ptr->get_hint() )
                    {
                    case motor::property::editor_hint::slider:
                        ImGui::SliderFloat( lable.c_str(), ptr->ptr(), mm.get_min(), mm.get_max() ) ;
                        break ;
                    default: break ;
                    }
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
                        if ( ImGui::InputText( lable.c_str(), buf, sizeof( buf ) ) )
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
                        if ( ImGui::BeginCombo( lable.c_str(), ptr->get_char_ptr(), 0 ) )
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