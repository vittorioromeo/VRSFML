
    ////////////////////////////////////////////////////////////
    void textOutlinedImpl(const char* text, const ImVec4& textColor = ImVec4(1, 1, 1, 1), float outlineThickness = 1.f)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        // Get current cursor position and compute text size.
        ImVec2 pos      = window->DC.CursorPos;
        ImVec2 textSize = ImGui::CalcTextSize(text);

        // Define the bounding box for the text item.
        ImRect bb(pos, pos + textSize);

        // Inform the layout system about the space consumed by this item.
        ImGui::ItemSize(bb.GetSize(), 0.0f);
        if (!ImGui::ItemAdd(bb, 0))
            return;

        const auto imGuiOutlineColor      = ImGui::GetColorU32(colorBlueOutline.toVec4<ImVec4>());
        const auto scaledOutlineThickness = outlineThickness * getUIScalingFactor();

        ImDrawList* drawLlist = ImGui::GetWindowDrawList();

        // Draw the outline by drawing the text at multiple offset positions.
        drawLlist->AddText(pos + ImVec2(-scaledOutlineThickness, 0), imGuiOutlineColor, text);
        drawLlist->AddText(pos + ImVec2(scaledOutlineThickness, 0), imGuiOutlineColor, text);
        drawLlist->AddText(pos + ImVec2(0, -scaledOutlineThickness), imGuiOutlineColor, text);
        drawLlist->AddText(pos + ImVec2(0, scaledOutlineThickness), imGuiOutlineColor, text);

        // Optionally, add diagonal offsets for a thicker outline.
        drawLlist->AddText(pos + ImVec2(-scaledOutlineThickness, -scaledOutlineThickness), imGuiOutlineColor, text);
        drawLlist->AddText(pos + ImVec2(scaledOutlineThickness, -scaledOutlineThickness), imGuiOutlineColor, text);
        drawLlist->AddText(pos + ImVec2(-scaledOutlineThickness, scaledOutlineThickness), imGuiOutlineColor, text);
        drawLlist->AddText(pos + ImVec2(scaledOutlineThickness, scaledOutlineThickness), imGuiOutlineColor, text);

        // Draw the main text on top.
        drawLlist->AddText(pos, ImGui::GetColorU32(textColor), text);
    }

    ////////////////////////////////////////////////////////////
    void textOutlined(const char* fmt, ...)
    {
        char buf[1024];

        va_list args{};

        va_start(args, fmt);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        vsnprintf(buf, sizeof(buf), fmt, args);
#pragma GCC diagnostic pop
        va_end(args);

        textOutlinedImpl(buf);
    }
