#include "custom.hpp"
#include <map>

using namespace ImGui;

float color_UI[4] = { 0.39f, 0.35f, 1.00f, 1.000f };


bool Custom::AnimButton(const char* label, const ImVec2& size_arg)
{
    return ButtonEx_animation(label, size_arg, ImGuiButtonFlags_None);
}

bool Custom::ButtonEx_animation(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render

    const ImU32 col = GetColorU32(ImGuiCol_WindowBg);

    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

    static std::map<ImGuiID, float> alpha_anim;
    auto it_alpha = alpha_anim.find(id);

    if (it_alpha == alpha_anim.end())
    {
        alpha_anim.insert({ id, 0.00f });
        it_alpha = alpha_anim.find(hovered || IsItemActive());
    }

    if (hovered || IsItemActive()) {
        ImGui::SetMouseCursor(7);
        if (it_alpha->second <= 0.22f)
            it_alpha->second += 0.01f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color_UI[0] - (float)it_alpha->second, color_UI[1] - (float)it_alpha->second, color_UI[2] - (float)it_alpha->second, 1.00f));
        RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    }
    else {
        if (it_alpha->second >= 0.00f)
            it_alpha->second -= 0.01f;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color_UI[0] - (float)it_alpha->second, color_UI[1] - (float)it_alpha->second, color_UI[2] - (float)it_alpha->second, 1.00f));
        RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    }
    ImGui::PopStyleColor(1);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}


bool Custom::BeginChildEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* parent_window = g.CurrentWindow;

    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
    flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

    // Size
    const ImVec2 content_avail = GetContentRegionAvail();
    ImVec2 size = ImFloor(size_arg);
    const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
    if (size.x <= 0.0f)
        size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too many issues)
    if (size.y <= 0.0f)
        size.y = ImMax(content_avail.y + size.y, 4.0f);

    SetNextWindowPos(ImVec2(parent_window->DC.CursorPos + ImVec2(0, 32)));
    SetNextWindowSize(size - ImVec2(0, 32));

    ImGui::GetWindowDrawList()->AddText(parent_window->DC.CursorPos + ImVec2(12, 4), GetColorU32(ImGuiCol_Text), name);

    // Build up name. If you need to append to a same child from multiple location in the ID stack, use BeginChild(ImGuiID id) with a stable value.
    const char* temp_window_name;
    if (name)
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%s_%08X", parent_window->Name, name, id);
    else
        ImFormatStringToTempBuffer(&temp_window_name, NULL, "%s/%08X", parent_window->Name, id);

    PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.f));
    PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
    bool ret = Begin(temp_window_name, NULL, flags);
    PopStyleColor(2);

    ImGuiWindow* child_window = g.CurrentWindow;
    child_window->ChildId = id;
    child_window->AutoFitChildAxises = (ImS8)auto_fit_axises;

    // Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
    // While this is not really documented/defined, it seems that the expected thing to do.
    if (child_window->BeginCount == 1)
        parent_window->DC.CursorPos = child_window->Pos;

    // Process navigation-in immediately so NavInit can run on first frame
    if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayersActiveMask != 0 || child_window->DC.NavHasScroll))
    {
        FocusWindow(child_window);
        NavInitWindow(child_window, false);
        SetActiveID(id + 1, child_window); // Steal ActiveId with another arbitrary id so that key-press won't activate child item
        g.ActiveIdSource = ImGuiInputSource_Nav;
    }
    return ret;
}

bool Custom::BeginChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 5));
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 15)); // DON'T CHANGE!!!
    return BeginChildEx(str_id, window->GetID(str_id), size_arg, border, extra_flags | ImGuiWindowFlags_AlwaysUseWindowPadding);
}

bool Custom::BeginChild(ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
    IM_ASSERT(id != 0);
    return BeginChildEx(NULL, id, size_arg, border, extra_flags);
}

void Custom::EndChild()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    PopStyleVar(2);
    IM_ASSERT(g.WithinEndChild == false);
    IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() calls

    g.WithinEndChild = true;
    if (window->BeginCount > 1)
    {
        End();
    }
    else
    {
        ImVec2 sz = window->Size;
        if (window->AutoFitChildAxises & (1 << ImGuiAxis_X)) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
            sz.x = ImMax(4.0f, sz.x);
        if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y))
            sz.y = ImMax(4.0f, sz.y);
        End();

        ImGuiWindow* parent_window = g.CurrentWindow;
        ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
        ItemSize(sz);
        if ((window->DC.NavLayersActiveMask != 0 || window->DC.NavHasScroll) && !(window->Flags & ImGuiWindowFlags_NavFlattened))
        {
            ItemAdd(bb, window->ChildId);
            RenderNavHighlight(bb, window->ChildId);

            // When browsing a window that has no activable items (scroll only) we keep a highlight on the child (pass g.NavId to trick into always displaying)
            if (window->DC.NavLayersActiveMask == 0 && window == g.NavWindow)
                RenderNavHighlight(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavHighlightFlags_TypeThin);
        }
        else
        {
            // Not navigable into
            ItemAdd(bb, 0);
        }
        if (g.HoveredWindow == window)
            g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
    }
    g.WithinEndChild = false;
    g.LogLinePosY = -FLT_MAX; // To enforce a carriage return
}


bool Custom::ButtonEx_Configuration(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render



    static std::map<ImGuiID, float> alpha_anim;
    auto it_alpha = alpha_anim.find(id);

    if (it_alpha == alpha_anim.end())
    {
        alpha_anim.insert({ id, 0.8f });
        it_alpha = alpha_anim.find(hovered || IsItemActive());
    }

    if (hovered || IsItemActive()) {
        ImGui::SetMouseCursor(7);
        if (it_alpha->second < 1.0f)
            it_alpha->second += 0.03f / ImGui::GetIO().Framerate * 60.f;

        RenderFrame(bb.Min, ImVec2(bb.Max.x, bb.Max.y + 4), ImColor(0.13f, 0.13f, 0.19f, (float)it_alpha->second), true, style.FrameRounding);
    }
    else {
        if (it_alpha->second > 0.8f)
            it_alpha->second -= 0.03f / ImGui::GetIO().Framerate * 60.f;

        RenderFrame(bb.Min, ImVec2(bb.Max.x, bb.Max.y + 4), ImColor(0.13f, 0.13f, 0.19, (float)it_alpha->second), true, style.FrameRounding);
    }
    RenderNavHighlight(bb, id);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

bool Custom::Configuration(const char* label, const ImVec2& size_arg)
{
    return ButtonEx_Configuration(label, size_arg, ImGuiButtonFlags_None);
}

extern ImFont* ico;

struct tab_state {
    ImVec4 background, rect, text;
};

bool Custom::Tab(bool selected, const char* icon_label, const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size = size_arg;
    ImVec2 pos = window->DC.CursorPos;

    const ImRect rect(pos, pos + size);
    ItemSize(rect, style.FramePadding.y);
    if (!ItemAdd(rect, id))
        return false;

    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(rect, id, &hovered, &held, flags);

    static std::map<ImGuiID, tab_state> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, tab_state() });
        it_anim = anim.find(id);
    }

    it_anim->second.background = ImLerp(it_anim->second.background, selected ? ImColor(32, 32, 48, 150) : ImColor(32, 32, 48, 0), g.IO.DeltaTime * 6.f);

    it_anim->second.rect = ImLerp(it_anim->second.rect, selected ? ImColor(100, 88, 255) : ImColor(70, 70, 70, 0), g.IO.DeltaTime * 6.f);

    it_anim->second.text = ImLerp(it_anim->second.text, selected ? ImColor(255, 255, 255, 255) : hovered ? ImColor(150, 150, 150, 255) : ImColor(107, 107, 107, 255), g.IO.DeltaTime * 6.f);

    window->DrawList->AddRectFilled(rect.Min, rect.Max, GetColorU32(it_anim->second.background), style.FrameRounding);
    window->DrawList->AddRectFilled(rect.Min + ImVec2(size_arg.x - 3, 5), rect.Max - ImVec2(0, 5), GetColorU32(it_anim->second.rect), 30.f, ImDrawCornerFlags_Left);


    PushStyleColor(ImGuiCol_Text, GetColorU32(it_anim->second.text));
    PushFont(ico);
    RenderTextClipped(rect.Min + ImVec2(10, 6), rect.Max + ImVec2(0, 9), icon_label, NULL, &label_size);
    PopFont();

    RenderTextClipped(rect.Min + ImVec2(45, 6), rect.Max + ImVec2(0, 6), label, NULL, &label_size);
    PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}


struct checkbox_state {
    ImVec4 background, circle, rect, text;
    float circle_pos;
};

extern ImFont* widgets;

bool Custom::Checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float width = GetContentRegionMax().x;

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos - ImVec2(0, 12), pos + ImVec2(square_sz + (label_size.x > 0.0f ? width : 0.0f), label_size.y + 10));

    const ImRect block(pos - ImVec2(-2, 10), pos + ImVec2(width - 12, 30));

    ItemSize(total_bb, style.FramePadding.y);

    if (!ItemAdd(total_bb, id))
    {
        IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
        return false;
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (IsItemClicked())
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    static std::map<ImGuiID, checkbox_state> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, checkbox_state() });
        it_anim = anim.find(id);
    }

    it_anim->second.background = ImLerp(it_anim->second.background, hovered ? ImColor(32, 32, 48, 255) : ImColor(32, 32, 48, 150), g.IO.DeltaTime * 6.f);

    it_anim->second.circle_pos = ImLerp(it_anim->second.circle_pos, *v ? 17.5f : 0.f, g.IO.DeltaTime * 9.f);
    it_anim->second.circle = ImLerp(it_anim->second.circle, *v ? ImColor(100, 88, 255) : ImColor(255, 255, 255, 255), g.IO.DeltaTime * 6.f);

    it_anim->second.rect = ImLerp(it_anim->second.rect, hovered ? ImColor(20, 20, 20, 100) : ImColor(20, 20, 20, 100), g.IO.DeltaTime * 6.f);
    it_anim->second.text = ImLerp(it_anim->second.text, *v ? ImColor(255, 255, 255, 255) : hovered ? ImColor(200, 200, 200, 255) : ImColor(180, 180, 180, 255), g.IO.DeltaTime * 6.f);

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));

    window->DrawList->AddRectFilled(block.Min, block.Max, GetColorU32(it_anim->second.background), style.FrameRounding);

    window->DrawList->AddRectFilled(block.Min + ImVec2(width - 60, 11), block.Max - ImVec2(10, 11), GetColorU32(it_anim->second.rect), 30.f);

    window->DrawList->AddCircleFilled(block.Min + ImVec2(width - 51 + it_anim->second.circle_pos, 20), 6.f, GetColorU32(it_anim->second.circle), 30.f);

    ImVec2 label_pos = ImVec2(check_bb.Max.x - 12, check_bb.Min.y - 1);

    window->DrawList->AddText(widgets, 20.f, label_pos, GetColorU32(it_anim->second.text), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}