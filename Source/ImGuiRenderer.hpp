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
    /// \brief Names how the target ImGui draws into stores colour.
    enum class Colorspace : UInt8
    {
        Linear,  ///< Stored as written, so a colour reaches the display unchanged.
        sRGB,    ///< Encoded on write, so a colour is decoded first to survive the round trip.
    };

    /// \brief Handles rendering of ImGui draw data using the engine graphics service.
    class ImGuiRenderer final
    {
    public:

        /// \brief Default constructor.
        ImGuiRenderer();

        /// \brief Initializes the ImGui renderer with the specified host.
        ///
        /// \param Host  The engine subsystem host used to access graphics services.
        /// \param Space The colour space the target starts at, which \ref SetColorspace changes afterwards.
        void Initialize(Ref<Engine::Subsystem::Host> Host, Colorspace Space = Colorspace::Linear);

        /// \brief Disposes of the renderer and releases all associated resources.
        void Dispose();

        /// \brief Sets the colour space the target stores, which every draw is encoded for from here on.
        ///
        /// \note Every colour space is compiled up front, so switching one never waits on a shader compile.
        ///
        /// \param Space The colour space of the target.
        ZY_INLINE void SetColorspace(Colorspace Space)
        {
            if (mColorspace != Space)
            {
                mColorspace = Space;

                if (mTechnique && mTechnique->HasCompleted())
                {
                    ObtainPipelines();
                }
            }
        }

        /// \brief Gets the colour space the renderer encodes its draws for.
        ///
        /// \return The colour space of the target.
        ZY_INLINE Colorspace GetColorspace() const
        {
            return mColorspace;
        }

        /// \brief Submits ImGui draw commands for rendering.
        ///
        /// \param Commands The ImGui draw data containing all commands to be rendered.
        void Submit(ConstRef<ImDrawData> Commands);

    private:

        /// \brief Enumerates the technique variants the renderer draws with, one per sampled texture layout.
        enum class Kind : UInt8
        {
            Flat,        ///< Samples a flat texture, the layout every ImGui-owned texture is created with.
            Layered,     ///< Samples the slice the draw's texture coordinates name within an array texture.
        };

        /// \brief Defines a type alias for the pipeline of each kind, compiled for the current colour space.
        using Pipelines = Array<Graphic::Object, Enum::Count<Kind>()>;

        /// \brief Resolves the technique variant each kind draws with under the current colour space.
        void ObtainPipelines();

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

        /// \brief Marks an identifier as naming an array texture, so the draw samples the layered technique.
        static constexpr ImTextureID kLayeredFlag = static_cast<ImTextureID>(1) << 63;

        /// \brief Width of the horizontal coordinate band a single array slice occupies.
        static constexpr Real32      kSliceStride = 2.0f;

        /// \brief Builds the ImGui texture identifier that samples a texture as a plain 2D image.
        ///
        /// \param Handle The 2D texture object to sample.
        /// \return The identifier accepted by ImGui's image and draw list functions.
        ZY_INLINE static ImTextureID GetTextureID(Graphic::Object Handle)
        {
            return Handle;
        }

        /// \brief Builds the ImGui texture identifier that samples an array texture.
        ///
        /// The identifier names the texture alone, so every slice of it shares one identifier and ImGui keeps
        /// merging consecutive draws into a single command. The slice rides along the texture coordinates
        /// instead, which \ref GetLayeredTextureUV encodes.
        ///
        /// \param Handle The array texture object to sample.
        /// \return The identifier accepted by ImGui's image and draw list functions.
        ZY_INLINE static ImTextureID GetLayeredTextureID(Graphic::Object Handle)
        {
            return static_cast<ImTextureID>(Handle) | kLayeredFlag;
        }

        /// \brief Encodes the slice a vertex samples into the texture coordinates it carries.
        ///
        /// it holds sub-texel accuracy for a 4096-wide texture up to roughly slice 255, and degrades gradually beyond that.
        ///
        /// \param Slice       The zero-based slice within the array.
        /// \param Coordinates The texture coordinates within that slice, normally within [0, 1].
        /// \return The encoded coordinates accepted by ImGui's image and draw list functions.
        ZY_INLINE static ImVec2 GetLayeredTextureUV(UInt16 Slice, ConstRef<ImVec2> Coordinates)
        {
            return ImVec2(Coordinates.x + static_cast<Real32>(Slice) * kSliceStride, Coordinates.y);
        }

        /// \brief Draws one slice of an array texture, pairing the identifier with the coordinates it expects.
        ///
        /// \param Handle The array texture object to sample.
        /// \param Slice  The zero-based slice within the array.
        /// \param Size   The size of the image in screen pixels.
        /// \param Min    The upper left texture coordinates within the slice.
        /// \param Max    The lower right texture coordinates within the slice.
        ZY_INLINE static void DrawLayeredImage(
            Graphic::Object  Handle,
            UInt16           Slice,
            ConstRef<ImVec2> Size,
            ConstRef<ImVec2> Min = ImVec2(0.0f, 0.0f),
            ConstRef<ImVec2> Max = ImVec2(1.0f, 1.0f))
        {
            ImGui::Image(GetLayeredTextureID(Handle),
                         Size,
                         GetLayeredTextureUV(Slice, Min),
                         GetLayeredTextureUV(Slice, Max));
        }
        
    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Service>   mGraphics;
        Retainer<Graphic::Technique> mTechnique;
        Pipelines                    mPipelines;
        Graphic::Object              mSampler;
        Colorspace                   mColorspace;
    };
}