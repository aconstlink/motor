
#include "imgui_custom.h"

#include "imgui_internal.h"

#include <motor/memory/malloc_guard.hpp>

using namespace ImGui ;

bool motor::tool::imgui_custom::ListBox(const char* label, int* current_item, int* hovered_item, size_t & double_clicked, 
    motor::vector<motor::string_t> & items, size_t & item_edited, int height_items)
{
    bool_t selected = false ;

    if( ImGui::BeginListBox( label ) )
    {
        for( size_t i=0; i<items.size(); ++i )
        {
            motor::memory::malloc_guard<char_t> m( items[i].c_str(), items[i].size()+1 ) ;
            selected = i == *current_item ;
            
            if( double_clicked == i )
            {
                motor::memory::malloc_guard<char_t> m2( 256 ) ;
                m2[0] = '\0' ;

                ImGui::SetKeyboardFocusHere() ;
                if( ImGui::InputText( m, m2, 256 ) )
                {}
                

                if( ImGui::IsKeyPressed( ImGuiKey_Enter ) )
                {
                    double_clicked = size_t( -1 ) ;
                    items[i] = motor::string_t( m2 ) ;
                    item_edited = i ;
                }
                else if( ImGui::IsKeyPressed( ImGuiKey_Escape ) )
                {
                    double_clicked = size_t( -1 ) ;
                }
            }
            else if( ImGui::Selectable( m, &selected, ImGuiSelectableFlags_AllowDoubleClick ) )
            {
                if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
                {
                    double_clicked = i ;
                }
                else *current_item = i ;
            }

            if( ImGui::IsItemHovered() )
            {
                *hovered_item = i ;
            }
        }

        EndListBox() ;
    }
    return selected ;
}

bool motor::tool::imgui_custom::ListBox(const char* label, int* current_item, int* hovered_item, size_t & double_clicked, 
                motor::vector<std::pair<motor::string_t, bool> > & items, size_t & item_edited, int height_in_items )
{
    bool_t any_selected = false ;

    if( ImGui::BeginListBox( label ) )
    {
        for( size_t i=0; i<items.size(); ++i )
        {
            bool_t selected = items[i].second ;

            motor::memory::malloc_guard<char_t> m( items[i].first.c_str(), items[i].first.size()+1 ) ;
            //selected = i == *current_item ;
            
            if( double_clicked == i )
            {
                motor::memory::malloc_guard<char_t> m2( 256 ) ;
                m2[0] = '\0' ;

                ImGui::SetKeyboardFocusHere() ;
                if( ImGui::InputText( m, m2, 256 ) )
                {}
                

                if( ImGui::IsKeyPressed( ImGuiKey_Enter ) )
                {
                    double_clicked = size_t( -1 ) ;
                    items[i].first = motor::string_t( m2 ) ;
                    item_edited = i ;
                }
                else if( ImGui::IsKeyPressed( ImGuiKey_Escape ) )
                {
                    double_clicked = size_t( -1 ) ;
                }
            }
            else if( ImGui::Selectable( m, &selected, ImGuiSelectableFlags_AllowDoubleClick ) )
            {
                if( ImGui::GetIO().KeyCtrl )
                {
                    items[i].second = selected ;
                    *current_item = i ;
                }
                else if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
                {
                    double_clicked = i ;
                }
                else 
                {
                    *current_item = i ;
                    for( auto & item :  items ) item.second = false ;
                    items[i].second = true ;
                }
            }

            if( ImGui::IsItemHovered() )
            {
                *hovered_item = i ;
            }
            any_selected = selected || any_selected ? true : false ;
        }

        EndListBox() ;
    }
    return any_selected ;
}

bool motor::tool::imgui_custom::ListBoxWithInputInt( const char* label, int* selected_item, int* hovered_item, size_t & double_clicked, 
    motor::vector< std::pair< motor::string_t, int > > & items, size_t & item_edited, bool & value_changed, int height_items )
{
    bool_t selected = false ;
    bool_t hovered = false ;

    if( ImGui::BeginListBox( label ) )
    {
        for( size_t i=0; i<items.size(); ++i )
        {
            selected = i == *selected_item ;
            hovered = i == *hovered_item ;

            {
                ImVec2 const item_dims = ImVec2( ImGui::GetContentRegionAvail().x * 0.6f,
                    ImGui::GetItemRectSize().y * 1.0f ) ;
                ImGui::SetNextItemWidth( item_dims.x ) ;

                motor::memory::malloc_guard<char_t> m( items[i].first.c_str(), items[i].first.size()+1 ) ;
                if( double_clicked == i )
                {
                    motor::memory::malloc_guard<char_t> m2( 256 ) ;
                    m2[0] = '\0' ;

                    ImGui::SetKeyboardFocusHere() ;
                    if( ImGui::InputText( m, m2, 256 ) )
                    {}
                

                    if( ImGui::IsKeyPressed( ImGuiKey_Enter ) )
                    {
                        double_clicked = size_t( -1 ) ;
                        items[i].first = motor::string_t( m2 ) ;
                        item_edited = i ;
                    }
                    else if( ImGui::IsKeyPressed( ImGuiKey_Escape ) )
                    {
                        double_clicked = size_t( -1 ) ;
                    }
                }
                else if( motor::tool::imgui_custom::Selectable( m, &selected, &hovered, ImGuiSelectableFlags_AllowDoubleClick, item_dims ) )
                {
                    if( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
                    {
                        double_clicked = i ;
                    }
                    else *selected_item = i ;
                }
                if( ImGui::IsItemHovered() )
                {
                    *hovered_item = i ;
                }
            }

            if( double_clicked != i )
            {
                ImVec2 const item_dims = ImVec2( ImGui::GetContentRegionAvail().x * 0.4f,
                    ImGui::GetItemRectSize().y * 1.0f ) ;
                ImGui::SetNextItemWidth( item_dims.x ) ;

                ImGui::SameLine() ;
                motor::string_t const name = motor::string_t( label ) + "##input_int" + motor::to_string(i) ;
                if( ImGui::InputInt( name.c_str(), &items[i].second ) )
                {
                    *selected_item = i ;
                    value_changed = true ;
                }

                if( ImGui::IsItemHovered() )
                {
                    *hovered_item = i ;
                }
            }

            
        }

        EndListBox() ;
    }
    return selected ;
}

bool motor::tool::imgui_custom::Selectable(const char* label, bool* p_selected, bool * p_hovered, ImGuiSelectableFlags flags , const ImVec2& size_arg )
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = pos;
    const ImVec2 text_max(min_x + size.x, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
        const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns)
    {
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const bool item_add = ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
    if (span_all_columns)
    {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add)
        return false;

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global) // Only testing this as an optimization
        BeginDisabled();

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns && window->DC.CurrentColumns)
        PushColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        TablePushBackgroundChannel();

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner)     { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
    if (flags & ImGuiSelectableFlags_SelectOnClick)     { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease)   { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick)  { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

    bool & selected = *p_selected ;
    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    hovered = *p_hovered ? true : hovered ;
    *p_hovered = hovered ;

    // Auto-select when moved into
    // - This will be more fully fleshed in the range-select branch
    // - This is not exposed as it won't nicely work with some user side handling of shift/control
    // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
    //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
    //   - (2) usage will fail with clipped items
    //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
    if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
        if (g.NavJustMovedToId == id)
            selected = pressed = true;

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, WindowRectAbsToRel(window, bb)); // (bb == NavRect)
            g.NavDisableHighlight = true;
        }
    }
    if (pressed)
        MarkItemEdited(id);

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) //-V547
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (hovered || selected)
    {
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
    }
    if (g.NavId == id)
        RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);

    if (span_all_columns && window->DC.CurrentColumns)
        PopColumnsBackground();
    else if (span_all_columns && g.CurrentTable)
        TablePopBackgroundChannel();

    RenderTextClipped(text_min, text_max, label, NULL, &label_size, style.SelectableTextAlign, &bb);

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();

    if (disabled_item && !disabled_global)
        EndDisabled();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed; //-V1020
}