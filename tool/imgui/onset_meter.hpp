#pragma once

#include "../typedefs.h"
#include "imgui.h"
#include <motor/audio/util/onset_detector.hpp>

namespace motor::tool
{
using namespace motor::core::types;
class imgui_onset_meter
{
    motor_this_typedefs( imgui_onset_meter );

  private:

    float_t _display_max = 0.01f;

  public:

    void_t draw( char const * label, float_t const raw, motor::audio::onset_detector & detector,
        float_t & onset ) noexcept
    {
        this->draw_impl( label, raw, detector, onset );
    }

  private:

    void_t draw_impl( char const * label, float_t const raw,
        motor::audio::onset_detector & detector, float_t & onset ) noexcept
    {
        ImGui::PushID( this );
        ImGui::PushID( label );
        ImGui::BeginGroup();
        float_t const minimum = detector.minimum_level();
        float_t const filtered = detector.envelope_value();

        ImVec2 const origin = ImGui::GetCursorScreenPos();
        ImVec2 const label_size = ImGui::CalcTextSize( label );
        ImVec2 const size( std::max( 56.0f, label_size.x + 24.0f ), 180.0f );
        if( ImGui::InvisibleButton( "meter", size ) )
        {
            ImGui::OpenPopup( "settings" );
        }
        bool_t const hovered = ImGui::IsItemHovered();
        if( hovered ) ImGui::SetTooltip( "Onset detector settings" );

        // draw meter
        {
            auto * draw = ImGui::GetWindowDrawList();
            ImVec2 const lo( origin.x + 8.0f, origin.y + 4.0f );
            ImVec2 const hi( origin.x + size.x - 8.0f, origin.y + size.y - 4.0f );
            auto const y_for = [ & ]( float_t const value )
            {
                return hi.y - std::clamp( value / _display_max, 0.0f, 1.0f ) * ( hi.y - lo.y );
            };

            float_t const pulse = std::clamp( onset, 0.0f, 1.0f );
            ImVec4 background = ImGui::GetStyleColorVec4( ImGuiCol_FrameBg );
            background.x += ( 0.20f - background.x ) * pulse;
            background.y += ( 0.70f - background.y ) * pulse;
            background.z += ( 0.35f - background.z ) * pulse;
            draw->AddRectFilled( lo, hi, ImGui::GetColorU32( background ) );
            draw->AddRectFilled( ImVec2( lo.x, y_for( raw ) ), hi, IM_COL32( 48, 172, 205, 255 ) );
            draw->AddRect(
                lo, hi, ImGui::GetColorU32( hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Border ) );
            // The border remains visible even when the raw bar fills the meter.
            if( pulse > 0.001f )
            {
                draw->AddRect(
                    lo, hi, ImGui::GetColorU32( ImVec4( 0.35f, 1.0f, 0.55f, pulse ) ), 0.0f, 2.0f );
            }

            float_t text_y = origin.y + ( size.y - label_size.y ) * 0.5f;
            for( char const * line = label; *line != '\0'; )
            {
                char const * end = line;
                while( *end != '\0' && *end != '\n' ) ++end;
                float_t const width = ImGui::CalcTextSize( line, end ).x;
                draw->AddText( ImVec2( origin.x + ( size.x - width ) * 0.5f, text_y ),
                    IM_COL32( 235, 240, 245, 100 ), line, end );
                text_y += ImGui::GetTextLineHeight();
                line = *end == '\n' ? end + 1 : end;
            }

            float_t const minimum_y = y_for( minimum );
            draw->AddLine( ImVec2( lo.x + 3.0f, minimum_y ), ImVec2( hi.x - 3.0f, minimum_y ),
                IM_COL32( 245, 190, 65, 255 ), 2.0f );

            float_t const filtered_y = y_for( filtered );
            draw->AddLine( ImVec2( lo.x + 3.0f, filtered_y ), ImVec2( hi.x - 3.0f, filtered_y ),
                IM_COL32( 15, 18, 22, 255 ), 5.0f );
            draw->AddLine( ImVec2( lo.x + 3.0f, filtered_y ), ImVec2( hi.x - 3.0f, filtered_y ),
                IM_COL32( 245, 245, 245, 255 ), 2.0f );
        }

        if( ImGui::BeginPopup( "settings" ) )
        {
            ImGui::TextUnformatted( label );
            ImGui::Separator();
            ImGui::TextColored( ImVec4( 0.19f, 0.67f, 0.80f, 1.0f ), "Raw: %.6g", double_t( raw ) );
            ImGui::Text( "Filtered: %.6g", double_t( filtered ) );
            ImGui::TextColored(
                ImVec4( 0.96f, 0.75f, 0.25f, 1.0f ), "Minimum: %.6g", double_t( minimum ) );
            ImGui::Separator();
            ImGui::PushItemWidth( 200.0f );

            ImGui::SliderFloat( "Scale max", &_display_max, 0.000001f, 1.0f, "%.6f",
                ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp );

            ImGui::Separator();
            ImGui::TextUnformatted( "Onset detector" );
            {
                ImGui::PushID( "onset" );
                auto settings = detector.parameters();
                float_t onset_attack_ms = settings.env_dat.attack_sec * 1000.0f;
                float_t onset_release_ms = settings.env_dat.release_sec * 1000.0f;
                float_t reference_ms = settings.reference_sec * 1000.0f;
                auto const flags = ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp;

                bool_t edited = ImGui::SliderFloat(
                    "Attack", &onset_attack_ms, 0.1f, 1000.0f, "%.1f ms", flags );
                edited |= ImGui::SliderFloat(
                    "Release", &onset_release_ms, 0.1f, 2000.0f, "%.1f ms", flags );
                edited |= ImGui::SliderFloat(
                    "Reference", &reference_ms, 0.1f, 2000.0f, "%.1f ms", flags );
                edited |= ImGui::SliderFloat(
                    "Minimum", &settings.minimum_level, 0.00000001f, 1.0f, "%.8f", flags );
                edited |= ImGui::SliderFloat( "Trigger ratio", &settings.trigger_ratio, 1.02f,
                    10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp );
                settings.rearm_ratio =
                    std::min( settings.rearm_ratio, settings.trigger_ratio - 0.01f );
                edited |= ImGui::SliderFloat( "Rearm ratio", &settings.rearm_ratio, 1.0f,
                    settings.trigger_ratio - 0.01f, "%.2f", ImGuiSliderFlags_AlwaysClamp );
                if( edited )
                {
                    settings.env_dat.attack_sec = onset_attack_ms * 0.001f;
                    settings.env_dat.release_sec = onset_release_ms * 0.001f;
                    settings.reference_sec = reference_ms * 0.001f;
                    detector.set_parameters( settings );
                }
                // ImGui::Text( "Onsets: %d", onset_count );
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            if( raw > _display_max || filtered > _display_max || minimum > _display_max )
            {
                ImGui::TextUnformatted( "Above display range" );
            }
            ImGui::EndPopup();
        }
        ImGui::EndGroup();
        ImGui::PopID();
        ImGui::PopID();
    }
};
motor_typedef( imgui_onset_meter );
} // namespace motor::tool