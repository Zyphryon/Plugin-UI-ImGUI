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
#include <Zyphryon.Graphic/Technique.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Plugin
{
    /// \brief Handles rendering of ImGui draw data using the engine graphics service.
    class ImGuiRenderer final
    {
    public:

        /// \brief Default constructor.
        ImGuiRenderer();

        /// \brief Initializes the ImGui renderer with the specified host.
        ///
        /// \param Host The engine subsystem host used to access graphics services.
        void Initialize(Ref<Engine::Subsystem::Host> Host);

        /// \brief Disposes of the renderer and releases all associated resources.
        void Dispose();

        /// \brief Submits ImGui draw commands for rendering.
        ///
        /// \param Commands The ImGui draw data containing all commands to be rendered.
        void Submit(ConstRef<ImDrawData> Commands);

    private:

        /// \brief Enumerates the technique variants the renderer draws with, one per sampled texture layout.
        enum class Kind : UInt8
        {
            Flat,        ///< Samples a flat texture, the layout every ImGui-owned texture is created with.
            Layered,     ///< Samples the slice the draw's instance block names within an array texture.
        };

        /// \brief Defines a type alias for a collection of rendering techniques.
        using Techniques = Array<Retainer<Graphic::Technique>, Enum::Count<Kind>()>;

        /// \brief Creates a texture resource for ImGui rendering.
        ///
        /// \param Texture The texture data to be created.
        void CreateTexture(Ptr<ImTextureData> Texture);

        /// \brief Deletes a texture resource previously created for ImGui rendering.
        ///
        /// \param Texture The texture data to be deleted.
        void DeleteTexture(Ptr<ImTextureData> Texture);

        /// \brief Updates an existing texture resource with new data.
        ///
        /// \param Texture The texture data to be updated.
        void UpdateTexture(Ptr<ImTextureData> Texture);

    public:

        /// \brief Builds the ImGui texture identifier that samples a texture as a plain 2D image.
        ///
        /// \param Handle The 2D texture object to sample.
        /// \return The identifier accepted by ImGui's image and draw list functions.
        ZY_INLINE static ImTextureID GetTextureID(Graphic::Object Handle)
        {
            return Handle;
        }

        /// \brief Builds the ImGui texture identifier that samples one slice of an array texture.
        ///
        /// \param Handle The array texture object to sample.
        /// \param Slice  The zero-based slice within the array.
        /// \return The identifier accepted by ImGui's image and draw list functions.
        ZY_INLINE static ImTextureID GetTextureID(Graphic::Object Handle, UInt16 Slice)
        {
            return static_cast<ImTextureID>(Handle) | ((static_cast<ImTextureID>(Slice) + 1) << 32);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Service> mGraphics;
        Techniques                 mTechniques;
        Graphic::Object            mSampler;
    };
}