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

#include "ImGuiSystem.hpp"
#include <Zyphryon.Input/Service.hpp>
#include <Zyphryon.Platform/Service.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyPlugin
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ImGuiKey GetKey(ZyInput::Key Key)
    {
        switch (Key)
        {
        case ZyInput::Key::Space:
            return ImGuiKey_Space;
        case ZyInput::Key::Apostrophe:
            return ImGuiKey_Apostrophe;
        case ZyInput::Key::Comma:
            return ImGuiKey_Comma;
        case ZyInput::Key::Minus:
            return ImGuiKey_Minus;
        case ZyInput::Key::Period:
            return ImGuiKey_Period;
        case ZyInput::Key::Slash:
            return ImGuiKey_Slash;
        case ZyInput::Key::Alpha0:
            return ImGuiKey_0;
        case ZyInput::Key::Alpha1:
            return ImGuiKey_1;
        case ZyInput::Key::Alpha2:
            return ImGuiKey_2;
        case ZyInput::Key::Alpha3:
            return ImGuiKey_3;
        case ZyInput::Key::Alpha4:
            return ImGuiKey_4;
        case ZyInput::Key::Alpha5:
            return ImGuiKey_5;
        case ZyInput::Key::Alpha6:
            return ImGuiKey_6;
        case ZyInput::Key::Alpha7:
            return ImGuiKey_7;
        case ZyInput::Key::Alpha8:
            return ImGuiKey_8;
        case ZyInput::Key::Alpha9:
            return ImGuiKey_9;
        case ZyInput::Key::Semicolon:
            return ImGuiKey_Semicolon;
        case ZyInput::Key::Equal:
            return ImGuiKey_Equal;
        case ZyInput::Key::A:
            return ImGuiKey_A;
        case ZyInput::Key::B:
            return ImGuiKey_B;
        case ZyInput::Key::C:
            return ImGuiKey_C;
        case ZyInput::Key::D:
            return ImGuiKey_D;
        case ZyInput::Key::E:
            return ImGuiKey_E;
        case ZyInput::Key::F:
            return ImGuiKey_F;
        case ZyInput::Key::G:
            return ImGuiKey_G;
        case ZyInput::Key::H:
            return ImGuiKey_H;
        case ZyInput::Key::I:
            return ImGuiKey_I;
        case ZyInput::Key::J:
            return ImGuiKey_J;
        case ZyInput::Key::K:
            return ImGuiKey_K;
        case ZyInput::Key::L:
            return ImGuiKey_L;
        case ZyInput::Key::M:
            return ImGuiKey_M;
        case ZyInput::Key::N:
            return ImGuiKey_N;
        case ZyInput::Key::O:
            return ImGuiKey_O;
        case ZyInput::Key::P:
            return ImGuiKey_P;
        case ZyInput::Key::Q:
            return ImGuiKey_Q;
        case ZyInput::Key::R:
            return ImGuiKey_R;
        case ZyInput::Key::S:
            return ImGuiKey_S;
        case ZyInput::Key::T:
            return ImGuiKey_T;
        case ZyInput::Key::U:
            return ImGuiKey_U;
        case ZyInput::Key::V:
            return ImGuiKey_V;
        case ZyInput::Key::W:
            return ImGuiKey_W;
        case ZyInput::Key::X:
            return ImGuiKey_X;
        case ZyInput::Key::Y:
            return ImGuiKey_Y;
        case ZyInput::Key::Z:
            return ImGuiKey_Z;
        case ZyInput::Key::Backslash:
            return ImGuiKey_Backslash;
        case ZyInput::Key::Grave:
            return ImGuiKey_GraveAccent;
        case ZyInput::Key::Escape:
            return ImGuiKey_Escape;
        case ZyInput::Key::Enter:
            return ImGuiKey_Enter;
        case ZyInput::Key::Tab:
            return ImGuiKey_Tab;
        case ZyInput::Key::Backspace:
            return ImGuiKey_Backspace;
        case ZyInput::Key::Insert:
            return ImGuiKey_Insert;
        case ZyInput::Key::Delete:
            return ImGuiKey_Delete;
        case ZyInput::Key::Right:
            return ImGuiKey_RightArrow;
        case ZyInput::Key::Left:
            return ImGuiKey_LeftArrow;
        case ZyInput::Key::Down:
            return ImGuiKey_DownArrow;
        case ZyInput::Key::Up:
            return ImGuiKey_UpArrow;
        case ZyInput::Key::PageUp:
            return ImGuiKey_PageUp;
        case ZyInput::Key::PageDown:
            return ImGuiKey_PageDown;
        case ZyInput::Key::Home:
            return ImGuiKey_Home;
        case ZyInput::Key::End:
            return ImGuiKey_End;
        case ZyInput::Key::Capital:
            return ImGuiKey_CapsLock;
        case ZyInput::Key::Scroll:
            return ImGuiKey_ScrollLock;
        case ZyInput::Key::NumLock:
            return ImGuiKey_NumLock;
        case ZyInput::Key::Print:
            return ImGuiKey_PrintScreen;
        case ZyInput::Key::Pause:
            return ImGuiKey_Pause;
        case ZyInput::Key::F1:
            return ImGuiKey_F1;
        case ZyInput::Key::F2:
            return ImGuiKey_F2;
        case ZyInput::Key::F3:
            return ImGuiKey_F3;
        case ZyInput::Key::F4:
            return ImGuiKey_F4;
        case ZyInput::Key::F5:
            return ImGuiKey_F5;
        case ZyInput::Key::F6:
            return ImGuiKey_F6;
        case ZyInput::Key::F7:
            return ImGuiKey_F7;
        case ZyInput::Key::F8:
            return ImGuiKey_F8;
        case ZyInput::Key::F9:
            return ImGuiKey_F9;
        case ZyInput::Key::F10:
            return ImGuiKey_F10;
        case ZyInput::Key::F11:
            return ImGuiKey_F11;
        case ZyInput::Key::F12:
            return ImGuiKey_F12;
        case ZyInput::Key::Keypad0:
            return ImGuiKey_Keypad0;
        case ZyInput::Key::Keypad1:
            return ImGuiKey_Keypad1;
        case ZyInput::Key::Keypad2:
            return ImGuiKey_Keypad2;
        case ZyInput::Key::Keypad3:
            return ImGuiKey_Keypad3;
        case ZyInput::Key::Keypad4:
            return ImGuiKey_Keypad4;
        case ZyInput::Key::Keypad5:
            return ImGuiKey_Keypad5;
        case ZyInput::Key::Keypad6:
            return ImGuiKey_Keypad6;
        case ZyInput::Key::Keypad7:
            return ImGuiKey_Keypad7;
        case ZyInput::Key::Keypad8:
            return ImGuiKey_Keypad8;
        case ZyInput::Key::Keypad9:
            return ImGuiKey_Keypad9;
        case ZyInput::Key::Decimal:
            return ImGuiKey_KeypadDecimal;
        case ZyInput::Key::Divide:
            return ImGuiKey_KeypadDivide;
        case ZyInput::Key::Multiply:
            return ImGuiKey_KeypadMultiply;
        case ZyInput::Key::Subtract:
            return ImGuiKey_KeypadSubtract;
        case ZyInput::Key::Add:
            return ImGuiKey_KeypadAdd;
        case ZyInput::Key::LeftBracket:
            return ImGuiKey_LeftBracket;
        case ZyInput::Key::LeftShift:
            return ImGuiKey_LeftShift;
        case ZyInput::Key::LeftCtrl:
            return ImGuiKey_LeftCtrl;
        case ZyInput::Key::LeftAlt:
            return ImGuiKey_LeftAlt;
        case ZyInput::Key::LeftSuper:
            return ImGuiKey_LeftSuper;
        case ZyInput::Key::Menu:
            return ImGuiKey_Menu;
        case ZyInput::Key::RightBracket:
            return ImGuiKey_RightBracket;
        case ZyInput::Key::RightShift:
            return ImGuiKey_RightShift;
        case ZyInput::Key::RightCtrl:
            return ImGuiKey_RightCtrl;
        case ZyInput::Key::RightAlt:
            return ImGuiKey_RightAlt;
        case ZyInput::Key::RightSuper:
            return ImGuiKey_RightSuper;
        case ZyInput::Key::Unknown:
            break;
        }
        return ImGuiKey_None;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ImGuiKey GetKeyModifier(ZyInput::Key Key)
    {
        switch (Key)
        {
        case ZyInput::Key::LeftCtrl:
        case ZyInput::Key::RightCtrl:
            return ImGuiMod_Ctrl;
        case ZyInput::Key::LeftAlt:
        case ZyInput::Key::RightAlt:
            return ImGuiMod_Alt;
        case ZyInput::Key::LeftShift:
        case ZyInput::Key::RightShift:
            return ImGuiMod_Shift;
        case ZyInput::Key::LeftSuper:
        case ZyInput::Key::RightSuper:
            return ImGuiMod_Super;
        default:
            return ImGuiMod_None;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static ImGuiMouseButton GetButton(ZyInput::Button Button)
    {
        switch (Button)
        {
        case ZyInput::Button::Left:
            return ZyEnum::Cast(ImGuiMouseButton_Left);
        case ZyInput::Button::Middle:
            return ZyEnum::Cast(ImGuiMouseButton_Middle);
        case ZyInput::Button::Right:
            return ZyEnum::Cast(ImGuiMouseButton_Right);
        case ZyInput::Button::Back:
            return 3;
        case ZyInput::Button::Forward:
            return 4;
        default:
            return -1;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiSystem::Initialize(Ref<ZyEngine::Subsystem::Host> Host, Colorspace Space)
    {
        ConstRetainer<ZyPlatform::Service> Platform = Host.GetService<ZyPlatform::Service>();

        // Fetches the window and the monitor the window is at.
        ConstRef<ZyPlatform::Window>        Window  = Platform->GetWindow();
        const ConstPtr<ZyPlatform::Monitor> Monitor = Platform->GetDisplay().GetMonitor(Window.GetX(), Window.GetY());
        ZY_ASSERT(Monitor, "Failed to get monitor for the window");

        const Real32 Width  = static_cast<Real32>(Window.GetWidth());
        const Real32 Height = static_cast<Real32>(Window.GetHeight());

        // Create the ImGui context and configure basic IO flags (keyboard navigation, docking, renderer features).
        ImGui::CreateContext();

        Ref<ImGuiIO> IO = ImGui::GetIO();
        IO.ConfigFlags            |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable;
        IO.DisplaySize             = ImVec2(Width, Height);
        IO.DisplayFramebufferScale = ImVec2(Monitor->GetScale(), Monitor->GetScale());
        IO.BackendPlatformUserData = &* Platform;

        // TODO: Clipboard functionality

        // Apply the default dark theme styling.
        ImGui::StyleColorsDark();

        // Initialize the renderer backend for ImGui.
        mRenderer.Initialize(Host, Space);

        // Register input event callbacks.
        ConstRetainer<ZyInput::Service> Input = Host.GetService<ZyInput::Service>();

        Input->OnKeyDown.AddFunction<&ImGuiSystem::OnKeyDown>();
        Input->OnKeyUp.AddFunction<&ImGuiSystem::OnKeyUp>();
        Input->OnKeyType.AddFunction<&ImGuiSystem::OnKeyType>();
        Input->OnMouseDown.AddFunction<&ImGuiSystem::OnMouseDown>();
        Input->OnMouseUp.AddFunction<&ImGuiSystem::OnMouseUp>();
        Input->OnMouseMove.AddFunction<&ImGuiSystem::OnMouseMove>();
        Input->OnMouseScroll.AddFunction<&ImGuiSystem::OnMouseScroll>();
        Input->OnWindowFocus.AddFunction<&ImGuiSystem::OnWindowFocus>();
        Input->OnWindowResize.AddFunction<&ImGuiSystem::OnWindowResize>();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiSystem::Teardown(Ref<ZyEngine::Subsystem::Host> Host)
    {
        // Dispose of the renderer backend.
        mRenderer.Dispose();

        // Releases all input event callbacks.
        ConstRetainer<ZyInput::Service> Input = Host.GetService<ZyInput::Service>();

        Input->OnKeyDown.RemoveFunction<&ImGuiSystem::OnKeyDown>();
        Input->OnKeyUp.RemoveFunction<&ImGuiSystem::OnKeyUp>();
        Input->OnKeyType.RemoveFunction<&ImGuiSystem::OnKeyType>();
        Input->OnMouseDown.RemoveFunction<&ImGuiSystem::OnMouseDown>();
        Input->OnMouseUp.RemoveFunction<&ImGuiSystem::OnMouseUp>();
        Input->OnMouseMove.RemoveFunction<&ImGuiSystem::OnMouseMove>();
        Input->OnMouseScroll.RemoveFunction<&ImGuiSystem::OnMouseScroll>();
        Input->OnWindowFocus.RemoveFunction<&ImGuiSystem::OnWindowFocus>();
        Input->OnWindowResize.RemoveFunction<&ImGuiSystem::OnWindowResize>();

        // Destroy the context last, since disposing the renderer above still walks the platform texture list.
        ImGui::DestroyContext();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiSystem::Begin(Real64 Time)
    {
        ImGui::GetIO().DeltaTime = static_cast<Real32>(Time);
        ImGui::NewFrame();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiSystem::End()
    {
        ImGui::Render();

        if (const ConstPtr<ImDrawData> Commands = ImGui::GetDrawData(); Commands && Commands->TotalVtxCount > 0)
        {
            mRenderer.Submit(* Commands);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnKeyType(Text Text)
    {
        StrIterateUTF8(Text, [](UInt32 Codepoint)
        {
            ImGui::GetIO().AddInputCharacter(Codepoint);
        });
        return ImGui::GetIO().WantCaptureKeyboard;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnKeyUp(ZyInput::Key Key)
    {
        if (const auto ImModifier = GetKeyModifier(Key); ImModifier != ImGuiMod_None)
        {
            ImGui::GetIO().AddKeyEvent(ImModifier, false);
        }
        if (const auto ImKey = GetKey(Key); ImKey != ImGuiKey_None)
        {
            ImGui::GetIO().AddKeyEvent(ImKey, false);
        }
        return ImGui::GetIO().WantCaptureKeyboard;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnKeyDown(ZyInput::Key Key)
    {
        if (const auto ImModifier = GetKeyModifier(Key); ImModifier != ImGuiMod_None)
        {
            ImGui::GetIO().AddKeyEvent(ImModifier, true);
        }
        if (const auto ImKey = GetKey(Key); ImKey != ImGuiKey_None)
        {
            ImGui::GetIO().AddKeyEvent(ImKey, true);
        }
        return ImGui::GetIO().WantCaptureKeyboard;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnMouseMove(Real32 X, Real32 Y, Real32 DeltaX, Real32 DeltaY)
    {
        ImGui::GetIO().AddMousePosEvent(X, Y);
        return ImGui::GetIO().WantCaptureMouse;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnMouseScroll(Real32 DeltaX, Real32 DeltaY)
    {
        ImGui::GetIO().AddMouseWheelEvent(DeltaX, DeltaY);
        return ImGui::GetIO().WantCaptureMouse;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnMouseUp(ZyInput::Button Button)
    {
        if (const auto ImButton = GetButton(Button); ImButton >= 0)
        {
            ImGui::GetIO().AddMouseButtonEvent(ImButton, false);
        }
        return ImGui::GetIO().WantCaptureMouse;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnMouseDown(ZyInput::Button Button)
    {
        if (const auto ImButton = GetButton(Button); ImButton >= 0)
        {
            ImGui::GetIO().AddMouseButtonEvent(ImButton, true);
        }
        return ImGui::GetIO().WantCaptureMouse;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnWindowFocus(Bool Focused)
    {
        ImGui::GetIO().AddFocusEvent(Focused);
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool ImGuiSystem::OnWindowResize(UInt32 Width, UInt32 Height)
    {
        ImGui::GetIO().DisplaySize = ImVec2(static_cast<Real32>(Width), static_cast<Real32>(Height));
        return false;
    }
}