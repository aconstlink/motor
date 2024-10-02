

#pragma once

#include "typedefs.h"

namespace motor
{
    namespace property
    {
        enum class editor_hint
        {
            check_box,
            slider,
            edit_box,
            combo_box,
            no_edit,
            max_entries
        };

        template< typename T, bool is_enum >
        struct default_editor_hint
        {
            static constexpr auto value = motor::property::editor_hint::no_edit ;
        };

        template< typename T >
        struct default_editor_hint< T, false >
        {
            static constexpr auto value = motor::property::editor_hint::no_edit ;
        };

        template< >
        struct default_editor_hint< motor::string_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::no_edit ;
        };

        template< typename T >
        struct default_editor_hint< T, true >
        {
            static constexpr auto value = motor::property::editor_hint::combo_box ;
        };

        template<>
        struct default_editor_hint< bool_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::check_box ;
        };

        template<>
        struct default_editor_hint< short_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::slider ;
        };

        template<>
        struct default_editor_hint< ushort_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::slider ;
        };

        template<>
        struct default_editor_hint< int_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::slider ;
        };

        template<>
        struct default_editor_hint< uint_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::slider ;
        };

        template<>
        struct default_editor_hint< float_t, false >
        {
            static constexpr auto value = motor::property::editor_hint::slider ;
        };
    }
}