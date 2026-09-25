// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// ImGui reads this file in place of its own imconfig.h (the build names it through IMGUI_USER_CONFIG), so every
// translation unit that includes imgui.h sees the same two vector types.

inline namespace ZyMath
{
    template<typename Type>
    class AnyVector2;

    template<typename Type>
    class AnyColor;
}

#define IM_VEC2_CLASS_EXTRA                                                                                     \
    template<typename Type>                                                                                     \
    constexpr ImVec2(const AnyVector2<Type> & Value)                                                            \
        : x { static_cast<float>(Value.GetX()) }, y { static_cast<float>(Value.GetY()) }                        \
    {                                                                                                           \
    }                                                                                                           \
    template<typename Type>                                                                                     \
    constexpr operator AnyVector2<Type>() const                                                                 \
    {                                                                                                           \
        return AnyVector2<Type>(static_cast<Type>(x), static_cast<Type>(y));                                    \
    }

#define IM_VEC4_CLASS_EXTRA                                                                                     \
    template<typename Type>                                                                                     \
    constexpr ImVec4(const AnyColor<Type> & Value)                                                              \
        : x { static_cast<float>(Value.GetRed())  }, y { static_cast<float>(Value.GetGreen()) },                \
          z { static_cast<float>(Value.GetBlue()) }, w { static_cast<float>(Value.GetAlpha()) }                 \
    {                                                                                                           \
        static_assert(Type(0.5) > Type(0), "Only a colour kept as reals converts to ImGui's");                  \
    }                                                                                                           \
    template<typename Type>                                                                                     \
    constexpr operator AnyColor<Type>() const                                                                   \
    {                                                                                                           \
        static_assert(Type(0.5) > Type(0), "Only a colour kept as reals converts to ImGui's");                  \
        return AnyColor<Type>(static_cast<Type>(x), static_cast<Type>(y),                                       \
                              static_cast<Type>(z), static_cast<Type>(w));                                      \
    }