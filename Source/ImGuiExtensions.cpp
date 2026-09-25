// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "ImGuiExtensions.hpp"
#include <imgui_internal.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlugin::Detail
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Str> GetScratch()
    {
        static Str sScratch;
        return sScratch;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Char> Terminate(Text Value, UInt32 Slot)
    {
        // Kept apart from the scratch string, so text formatted there can be handed to a call that copies it here.
        static Array<Str, 2> sSlots;

        ZY_ASSERT(Slot < sSlots.GetSize(), "Only two pieces of text are kept at once");

        Ref<Str> Copy = sSlots[Slot];
        Copy = Value;
        return Copy.GetData();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ConstPtr<Char> TerminateFormat(Text Format)
    {
        return Format.IsEmpty() ? nullptr : Terminate(Format, 1);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ShowLabelAfter(Text Label, Real32 Spacing)
    {
        if (Label.IsEmpty())
        {
            return;
        }

        const ConstPtr<Char> First = Label.GetData();
        const ConstPtr<Char> Last  = ImGui::FindRenderedTextEnd(First, First + Label.GetSize());

        if (First != Last)
        {
            ImGui::SameLine(0.0f, Spacing);
            ImGui::TextEx(First, Last);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool DragN(
        Text             Label,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        UInt             Count,
        Real32           Speed,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags)
    {
        const ConstPtr<Char> Name    = Terminate(Label);
        const ConstPtr<Char> Pattern = TerminateFormat(Format);

        // A single number goes through the plain widget, which lays out and reports as one item, not a group.
        return Count == 1
            ? ImGui::DragScalar(Name, Kind, Data, Speed, Min, Max, Pattern, Flags)
            : ImGui::DragScalarN(Name, Kind, Data, static_cast<int>(Count), Speed, Min, Max, Pattern, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SliderN(
        Text             Label,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        UInt             Count,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags)
    {
        const ConstPtr<Char> Name    = Terminate(Label);
        const ConstPtr<Char> Pattern = TerminateFormat(Format);

        return Count == 1
            ? ImGui::SliderScalar(Name, Kind, Data, Min, Max, Pattern, Flags)
            : ImGui::SliderScalarN(Name, Kind, Data, static_cast<int>(Count), Min, Max, Pattern, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool InputN(
        Text                Label,
        ImGuiDataType       Kind,
        Ptr<void>           Data,
        UInt                Count,
        ConstPtr<void>      Step,
        ConstPtr<void>      StepFast,
        Text                Format,
        ImGuiInputTextFlags Flags)
    {
        const ConstPtr<Char> Name    = Terminate(Label);
        const ConstPtr<Char> Pattern = TerminateFormat(Format);

        return Count == 1
            ? ImGui::InputScalar(Name, Kind, Data, Step, StepFast, Pattern, Flags)
            : ImGui::InputScalarN(Name, Kind, Data, static_cast<int>(Count), Step, StepFast, Pattern, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ColorN(Text Label, Ptr<Real32> Channels, ImGuiColorEditFlags Flags, Bool Picker)
    {
        return Picker
            ? ImGui::ColorPicker4(Terminate(Label), Channels, Flags)
            : ImGui::ColorEdit4(Terminate(Label), Channels, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool DragAxisN(
        Text             ID,
        Text             Tag,
        ImU32            Tint,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        Real32           Speed,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags)
    {
        ConstRef<ImGuiStyle> Style = ImGui::GetStyle();

        // Read before the tag is laid out, which would otherwise spend a width set for the next item.
        const Real32 Width    = ImGui::CalcItemWidth();
        const ImVec2 TagSize  = ImGui::CalcTextSize(Tag);
        const Real32 TagWidth = IM_ROUND(TagSize.x + Style.FramePadding.x * 2.0f);
        const Real32 Height   = ImGui::GetFrameHeight();
        const ImVec2 Origin   = ImGui::GetCursorScreenPos();

        // A drag centres its reading in its own frame whatever padding it is given, so the tag is stood beside the
        // field rather than over it; a long value would otherwise run back under the letter.
        ImGui::Dummy(ImVec2(TagWidth, Height));
        ImGui::SameLine(0.0f, 0.0f);

        ImGui::SetNextItemWidth(Width > TagWidth + 1.0f ? Width - TagWidth : 1.0f);
        const Bool Changed = DragN(ID, Kind, Data, 1, Speed, Min, Max, Format, Flags);

        // The tag is drawn after the field and reaches into it by the rounding, covering the field's rounded left
        // corners so the two read as one frame.
        const Ptr<ImDrawList> Canvas = ImGui::GetWindowDrawList();

        Canvas->AddRectFilled(
            Origin,
            ImVec2(Origin.x + TagWidth + Style.FrameRounding, Origin.y + Height),
            Tint,
            Style.FrameRounding,
            ImDrawFlags_RoundCornersLeft);

        if (!Tag.IsEmpty())
        {
            Canvas->AddText(
                ImVec2(Origin.x + Style.FramePadding.x, Origin.y + (Height - TagSize.y) * 0.5f),
                ImGui::GetAxisStyle().Letter,
                Tag.GetData(),
                Tag.GetData() + Tag.GetSize());
        }
        return Changed;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool DragAxesN(
        Text             Label,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        UInt             Count,
        Real32           Speed,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags)
    {
        static constexpr Array<Text, 4> kTags("X", "Y", "Z", "W");

        ZY_ASSERT(Count <= kTags.GetSize(), "Only four axes are named");

        ConstRef<ImGuiStyle>       Style = ImGui::GetStyle();
        ConstRef<ImGui::AxisStyle> Axes  = ImGui::GetAxisStyle();

        // ImGui truncates every item width, so each field takes a whole share and the last also takes what the
        // truncation left over, ending the fields exactly where the item width does.
        const Real32 Total   = ImGui::CalcItemWidth();
        const Real32 Spacing = Style.ItemInnerSpacing.x;
        const Real32 Share   = IM_TRUNC((Total - Spacing * static_cast<Real32>(Count - 1)) / static_cast<Real32>(Count));
        const Real32 Last    = Total - (Share + Spacing) * static_cast<Real32>(Count - 1);
        const UInt   Stride  = ImGui::DataTypeGetInfo(Kind)->Size;

        Bool Changed = false;

        ImGui::BeginGroup();
        ImGui::PushID(Label);

        for (UInt Index = 0; Index < Count; ++Index)
        {
            if (Index > 0)
            {
                ImGui::SameLine(0.0f, Spacing);
            }

            ImGui::PushID(static_cast<int>(Index));

            // A negative width would be read as a distance from the right edge rather than as too little room.
            const Real32 Width = Index + 1 == Count ? Last : Share;
            ImGui::SetNextItemWidth(Width > 1.0f ? Width : 1.0f);

            Changed |= DragAxisN(
                "",
                kTags[Index],
                Axes.Tints[Index],
                Kind,
                static_cast<Ptr<UInt8>>(Data) + Index * Stride,
                Speed,
                Min,
                Max,
                Format,
                Flags);

            ImGui::PopID();
        }

        ImGui::PopID();

        ShowLabelAfter(Label, Spacing);

        ImGui::EndGroup();
        return Changed;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool EditText(Text Label, Text Value, ImGuiInputTextFlags Flags, Ref<Text> Edited)
    {
        // Kept between fields, so a frame does not allocate once it has seen its longest text.
        static Str sCopy;

        sCopy = Value;

        if (!ImGui::InputText(Label, sCopy, Flags))
        {
            return false;
        }

        Edited = Text(sCopy);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool EditTextWithButton(
        Text                Label,
        Text                Value,
        ImGuiInputTextFlags Flags,
        Ref<Text>           Edited,
        Text                Button,
        Ref<Bool>           Clicked)
    {
        ConstRef<ImGuiStyle> Style = ImGui::GetStyle();

        // The button is given a whole width of its own, so the field and the button end exactly where the item
        // width does.
        const Real32 Total   = ImGui::CalcItemWidth();
        const Real32 Spacing = Style.ItemInnerSpacing.x;
        const Real32 Width   = IM_ROUND(ImGui::CalcTextSize(Button, true).x + Style.FramePadding.x * 2.0f);
        const Real32 Field   = Total - Width - Spacing;

        ImGui::BeginGroup();
        ImGui::PushID(Label);

        // A negative width would be read as a distance from the right edge rather than as too little room.
        ImGui::SetNextItemWidth(Field > 1.0f ? Field : 1.0f);
        const Bool Changed = EditText("##field", Value, Flags, Edited);

        // A full button rather than a small one, so it stands as tall as the field beside it.
        ImGui::SameLine(0.0f, Spacing);
        Clicked = ImGui::Button(Button, ImVec2(Width, 0.0f));

        ImGui::PopID();

        ShowLabelAfter(Label, Spacing);

        ImGui::EndGroup();
        return Changed;
    }
}

namespace ImGui
{
    using ZyPlugin::Detail::Editing;
    using ZyPlugin::Detail::Terminate;

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static int OnEdit(Ptr<ImGuiInputTextCallbackData> Data)
    {
        Ref<Editing> State = * static_cast<Ptr<Editing>>(Data->UserData);

        if (Data->EventFlag != ImGuiInputTextFlags_CallbackResize)
        {
            if (State.Callback == nullptr)
            {
                return 0;
            }

            // The caller's callback sees its own data, not the state riding in its place.
            Data->UserData = State.UserData;
            const int Result = State.Callback(Data);
            Data->UserData = AddressOf(State);
            return Result;
        }

        // ImGui asks for room for a text longer than the buffer, which it writes in afterwards, so only what the
        // buffer already holds is carried over.
        Sequence<Char> Grown;
        Grown.Resize(static_cast<UInt>(Data->BufSize));
        Copy(Grown.GetData(), Min(State.Buffer->GetSize(), static_cast<UInt>(Data->BufTextLen)), Data->Buf);

        * State.Buffer = Move(Grown);
        Data->Buf      = State.Buffer->GetData();
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Draw>
    static Bool Edit(
        Ref<Str>               Value,
        ImGuiInputTextFlags    Flags,
        ImGuiInputTextCallback Callback,
        Ptr<void>              UserData,
        AnyRef<Draw>           Field)
    {
        IM_ASSERT((Flags & ImGuiInputTextFlags_CallbackResize) == 0 && "The field already grows with its text");

        // The string cannot be told its new length after ImGui writes into it, so ImGui edits a copy instead, and
        // the copy is kept between fields so a frame does not allocate once it has seen its longest text.
        static Sequence<Char> sBuffer;

        const UInt Needed = Value.GetSize() + 1;

        if (sBuffer.GetSize() < Needed)
        {
            sBuffer.Resize(Needed);
        }
        Copy(sBuffer.GetData(), Needed, Value.GetData());

        Editing State(AddressOf(sBuffer), Callback, UserData);

        if (!Field(sBuffer.GetData(), sBuffer.GetSize(), Flags | ImGuiInputTextFlags_CallbackResize, AddressOf(State)))
        {
            return false;
        }

        Value = ::Text(sBuffer.GetData(), StrLength(sBuffer.GetData()));
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void TextUnformatted(::Text Value)
    {
        TextUnformatted(Value.GetData(), Value.GetData() + Value.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void TextDisabled(::Text Value)
    {
        PushStyleColor(ImGuiCol_Text, GetStyle().Colors[ImGuiCol_TextDisabled]);
        TextUnformatted(Value);
        PopStyleColor();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void TextColored(ImVec4 Tint, ::Text Value)
    {
        PushStyleColor(ImGuiCol_Text, Tint);
        TextUnformatted(Value);
        PopStyleColor();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void TextWrapped(::Text Value)
    {
        // A wrap position of zero is the edge of the window, which is where ImGui's own TextWrapped wraps.
        PushTextWrapPos(0.0f);
        TextUnformatted(Value);
        PopTextWrapPos();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void SetTooltip(::Text Value)
    {
        // Empty text opens nothing, where ImGui would show an empty box.
        if (Value.IsEmpty())
        {
            return;
        }

        // The same tooltip ImGui's own SetTooltip opens, which replaces one opened earlier in the frame.
        if (BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
        {
            TextUnformatted(Value);
            EndTooltip();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void SetItemTooltip(::Text Value)
    {
        if (!Value.IsEmpty() && IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            SetTooltip(Value);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PushID(::Text Value)
    {
        // ImGui reads an empty range as text ending in a NUL, which past an empty view is whatever follows it.
        const ConstPtr<Char> First = Value.IsEmpty() ? "" : Value.GetData();
        PushID(First, First + Value.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ImGuiID GetID(::Text Value)
    {
        // As in PushID, an empty view is handed over as an empty string rather than an empty range.
        const ConstPtr<Char> First = Value.IsEmpty() ? "" : Value.GetData();
        return GetID(First, First + Value.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool InputText(
        ::Text                 Label,
        Ref<Str>               Value,
        ImGuiInputTextFlags    Flags,
        ImGuiInputTextCallback Callback,
        Ptr<void>              UserData)
    {
        const auto Field = [&](Ptr<Char> Buffer, UInt Length, ImGuiInputTextFlags All, Ptr<void> State)
        {
            return InputText(Terminate(Label), Buffer, Length, All, OnEdit, State);
        };
        return Edit(Value, Flags, Callback, UserData, Field);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool InputTextMultiline(
        ::Text                 Label,
        Ref<Str>               Value,
        ImVec2                 Size,
        ImGuiInputTextFlags    Flags,
        ImGuiInputTextCallback Callback,
        Ptr<void>              UserData)
    {
        const auto Field = [&](Ptr<Char> Buffer, UInt Length, ImGuiInputTextFlags All, Ptr<void> State)
        {
            return InputTextMultiline(Terminate(Label), Buffer, Length, Size, All, OnEdit, State);
        };
        return Edit(Value, Flags, Callback, UserData, Field);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool InputTextWithHint(
        ::Text                 Label,
        ::Text                 Hint,
        Ref<Str>               Value,
        ImGuiInputTextFlags    Flags,
        ImGuiInputTextCallback Callback,
        Ptr<void>              UserData)
    {
        const auto Field = [&](Ptr<Char> Buffer, UInt Length, ImGuiInputTextFlags All, Ptr<void> State)
        {
            return InputTextWithHint(Terminate(Label), Terminate(Hint, 1), Buffer, Length, All, OnEdit, State);
        };
        return Edit(Value, Flags, Callback, UserData, Field);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Begin(::Text Name, Ptr<Bool> Open, ImGuiWindowFlags Flags)
    {
        return Begin(Terminate(Name), Open, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginChild(::Text ID, ImVec2 Size, ImGuiChildFlags ChildFlags, ImGuiWindowFlags WindowFlags)
    {
        return BeginChild(Terminate(ID), Size, ChildFlags, WindowFlags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Button(::Text Label, ImVec2 Size)
    {
        return Button(Terminate(Label), Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SmallButton(::Text Label)
    {
        return SmallButton(Terminate(Label));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool InvisibleButton(::Text ID, ImVec2 Size, ImGuiButtonFlags Flags)
    {
        return InvisibleButton(Terminate(ID), Size, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Checkbox(::Text Label, Ptr<Bool> Value)
    {
        return Checkbox(Terminate(Label), Value);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool RadioButton(::Text Label, Bool Active)
    {
        return RadioButton(Terminate(Label), Active);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Selectable(::Text Label, Bool Selected, ImGuiSelectableFlags Flags, ImVec2 Size)
    {
        return Selectable(Terminate(Label), Selected, Flags, Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Selectable(::Text Label, Ptr<Bool> Selected, ImGuiSelectableFlags Flags, ImVec2 Size)
    {
        return Selectable(Terminate(Label), Selected, Flags, Size);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TreeNode(::Text Label)
    {
        return TreeNode(Terminate(Label));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool TreeNodeEx(::Text Label, ImGuiTreeNodeFlags Flags)
    {
        return TreeNodeEx(Terminate(Label), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool CollapsingHeader(::Text Label, ImGuiTreeNodeFlags Flags)
    {
        return CollapsingHeader(Terminate(Label), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool CollapsingHeader(::Text Label, Ptr<Bool> Visible, ImGuiTreeNodeFlags Flags)
    {
        return CollapsingHeader(Terminate(Label), Visible, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginMenu(::Text Label, Bool Enabled)
    {
        return BeginMenu(Terminate(Label), Enabled);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MenuItem(::Text Label, ::Text Shortcut, Bool Selected, Bool Enabled)
    {
        return MenuItem(Terminate(Label), Shortcut.IsEmpty() ? nullptr : Terminate(Shortcut, 1), Selected, Enabled);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MenuItem(::Text Label, ::Text Shortcut, Ptr<Bool> Selected, Bool Enabled)
    {
        return MenuItem(Terminate(Label), Shortcut.IsEmpty() ? nullptr : Terminate(Shortcut, 1), Selected, Enabled);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginCombo(::Text Label, ::Text Preview, ImGuiComboFlags Flags)
    {
        return BeginCombo(Terminate(Label), Terminate(Preview, 1), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginTabBar(::Text ID, ImGuiTabBarFlags Flags)
    {
        return BeginTabBar(Terminate(ID), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginTabItem(::Text Label, Ptr<Bool> Open, ImGuiTabItemFlags Flags)
    {
        return BeginTabItem(Terminate(Label), Open, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool OpenPopup(::Text ID, ImGuiPopupFlags Flags)
    {
        return OpenPopup(Terminate(ID), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginPopup(::Text ID, ImGuiWindowFlags Flags)
    {
        return BeginPopup(Terminate(ID), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginPopupModal(::Text Name, Ptr<Bool> Open, ImGuiWindowFlags Flags)
    {
        return BeginPopupModal(Terminate(Name), Open, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginPopupContextItem(::Text ID, ImGuiPopupFlags Flags)
    {
        return BeginPopupContextItem(Terminate(ID), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginPopupContextWindow(::Text ID, ImGuiPopupFlags Flags)
    {
        return BeginPopupContextWindow(Terminate(ID), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool IsPopupOpen(::Text ID, ImGuiPopupFlags Flags)
    {
        return IsPopupOpen(Terminate(ID), Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool BeginTable(::Text ID, SInt32 Columns, ImGuiTableFlags Flags, ImVec2 OuterSize, Real32 InnerWidth)
    {
        return BeginTable(Terminate(ID), Columns, Flags, OuterSize, InnerWidth);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void TableSetupColumn(::Text Label, ImGuiTableColumnFlags Flags, Real32 Width, ImGuiID UserID)
    {
        // The table keeps a copy of the header, so the text need only last the call.
        TableSetupColumn(Terminate(Label), Flags, Width, UserID);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void SeparatorText(::Text Label)
    {
        SeparatorText(Terminate(Label));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ImVec2 CalcTextSize(::Text Value, Bool HideAfterDoubleHash, Real32 WrapWidth)
    {
        const ConstPtr<Char> First = Value.IsEmpty() ? "" : Value.GetData();
        return CalcTextSize(First, First + Value.GetSize(), HideAfterDoubleHash, WrapWidth);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<AxisStyle> GetAxisStyle()
    {
        static AxisStyle sStyle {
            Array<ImU32, 4>(
                IM_COL32(220,  70,  70, 255),
                IM_COL32( 90, 200,  90, 255),
                IM_COL32( 90, 160, 235, 255),
                IM_COL32(200, 200, 200, 255)),
            IM_COL32(255, 255, 255, 235)
        };
        return sStyle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ToggleButton(::Text Label, Bool Active, ImVec2 Size)
    {
        // An active button keeps the colour it has while held down, hovered or not.
        if (Active)
        {
            PushStyleColor(ImGuiCol_Button,        GetStyleColorVec4(ImGuiCol_ButtonActive));
            PushStyleColor(ImGuiCol_ButtonHovered, GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        const Bool Pressed = Button(Label, Size);

        if (Active)
        {
            PopStyleColor(2);
        }
        return Pressed;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void TextCentered(::Text Value)
    {
        const Real32 Width = CalcTextSize(Value).x;
        const Real32 Room  = GetContentRegionAvail().x;

        // Text wider than the room starts where the line does, so its beginning is not pushed out of sight.
        if (Width < Room)
        {
            SetCursorPosX(GetCursorPosX() + (Room - Width) * 0.5f);
        }
        TextUnformatted(Value);
    }
}