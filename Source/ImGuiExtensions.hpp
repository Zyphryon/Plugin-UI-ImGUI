// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include <imgui.h>
#include <Zyphryon.Math/Color.hpp>
#include <Zyphryon.Math/Vector2.hpp>
#include <Zyphryon.Math/Vector3.hpp>
#include <Zyphryon.Math/Vector4.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlugin::Detail
{
    /// \brief Gets the string the widgets below write their text into, reused so that a frame allocates nothing.
    ///
    /// \return The scratch string, whose contents last only until the next widget writes into it.
    Ref<Str> GetScratch();

    /// \brief Copies a piece of text into a string kept for the purpose, for an ImGui call that reads up to a NUL.
    ///
    /// \param Value The text to copy.
    /// \param Slot  The string to copy into, `0` or `1`, so that a call reading two pieces of text keeps both.
    /// \return The copy, ending in a NUL, valid until the next text copied into the same slot.
    ConstPtr<Char> Terminate(Text Value, UInt32 Slot = 0);

    /// \brief Represents a string being edited, and the callback the caller asked for besides growing it.
    struct Editing final
    {
        /// The buffer ImGui writes into, grown as the text grows.
        Ptr<Sequence<Char>>    Buffer;

        /// The callback the caller asked for, or `nullptr`.
        ImGuiInputTextCallback Callback;

        /// The data handed to \ref Callback.
        Ptr<void>              UserData;
    };

    /// \brief Checks whether a type is a number ImGui's scalar widgets can edit.
    template<typename Type>
    concept IsScalar = (IsIntegral<Type> && !IsAnyOf<Type, Bool, Char>) || IsAnyOf<Type, Real32, Real64>;

    /// \brief Gets the ImGui data type a scalar is edited as.
    ///
    /// \return The data type matching the scalar's size, sign and kind.
    template<IsScalar Type>
    ZY_INLINE constexpr ImGuiDataType GetDataType()
    {
        if constexpr (IsReal<Type>)
        {
            return sizeof(Type) == 4 ? ImGuiDataType_Float : ImGuiDataType_Double;
        }
        else if constexpr (sizeof(Type) == 1)
        {
            return IsSigned<Type> ? ImGuiDataType_S8 : ImGuiDataType_U8;
        }
        else if constexpr (sizeof(Type) == 2)
        {
            return IsSigned<Type> ? ImGuiDataType_S16 : ImGuiDataType_U16;
        }
        else if constexpr (sizeof(Type) == 4)
        {
            return IsSigned<Type> ? ImGuiDataType_S32 : ImGuiDataType_U32;
        }
        else
        {
            return IsSigned<Type> ? ImGuiDataType_S64 : ImGuiDataType_U64;
        }
    }

    /// \brief Names a type without letting a call deduce it, so a bound given as a literal takes the value's type.
    template<typename Type>
    struct Identity final
    {
        using Result = Type;
    };

    /// \brief Names a type without letting a call deduce it.
    template<typename Type>
    using Exactly = typename Identity<Type>::Result;

    /// \brief Describes how a vector is taken apart into the scalars ImGui edits, and put back together.
    template<typename Vector>
    struct Components;

    /// \brief Describes a vector of two scalars.
    template<IsScalar Type>
    struct Components<AnyVector2<Type>> final
    {
        using Scalar = Type;

        static constexpr UInt kCount = 2;

        ZY_INLINE static Array<Type, 2> Load(ConstRef<AnyVector2<Type>> Value)
        {
            return Array<Type, 2>(Value.GetX(), Value.GetY());
        }

        ZY_INLINE static AnyVector2<Type> Store(ConstRef<Array<Type, 2>> Value)
        {
            return AnyVector2<Type>(Value[0], Value[1]);
        }
    };

    /// \brief Describes a vector of three scalars.
    template<IsScalar Type>
    struct Components<AnyVector3<Type>> final
    {
        using Scalar = Type;

        static constexpr UInt kCount = 3;

        ZY_INLINE static Array<Type, 3> Load(ConstRef<AnyVector3<Type>> Value)
        {
            return Array<Type, 3>(Value.GetX(), Value.GetY(), Value.GetZ());
        }

        ZY_INLINE static AnyVector3<Type> Store(ConstRef<Array<Type, 3>> Value)
        {
            return AnyVector3<Type>(Value[0], Value[1], Value[2]);
        }
    };

    /// \brief Describes a vector of four reals.
    template<>
    struct Components<Vector4> final
    {
        using Scalar = Real32;

        static constexpr UInt kCount = 4;

        ZY_INLINE static Array<Real32, 4> Load(ConstRef<Vector4> Value)
        {
            return Array(Value.GetX(), Value.GetY(), Value.GetZ(), Value.GetW());
        }

        ZY_INLINE static Vector4 Store(ConstRef<Array<Real32, 4>> Value)
        {
            return Vector4(Value[0], Value[1], Value[2], Value[3]);
        }
    };

    /// \brief Checks whether a type is a vector ImGui's scalar widgets can edit component by component.
    template<typename Type>
    concept IsVector = requires { Components<Type>::kCount; };

    /// \brief Edits scalars by dragging.
    Bool DragN(
        Text             Label,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        UInt             Count,
        Real32           Speed,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags);

    /// \brief Edits scalars with sliders.
    Bool SliderN(
        Text             Label,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        UInt             Count,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags);

    /// \brief Edits scalars by typing.
    Bool InputN(
        Text                Label,
        ImGuiDataType       Kind,
        Ptr<void>           Data,
        UInt                Count,
        ConstPtr<void>      Step,
        ConstPtr<void>      StepFast,
        Text                Format,
        ImGuiInputTextFlags Flags);

    /// \brief Edits a colour held as four reals in [0, 1].
    Bool ColorN(Text Label, Ptr<Real32> Channels, ImGuiColorEditFlags Flags, Bool Picker);

    /// \brief Edits a colour of any channel type, converting it to reals in [0, 1] for ImGui and back.
    template<typename Type>
    Bool EditColor(Text Label, Ref<AnyColor<Type>> Value, ImGuiColorEditFlags Flags, Bool Picker)
    {
        constexpr Real32 kLimit = IsReal<Type> ? 1.0f : static_cast<Real32>(kMaximum<Type>);

        Array Channels(
            static_cast<Real32>(Value.GetRed())   / kLimit,
            static_cast<Real32>(Value.GetGreen()) / kLimit,
            static_cast<Real32>(Value.GetBlue())  / kLimit,
            static_cast<Real32>(Value.GetAlpha()) / kLimit);

        if (!ColorN(Label, Channels.GetData(), Flags, Picker))
        {
            return false;
        }

        // A colour kept as integers rounds to its nearest step, and cannot hold what an HDR picker lets past one.
        const auto Encode = [](Real32 Channel)
        {
            if constexpr (IsReal<Type>)
            {
                return static_cast<Type>(Channel);
            }
            else
            {
                return static_cast<Type>(Clamp(Channel, 0.0f, 1.0f) * kLimit + 0.5f);
            }
        };

        Value = AnyColor<Type>(Encode(Channels[0]), Encode(Channels[1]), Encode(Channels[2]), Encode(Channels[3]));
        return true;
    }

    /// \brief Edits a copy of a piece of text.
    ///
    /// \return `true` with \p Edited viewing the copy if it changed, otherwise `false`.
    Bool EditText(Text Label, Text Value, ImGuiInputTextFlags Flags, Ref<Text> Edited);

    /// \brief Edits a copy of a piece of text with a button after it.
    ///
    /// \return `true` with \p Edited viewing the copy if it changed, otherwise `false`.
    Bool EditTextWithButton(
        Text                Label,
        Text                Value,
        ImGuiInputTextFlags Flags,
        Ref<Text>           Edited,
        Text                Button,
        Ref<Bool>           Clicked);

    /// \brief Edits a scalar by dragging, behind a tag naming its axis.
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
        ImGuiSliderFlags Flags);

    /// \brief Edits scalars by dragging, each behind the tag of its axis.
    Bool DragAxesN(
        Text             Label,
        ImGuiDataType    Kind,
        Ptr<void>        Data,
        UInt             Count,
        Real32           Speed,
        ConstPtr<void>   Min,
        ConstPtr<void>   Max,
        Text             Format,
        ImGuiSliderFlags Flags);
}

namespace ImGui
{
    /// \brief Shows a piece of text as it is, which need not end in a NUL.
    ///
    /// \param Value The text to show.
    void TextUnformatted(::Text Value);

    /// \brief Shows a piece of text in the disabled colour.
    ///
    /// \param Value The text to show.
    void TextDisabled(::Text Value);

    /// \brief Shows a piece of text in a colour of its own.
    ///
    /// \param Tint  The colour of the text.
    /// \param Value The text to show.
    void TextColored(ImVec4 Tint, ::Text Value);

    /// \brief Shows a piece of text, wrapped at the edge of the window.
    ///
    /// \param Value The text to show.
    void TextWrapped(::Text Value);

    /// \brief Shows a tooltip holding a piece of text, replacing any other, unless the text is empty.
    ///
    /// \param Value The text to show.
    void SetTooltip(::Text Value);

    /// \brief Shows a tooltip holding a piece of text while the last item is hovered, unless the text is empty.
    ///
    /// \param Value The text to show.
    void SetItemTooltip(::Text Value);

    /// \brief Pushes a piece of text onto the identifier stack, which need not end in a NUL.
    ///
    /// \param Value The text to push.
    void PushID(::Text Value);

    /// \brief Gets the identifier a piece of text hashes to under the current identifier stack.
    ///
    /// \param Value The text to hash.
    /// \return The identifier.
    ImGuiID GetID(::Text Value);

    /// \brief Shows text formatted from a pattern, as `Str::Print` formats it.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void Text(AnyRef<Arguments>... Parameters)
    {
        Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
        Scratch.template Format<Pattern>(Parameters...);
        TextUnformatted(::Text(Scratch.GetData(), Scratch.GetSize()));
    }

    /// \brief Shows text formatted from a pattern, in the disabled colour.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void TextDisabled(AnyRef<Arguments>... Parameters)
    {
        Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
        Scratch.template Format<Pattern>(Parameters...);
        TextDisabled(::Text(Scratch.GetData(), Scratch.GetSize()));
    }

    /// \brief Shows text formatted from a pattern, in a colour of its own.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Tint       The colour of the text.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void TextColored(ImVec4 Tint, AnyRef<Arguments>... Parameters)
    {
        Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
        Scratch.template Format<Pattern>(Parameters...);
        TextColored(Tint, ::Text(Scratch.GetData(), Scratch.GetSize()));
    }

    /// \brief Shows text formatted from a pattern, wrapped at the edge of the window.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void TextWrapped(AnyRef<Arguments>... Parameters)
    {
        Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
        Scratch.template Format<Pattern>(Parameters...);
        TextWrapped(::Text(Scratch.GetData(), Scratch.GetSize()));
    }

    /// \brief Shows a tooltip holding text formatted from a pattern, replacing any other shown this frame.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void SetTooltip(AnyRef<Arguments>... Parameters)
    {
        Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
        Scratch.template Format<Pattern>(Parameters...);
        SetTooltip(::Text(Scratch.GetData(), Scratch.GetSize()));
    }

    /// \brief Shows a tooltip holding text formatted from a pattern while the last item is hovered.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void SetItemTooltip(AnyRef<Arguments>... Parameters)
    {
        // Formatting is skipped for the many frames the item is not hovered.
        if (IsItemHovered(ImGuiHoveredFlags_ForTooltip))
        {
            Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
            Scratch.template Format<Pattern>(Parameters...);
            SetTooltip(::Text(Scratch.GetData(), Scratch.GetSize()));
        }
    }

    /// \brief Opens a window, whose title need not end in a NUL.
    ///
    /// \param Name  The window's title, which also identifies it.
    /// \param Open  The flag a close button clears, or `nullptr` for no close button.
    /// \param Flags The flags the window is drawn with.
    /// \return `true` if the window is shown, otherwise `false`; \ref End is called either way.
    Bool Begin(::Text Name, Ptr<Bool> Open = nullptr, ImGuiWindowFlags Flags = ImGuiWindowFlags_None);

    /// \brief Opens a child window, whose identifier need not end in a NUL.
    ///
    /// \param ID          The child's identifier.
    /// \param Size        The size of the child; zero on an axis for the remaining space.
    /// \param ChildFlags  The flags the child is laid out with.
    /// \param WindowFlags The flags the child's window is drawn with.
    /// \return `true` if the child is shown, otherwise `false`; \ref EndChild is called either way.
    Bool BeginChild(
        ::Text           ID,
        ImVec2           Size        = ImVec2(0.0f, 0.0f),
        ImGuiChildFlags  ChildFlags  = ImGuiChildFlags_None,
        ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_None);

    /// \brief Shows a button.
    ///
    /// \param Label The button's label.
    /// \param Size  The size of the button; zero on an axis to fit the label.
    /// \return `true` if the button was clicked, otherwise `false`.
    Bool Button(::Text Label, ImVec2 Size = ImVec2(0.0f, 0.0f));

    /// \brief Shows a button without vertical padding, to sit within a line of text.
    ///
    /// \param Label The button's label.
    /// \return `true` if the button was clicked, otherwise `false`.
    Bool SmallButton(::Text Label);

    /// \brief Reserves an area that behaves as a button but draws nothing.
    ///
    /// \param ID    The button's identifier.
    /// \param Size  The size of the area.
    /// \param Flags The flags the button behaves with.
    /// \return `true` if the area was clicked, otherwise `false`.
    Bool InvisibleButton(::Text ID, ImVec2 Size, ImGuiButtonFlags Flags = ImGuiButtonFlags_None);

    /// \brief Shows a checkbox.
    ///
    /// \param Label The checkbox's label.
    /// \param Value The flag the checkbox shows and toggles.
    /// \return `true` if the flag changed, otherwise `false`.
    Bool Checkbox(::Text Label, Ptr<Bool> Value);

    /// \brief Shows a radio button.
    ///
    /// \param Label  The button's label.
    /// \param Active Whether the button is shown as chosen.
    /// \return `true` if the button was clicked, otherwise `false`.
    Bool RadioButton(::Text Label, Bool Active);

    /// \brief Shows a selectable row.
    ///
    /// \param Label    The row's label.
    /// \param Selected Whether the row is shown as selected.
    /// \param Flags    The flags the row behaves with.
    /// \param Size     The size of the row; zero on an axis for the default.
    /// \return `true` if the row was clicked, otherwise `false`.
    Bool Selectable(
        ::Text               Label,
        Bool                 Selected = false,
        ImGuiSelectableFlags Flags    = ImGuiSelectableFlags_None,
        ImVec2               Size     = ImVec2(0.0f, 0.0f));

    /// \brief Shows a selectable row that toggles a flag when clicked.
    ///
    /// \param Label    The row's label.
    /// \param Selected The flag the row shows and toggles.
    /// \param Flags    The flags the row behaves with.
    /// \param Size     The size of the row; zero on an axis for the default.
    /// \return `true` if the row was clicked, otherwise `false`.
    Bool Selectable(
        ::Text               Label,
        Ptr<Bool>            Selected,
        ImGuiSelectableFlags Flags = ImGuiSelectableFlags_None,
        ImVec2               Size  = ImVec2(0.0f, 0.0f));

    /// \brief Opens a tree node.
    ///
    /// \param Label The node's label.
    /// \return `true` if the node is open, in which case \ref TreePop must be called, otherwise `false`.
    Bool TreeNode(::Text Label);

    /// \brief Opens a tree node drawn with flags of its own.
    ///
    /// \param Label The node's label.
    /// \param Flags The flags the node is drawn with.
    /// \return `true` if the node is open, otherwise `false`.
    Bool TreeNodeEx(::Text Label, ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None);

    /// \brief Shows a header that folds the items under it.
    ///
    /// \param Label The header's label.
    /// \param Flags The flags the header is drawn with.
    /// \return `true` if the header is open, otherwise `false`.
    Bool CollapsingHeader(::Text Label, ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None);

    /// \brief Shows a header that folds the items under it, with a button that hides it.
    ///
    /// \param Label   The header's label.
    /// \param Visible The flag the close button clears; while it is cleared the header is not shown.
    /// \param Flags   The flags the header is drawn with.
    /// \return `true` if the header is open, otherwise `false`.
    Bool CollapsingHeader(::Text Label, Ptr<Bool> Visible, ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None);

    /// \brief Opens a sub-menu.
    ///
    /// \param Label   The menu's label.
    /// \param Enabled Whether the menu can be opened.
    /// \return `true` if the menu is open, in which case \ref EndMenu must be called, otherwise `false`.
    Bool BeginMenu(::Text Label, Bool Enabled = true);

    /// \brief Shows an entry in a menu.
    ///
    /// \param Label    The entry's label.
    /// \param Shortcut The shortcut shown beside the label, or empty for none.
    /// \param Selected Whether the entry is shown with a check mark.
    /// \param Enabled  Whether the entry can be activated.
    /// \return `true` if the entry was activated, otherwise `false`.
    Bool MenuItem(::Text Label, ::Text Shortcut = {}, Bool Selected = false, Bool Enabled = true);

    /// \brief Shows an entry in a menu that toggles a flag when activated.
    ///
    /// \param Label    The entry's label.
    /// \param Shortcut The shortcut shown beside the label, or empty for none.
    /// \param Selected The flag the entry shows with a check mark and toggles.
    /// \param Enabled  Whether the entry can be activated.
    /// \return `true` if the entry was activated, otherwise `false`.
    Bool MenuItem(::Text Label, ::Text Shortcut, Ptr<Bool> Selected, Bool Enabled = true);

    /// \brief Opens a combo.
    ///
    /// \param Label   The combo's label.
    /// \param Preview The text shown in the closed combo.
    /// \param Flags   The flags the combo is drawn with.
    /// \return `true` if the combo is open, in which case \ref EndCombo must be called, otherwise `false`.
    Bool BeginCombo(::Text Label, ::Text Preview, ImGuiComboFlags Flags = ImGuiComboFlags_None);

    /// \brief Opens a tab bar.
    ///
    /// \param ID    The tab bar's identifier.
    /// \param Flags The flags the tab bar is drawn with.
    /// \return `true` if the tab bar is shown, in which case \ref EndTabBar must be called, otherwise `false`.
    Bool BeginTabBar(::Text ID, ImGuiTabBarFlags Flags = ImGuiTabBarFlags_None);

    /// \brief Opens a tab.
    ///
    /// \param Label The tab's label.
    /// \param Open  The flag a close button clears, or `nullptr` for no close button.
    /// \param Flags The flags the tab is drawn with.
    /// \return `true` if the tab is selected, in which case \ref EndTabItem must be called, otherwise `false`.
    Bool BeginTabItem(::Text Label, Ptr<Bool> Open = nullptr, ImGuiTabItemFlags Flags = ImGuiTabItemFlags_None);

    /// \brief Marks a popup as open.
    ///
    /// \param ID    The popup's identifier.
    /// \param Flags The flags the popup opens with.
    /// \return `true` if the popup was opened, otherwise `false`.
    Bool OpenPopup(::Text ID, ImGuiPopupFlags Flags = ImGuiPopupFlags_None);

    /// \brief Opens a popup that was marked open.
    ///
    /// \param ID    The popup's identifier.
    /// \param Flags The flags the popup's window is drawn with.
    /// \return `true` if the popup is open, in which case \ref EndPopup must be called, otherwise `false`.
    Bool BeginPopup(::Text ID, ImGuiWindowFlags Flags = ImGuiWindowFlags_None);

    /// \brief Opens a modal popup that was marked open.
    ///
    /// \param Name  The popup's title, which also identifies it.
    /// \param Open  The flag a close button clears, or `nullptr` for no close button.
    /// \param Flags The flags the popup's window is drawn with.
    /// \return `true` if the popup is open, in which case \ref EndPopup must be called, otherwise `false`.
    Bool BeginPopupModal(::Text Name, Ptr<Bool> Open = nullptr, ImGuiWindowFlags Flags = ImGuiWindowFlags_None);

    /// \brief Opens a popup when the last item is right-clicked.
    ///
    /// \param ID    The popup's identifier.
    /// \param Flags The flags the popup opens with.
    /// \return `true` if the popup is open, in which case \ref EndPopup must be called, otherwise `false`.
    Bool BeginPopupContextItem(::Text ID, ImGuiPopupFlags Flags = ImGuiPopupFlags_None);

    /// \brief Opens a popup when the current window is right-clicked.
    ///
    /// \param ID    The popup's identifier.
    /// \param Flags The flags the popup opens with.
    /// \return `true` if the popup is open, in which case \ref EndPopup must be called, otherwise `false`.
    Bool BeginPopupContextWindow(::Text ID, ImGuiPopupFlags Flags = ImGuiPopupFlags_None);

    /// \brief Checks whether a popup is open.
    ///
    /// \param ID    The popup's identifier.
    /// \param Flags The flags that widen the check.
    /// \return `true` if the popup is open, otherwise `false`.
    Bool IsPopupOpen(::Text ID, ImGuiPopupFlags Flags = ImGuiPopupFlags_None);

    /// \brief Opens a table.
    ///
    /// \param ID         The table's identifier.
    /// \param Columns    The number of columns.
    /// \param Flags      The flags the table is drawn with.
    /// \param OuterSize  The size of the table; zero on an axis for the default.
    /// \param InnerWidth The width of the table's contents, when it scrolls horizontally.
    /// \return `true` if the table is shown, in which case \ref EndTable must be called, otherwise `false`.
    Bool BeginTable(
        ::Text           ID,
        SInt32           Columns,
        ImGuiTableFlags  Flags      = ImGuiTableFlags_None,
        ImVec2           OuterSize  = ImVec2(0.0f, 0.0f),
        Real32           InnerWidth = 0.0f);

    /// \brief Describes a column of the current table.
    ///
    /// \param Label  The column's header.
    /// \param Flags  The flags the column is drawn with.
    /// \param Width  The column's initial width, or its weight when it stretches.
    /// \param UserID The identifier the column is reported with when sorting.
    void TableSetupColumn(
        ::Text                Label,
        ImGuiTableColumnFlags Flags  = ImGuiTableColumnFlags_None,
        Real32                Width  = 0.0f,
        ImGuiID               UserID = 0);

    /// \brief Shows a horizontal line with a piece of text within it.
    ///
    /// \param Label The text to show.
    void SeparatorText(::Text Label);

    /// \brief Measures a piece of text, which need not end in a NUL.
    ///
    /// \param Value              The text to measure.
    /// \param HideAfterDoubleHash Whether the text after a `##` is left out, as a label hides it.
    /// \param WrapWidth          The width the text wraps at, or a negative value for no wrapping.
    /// \return The size of the text in screen pixels.
    ImVec2 CalcTextSize(::Text Value, Bool HideAfterDoubleHash = false, Real32 WrapWidth = -1.0f);

    /// \brief Edits a string in a single-line field, of any length.
    ///
    /// \param Label    The field's label.
    /// \param Value    The string to edit.
    /// \param Flags    The flags the field is drawn with.
    /// \param Callback The callback ImGui also calls, as the flags ask, or `nullptr`.
    /// \param UserData The data handed to \p Callback.
    /// \return `true` if the string changed, otherwise `false`.
    Bool InputText(
        ::Text                 Label,
        Ref<Str>               Value,
        ImGuiInputTextFlags    Flags    = ImGuiInputTextFlags_None,
        ImGuiInputTextCallback Callback = nullptr,
        Ptr<void>              UserData = nullptr);

    /// \brief Edits a string in a multi-line field, of any length.
    ///
    /// \param Label    The field's label.
    /// \param Value    The string to edit.
    /// \param Size     The size of the field; zero on an axis for the default.
    /// \param Flags    The flags the field is drawn with.
    /// \param Callback The callback ImGui also calls, as the flags ask, or `nullptr`.
    /// \param UserData The data handed to \p Callback.
    /// \return `true` if the string changed, otherwise `false`.
    Bool InputTextMultiline(
        ::Text                 Label,
        Ref<Str>               Value,
        ImVec2                 Size     = ImVec2(0.0f, 0.0f),
        ImGuiInputTextFlags    Flags    = ImGuiInputTextFlags_None,
        ImGuiInputTextCallback Callback = nullptr,
        Ptr<void>              UserData = nullptr);

    /// \brief Edits a string in a single-line field showing a hint while it is empty, of any length.
    ///
    /// \param Label    The field's label.
    /// \param Hint     The text shown while the field is empty.
    /// \param Value    The string to edit.
    /// \param Flags    The flags the field is drawn with.
    /// \param Callback The callback ImGui also calls, as the flags ask, or `nullptr`.
    /// \param UserData The data handed to \p Callback.
    /// \return `true` if the string changed, otherwise `false`.
    Bool InputTextWithHint(
        ::Text                 Label,
        ::Text                 Hint,
        Ref<Str>               Value,
        ImGuiInputTextFlags    Flags    = ImGuiInputTextFlags_None,
        ImGuiInputTextCallback Callback = nullptr,
        Ptr<void>              UserData = nullptr);

    /// \brief Edits a piece of text the caller cannot hand over as a string, such as one read through a getter,
    ///        reporting each change to a callback instead, of any length.
    ///
    /// \param Label    The field's label.
    /// \param Value    The text the field shows.
    /// \param OnChange The callback given the text as edited, valid only until the next field is edited.
    /// \param Flags    The flags the field is drawn with.
    /// \return `true` if the text changed, otherwise `false`.
    template<typename Action>
        requires requires(Ref<Action> Call, ::Text Edited) { Call(Edited); }
    Bool InputText(::Text Label, ::Text Value, AnyRef<Action> OnChange, ImGuiInputTextFlags Flags = ImGuiInputTextFlags_None)
    {
        ::Text Edited;

        if (!ZyPlugin::Detail::EditText(Label, Value, Flags, Edited))
        {
            return false;
        }

        OnChange(Edited);
        return true;
    }

    /// \brief Edits a piece of text as \ref InputText does, with a button after it, sharing the item width
    ///        between the two.
    ///
    /// \param Label    The field's label, shown after the button unless hidden with `##`.
    /// \param Value    The text the field shows.
    /// \param OnChange The callback given the text as edited, valid only until the next field is edited.
    /// \param Button   The button's label.
    /// \param OnClick  The callback called when the button is clicked.
    /// \param Flags    The flags the field is drawn with.
    /// \return `true` if the text changed, otherwise `false`.
    template<typename Action, typename Click>
        requires requires(Ref<Action> Call, ::Text Edited) { Call(Edited); }
    Bool InputTextWithButton(
        ::Text              Label,
        ::Text              Value,
        AnyRef<Action>      OnChange,
        ::Text              Button,
        AnyRef<Click>       OnClick,
        ImGuiInputTextFlags Flags = ImGuiInputTextFlags_None)
    {
        ::Text Edited;
        Bool   Clicked = false;

        const Bool Changed = ZyPlugin::Detail::EditTextWithButton(Label, Value, Flags, Edited, Button, Clicked);

        if (Changed)
        {
            OnChange(Edited);
        }

        if (Clicked)
        {
            OnClick();
        }
        return Changed;
    }

    /// \brief Picks one value of an enumeration from a combo, offering only the values a filter accepts.
    ///
    /// \param Label  The combo's label.
    /// \param Value  The value to pick.
    /// \param Accept The filter each value is offered through, called with the value.
    /// \return `true` if the value changed, otherwise `false`.
    template<IsEnum Type, typename Filter>
    Bool Combo(::Text Label, Ref<Type> Value, AnyRef<Filter> Accept)
    {
        Bool Changed = false;

        if (BeginCombo(Label, ZyEnum::GetName(Value)))
        {
            for (const Type Candidate : ZyEnum::GetValues<Type>())
            {
                if (!Accept(Candidate))
                {
                    continue;
                }

                const Bool Selected = (Candidate == Value);

                if (Selectable(ZyEnum::GetName(Candidate), Selected))
                {
                    Changed = !Selected;
                    Value   = Candidate;
                }

                if (Selected)
                {
                    SetItemDefaultFocus();
                }
            }
            EndCombo();
        }
        return Changed;
    }

    /// \brief Picks one value of an enumeration from a combo, offering every value it declares.
    ///
    /// \param Label The combo's label.
    /// \param Value The value to pick.
    /// \return `true` if the value changed, otherwise `false`.
    template<IsEnum Type>
    Bool Combo(::Text Label, Ref<Type> Value)
    {
        return Combo(Label, Value, [](Type) { return true; });
    }

    /// \brief Edits a number by dragging.
    ///
    /// \param Label  The field's label.
    /// \param Value  The number to edit.
    /// \param Speed  How much the number changes for each pixel the mouse moves.
    /// \param Min    The smallest value allowed, which with \p Max equal to it means no bounds.
    /// \param Max    The largest value allowed.
    /// \param Format The pattern the number is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags  The flags the field behaves with.
    /// \return `true` if the number changed, otherwise `false`.
    template<ZyPlugin::Detail::IsScalar Type>
    Bool Drag(
        ::Text                           Label,
        Ref<Type>                        Value,
        Real32                           Speed  = 1.0f,
        ZyPlugin::Detail::Exactly<Type>  Min    = Type(0),
        ZyPlugin::Detail::Exactly<Type>  Max    = Type(0),
        ::Text                           Format = {},
        ImGuiSliderFlags                 Flags  = ImGuiSliderFlags_None)
    {
        return ZyPlugin::Detail::DragN(
            Label,
            ZyPlugin::Detail::GetDataType<Type>(),
            AddressOf(Value),
            1,
            Speed,
            AddressOf(Min),
            AddressOf(Max),
            Format,
            Flags);
    }

    /// \brief Edits a vector by dragging each of its components.
    ///
    /// \param Label  The field's label.
    /// \param Value  The vector to edit.
    /// \param Speed  How much a component changes for each pixel the mouse moves.
    /// \param Min    The smallest value a component may take, which with \p Max equal to it means no bounds.
    /// \param Max    The largest value a component may take.
    /// \param Format The pattern each component is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags  The flags the fields behave with.
    /// \return `true` if the vector changed, otherwise `false`.
    template<ZyPlugin::Detail::IsVector Vector, typename Traits = ZyPlugin::Detail::Components<Vector>>
    Bool Drag(
        ::Text                   Label,
        Ref<Vector>              Value,
        Real32                   Speed  = 1.0f,
        typename Traits::Scalar  Min    = 0,
        typename Traits::Scalar  Max    = 0,
        ::Text                   Format = {},
        ImGuiSliderFlags         Flags  = ImGuiSliderFlags_None)
    {
        auto Data = Traits::Load(Value);

        if (!ZyPlugin::Detail::DragN(
            Label,
            ZyPlugin::Detail::GetDataType<typename Traits::Scalar>(),
            Data.GetData(),
            Traits::kCount,
            Speed,
            AddressOf(Min),
            AddressOf(Max),
            Format,
            Flags))
        {
            return false;
        }

        Value = Traits::Store(Data);
        return true;
    }

    /// \brief Edits a number with a slider.
    ///
    /// \param Label  The field's label.
    /// \param Value  The number to edit.
    /// \param Min    The value at the slider's left end.
    /// \param Max    The value at the slider's right end.
    /// \param Format The pattern the number is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags  The flags the field behaves with.
    /// \return `true` if the number changed, otherwise `false`.
    template<ZyPlugin::Detail::IsScalar Type>
    Bool Slider(
        ::Text                          Label,
        Ref<Type>                       Value,
        ZyPlugin::Detail::Exactly<Type> Min,
        ZyPlugin::Detail::Exactly<Type> Max,
        ::Text                          Format = {},
        ImGuiSliderFlags                Flags  = ImGuiSliderFlags_None)
    {
        return ZyPlugin::Detail::SliderN(
            Label,
            ZyPlugin::Detail::GetDataType<Type>(),
            AddressOf(Value),
            1,
            AddressOf(Min),
            AddressOf(Max),
            Format,
            Flags);
    }

    /// \brief Edits a vector with a slider for each of its components.
    ///
    /// \param Label  The field's label.
    /// \param Value  The vector to edit.
    /// \param Min    The value at each slider's left end.
    /// \param Max    The value at each slider's right end.
    /// \param Format The pattern each component is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags  The flags the fields behave with.
    /// \return `true` if the vector changed, otherwise `false`.
    template<ZyPlugin::Detail::IsVector Vector, typename Traits = ZyPlugin::Detail::Components<Vector>>
    Bool Slider(
        ::Text                  Label,
        Ref<Vector>             Value,
        typename Traits::Scalar Min,
        typename Traits::Scalar Max,
        ::Text                  Format = {},
        ImGuiSliderFlags        Flags  = ImGuiSliderFlags_None)
    {
        auto Data = Traits::Load(Value);

        if (!ZyPlugin::Detail::SliderN(
            Label,
            ZyPlugin::Detail::GetDataType<typename Traits::Scalar>(),
            Data.GetData(),
            Traits::kCount,
            AddressOf(Min),
            AddressOf(Max),
            Format,
            Flags))
        {
            return false;
        }

        Value = Traits::Store(Data);
        return true;
    }

    /// \brief Edits a number by typing it.
    ///
    /// \param Label    The field's label.
    /// \param Value    The number to edit.
    /// \param Step     How much the step buttons change the number, or zero for no buttons.
    /// \param StepFast How much the step buttons change the number while Ctrl is held.
    /// \param Format   The pattern the number is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags    The flags the field behaves with.
    /// \return `true` if the number changed, otherwise `false`.
    template<ZyPlugin::Detail::IsScalar Type>
    Bool Input(
        ::Text                          Label,
        Ref<Type>                       Value,
        ZyPlugin::Detail::Exactly<Type> Step     = Type(0),
        ZyPlugin::Detail::Exactly<Type> StepFast = Type(0),
        ::Text                          Format   = {},
        ImGuiInputTextFlags             Flags    = ImGuiInputTextFlags_None)
    {
        return ZyPlugin::Detail::InputN(
            Label,
            ZyPlugin::Detail::GetDataType<Type>(),
            AddressOf(Value),
            1,
            Step != Type(0) ? AddressOf(Step) : nullptr,
            Step != Type(0) && StepFast != Type(0) ? AddressOf(StepFast) : nullptr,
            Format,
            Flags);
    }

    /// \brief Edits a vector by typing each of its components.
    ///
    /// \param Label    The field's label.
    /// \param Value    The vector to edit.
    /// \param Step     How much the step buttons change a component, or zero for no buttons.
    /// \param StepFast How much the step buttons change a component while Ctrl is held.
    /// \param Format   The pattern each component is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags    The flags the fields behave with.
    /// \return `true` if the vector changed, otherwise `false`.
    template<ZyPlugin::Detail::IsVector Vector, typename Traits = ZyPlugin::Detail::Components<Vector>>
    Bool Input(
        ::Text                  Label,
        Ref<Vector>             Value,
        typename Traits::Scalar Step     = 0,
        typename Traits::Scalar StepFast = 0,
        ::Text                  Format   = {},
        ImGuiInputTextFlags     Flags    = ImGuiInputTextFlags_None)
    {
        using Scalar = typename Traits::Scalar;

        auto Data = Traits::Load(Value);

        if (!ZyPlugin::Detail::InputN(
            Label,
            ZyPlugin::Detail::GetDataType<Scalar>(),
            Data.GetData(),
            Traits::kCount,
            Step != Scalar(0) ? AddressOf(Step) : nullptr,
            Step != Scalar(0) && StepFast != Scalar(0) ? AddressOf(StepFast) : nullptr,
            Format,
            Flags))
        {
            return false;
        }

        Value = Traits::Store(Data);
        return true;
    }

    /// \brief Edits a colour with a swatch and a field per channel, whether it keeps its channels.
    ///
    /// \param Label The field's label.
    /// \param Value The colour to edit.
    /// \param Flags The flags the field is drawn with; `ImGuiColorEditFlags_NoAlpha` leaves alpha untouched.
    /// \return `true` if the colour changed, otherwise `false`.
    template<typename Type>
    Bool ColorEdit(::Text Label, Ref<AnyColor<Type>> Value, ImGuiColorEditFlags Flags = ImGuiColorEditFlags_None)
    {
        return ZyPlugin::Detail::EditColor(Label, Value, Flags, false);
    }

    /// \brief Edits a colour with a full picker, whether it keeps its channels as reals or as integers.
    ///
    /// \param Label The picker's label.
    /// \param Value The colour to edit.
    /// \param Flags The flags the picker is drawn with; `ImGuiColorEditFlags_NoAlpha` leaves alpha untouched.
    /// \return `true` if the colour changed, otherwise `false`.
    template<typename Type>
    Bool ColorPicker(::Text Label, Ref<AnyColor<Type>> Value, ImGuiColorEditFlags Flags = ImGuiColorEditFlags_None)
    {
        return ZyPlugin::Detail::EditColor(Label, Value, Flags, true);
    }

    /// \brief Holds the colours the axis widgets tag their fields with, which ImGui's own style has no room for.
    struct AxisStyle final
    {
        /// The colour of the tag on each component, in the order X, Y, Z and W.
        Array<ImU32, 4> Tints;

        /// The colour of the letter written on each tag.
        ImU32           Letter;
    };

    /// \brief Gets the colours the axis widgets tag their fields with, shared by every ImGui context.
    ///
    /// \return The style, which may be changed to match a theme.
    Ref<AxisStyle> GetAxisStyle();

    /// \brief Shows a button that stays pressed while a state is active.
    ///
    /// \param Label  The button's label.
    /// \param Active Whether the button is shown pressed.
    /// \param Size   The size of the button; zero on an axis to fit the label.
    /// \return `true` if the button was clicked, otherwise `false`.
    Bool ToggleButton(::Text Label, Bool Active, ImVec2 Size = ImVec2(0.0f, 0.0f));

    /// \brief Shows a piece of text centred in the space left on the current line.
    ///
    /// \param Value The text to show.
    void TextCentered(::Text Value);

    /// \brief Shows text formatted from a pattern, centred in the space left on the current line.
    ///
    /// \tparam Pattern   The pattern, such as `"{0} x {1}"`.
    /// \param Parameters The values the pattern's placeholders take.
    template<Symbol Pattern, typename... Arguments>
    void TextCentered(AnyRef<Arguments>... Parameters)
    {
        Ref<Str> Scratch = ZyPlugin::Detail::GetScratch();
        Scratch.template Format<Pattern>(Parameters...);
        TextCentered(::Text(Scratch.GetData(), Scratch.GetSize()));
    }

    /// \brief Edits a number by dragging, behind a coloured tag naming its axis, filling the item width.
    ///
    /// \param ID     The field's identifier.
    /// \param Tag    The text on the tag, such as `"X"`.
    /// \param Tint   The colour of the tag.
    /// \param Value  The number to edit.
    /// \param Speed  How much the number changes for each pixel the mouse moves.
    /// \param Min    The smallest value allowed, which with \p Max equal to it means no bounds.
    /// \param Max    The largest value allowed.
    /// \param Format The pattern the number is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags  The flags the field behaves with.
    /// \return `true` if the number changed, otherwise `false`.
    template<ZyPlugin::Detail::IsScalar Type>
    Bool DragAxis(
        ::Text                          ID,
        ::Text                          Tag,
        ImU32                           Tint,
        Ref<Type>                       Value,
        Real32                          Speed  = 1.0f,
        ZyPlugin::Detail::Exactly<Type> Min    = Type(0),
        ZyPlugin::Detail::Exactly<Type> Max    = Type(0),
        ::Text                          Format = {},
        ImGuiSliderFlags                Flags  = ImGuiSliderFlags_None)
    {
        return ZyPlugin::Detail::DragAxisN(
            ID,
            Tag,
            Tint,
            ZyPlugin::Detail::GetDataType<Type>(),
            AddressOf(Value),
            Speed,
            AddressOf(Min),
            AddressOf(Max),
            Format,
            Flags);
    }

    /// \brief Edits a vector by dragging each of its components, each behind a tag naming its axis in the colours.
    ///
    /// \param Label  The field's label, shown after the fields unless hidden with `##`.
    /// \param Value  The vector to edit.
    /// \param Speed  How much a component changes for each pixel the mouse moves.
    /// \param Min    The smallest value a component may take, which with \p Max equal to it means no bounds.
    /// \param Max    The largest value a component may take.
    /// \param Format The pattern each component is shown with, as `printf` reads it, or empty for the default.
    /// \param Flags  The flags the fields behave with.
    /// \return `true` if the vector changed, otherwise `false`.
    template<ZyPlugin::Detail::IsVector Vector, typename Traits = ZyPlugin::Detail::Components<Vector>>
    Bool DragAxes(
        ::Text                  Label,
        Ref<Vector>             Value,
        Real32                  Speed  = 1.0f,
        typename Traits::Scalar Min    = 0,
        typename Traits::Scalar Max    = 0,
        ::Text                  Format = {},
        ImGuiSliderFlags        Flags  = ImGuiSliderFlags_None)
    {
        auto Data = Traits::Load(Value);

        if (!ZyPlugin::Detail::DragAxesN(
            Label,
            ZyPlugin::Detail::GetDataType<typename Traits::Scalar>(),
            Data.GetData(),
            Traits::kCount,
            Speed,
            AddressOf(Min),
            AddressOf(Max),
            Format,
            Flags))
        {
            return false;
        }

        Value = Traits::Store(Data);
        return true;
    }
}