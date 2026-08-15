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

#include "ImGuiRenderer.hpp"
#include <Zyphryon.Math/Matrix4x4.hpp>
#include <Zyphryon.Content/Service.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Plugin
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ImGuiRenderer::ImGuiRenderer()
        : mColorspace { Colorspace::Linear },
          mPipelines  { },
          mSampler    { }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::Initialize(Ref<Engine::Subsystem::Host> Host, Colorspace Space)
    {
        mGraphics   = Host.GetService<Graphic::Service>();
        mColorspace = Space;

        ConstRetainer<Content::Service> Content = Host.GetService<Content::Service>();

        // One technique declares every kind and colour space as a feature, and preloads each combination of them.
        mTechnique = Content->Load<Graphic::Technique>("Embedded://Technique/ImGui/Overlay.vfx");

        // Loading only queues the request, so resolve the variants once the technique has uploaded them all.
        Content->Subscribe(mTechnique->GetKey(), [this](Ref<Content::Resource>)
        {
            ObtainPipelines();
        });

        mSampler = mGraphics->ObtainSampler(Graphic::Sampler {
            .Filter = Graphic::TextureFilter::Linear
        });

        ConstRef<Graphic::Capabilities> Capabilities = mGraphics->GetDescription().Capabilities;

        Ref<ImGuiIO> IO = ImGui::GetIO();
        IO.BackendRendererName = "Zyphryon";
        IO.BackendFlags = SetBit(IO.BackendFlags, ImGuiBackendFlags_RendererHasTextures);
        IO.BackendFlags = SetBit(IO.BackendFlags, ImGuiBackendFlags_RendererHasVtxOffset);

        Ref<ImGuiPlatformIO> PlatformIO = ImGui::GetPlatformIO();
        PlatformIO.Renderer_TextureMaxWidth  = static_cast<SInt32>(Capabilities.MaxTextureDimension);
        PlatformIO.Renderer_TextureMaxHeight = PlatformIO.Renderer_TextureMaxWidth;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::Dispose()
    {
        for (const Ptr<ImTextureData> Texture : ImGui::GetPlatformIO().Textures)
        {
            if (Texture->RefCount == 1)
            {
                DeleteTexture(Texture);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::Submit(ConstRef<ImDrawData> Commands)
    {
        // Abort drawing if the technique has not finished loading or compiling.
        if (!mTechnique->HasCompleted())
        {
            return;
        }

        // Handle all pending texture operations.
        if (Commands.Textures != nullptr)
        {
            for (const Ptr<ImTextureData> Texture : * Commands.Textures)
            {
                switch (Texture->Status)
                {
                case ImTextureStatus_WantCreate:
                    CreateTexture(Texture);
                    break;
                case ImTextureStatus_WantUpdates:
                    UpdateTexture(Texture);
                    break;
                case ImTextureStatus_WantDestroy:
                    DeleteTexture(Texture);
                    break;
                default:
                    break;
                }
            }
        }

        Graphic::Transient<ImDrawVert> VtxSlice = mGraphics->AllocateInFlightVertices<ImDrawVert>(Commands.TotalVtxCount);
        Graphic::Transient<ImDrawIdx>  IdxSlice = mGraphics->AllocateInFlightIndices<ImDrawIdx>(Commands.TotalIdxCount);

        const Matrix4x4 Projection = Matrix4x4::CreateOrthographic(
                Commands.DisplayPos.x,
                Commands.DisplayPos.x + Commands.DisplaySize.x,
                Commands.DisplayPos.y + Commands.DisplaySize.y,
                Commands.DisplayPos.y,
                -1.0f,
                +1.0f);

        // Every draw reads the same camera block, so it is written once and its stream handed to each.
        const Graphic::Stream Camera = mGraphics->AllocateInFlightUniforms<Matrix4x4>(ConstSpan<Matrix4x4>(Projection));

        UInt32 VtxOffset = 0;
        UInt32 IdxOffset = 0;

        const Bool SupportsVertexBaseOffset = mGraphics->GetDescription().Capabilities.SupportsBaseVertex;

        for (const ConstPtr<ImDrawList> CommandList : Commands.CmdLists)
        {
            VtxSlice.Copy(ConstSpan(CommandList->VtxBuffer.Data, CommandList->VtxBuffer.Size), VtxOffset);
            IdxSlice.Copy(ConstSpan(CommandList->IdxBuffer.Data, CommandList->IdxBuffer.Size), IdxOffset);

            for (SInt32 Element = 0; Element < CommandList->CmdBuffer.Size; ++Element)
            {
                ConstRef<ImDrawCmd> Command = CommandList->CmdBuffer[Element];

                if (Command.UserCallback)
                {
                    Command.UserCallback(CommandList, AddressOf(Command));
                    continue;
                }

                const Real32 MinX = Max(Command.ClipRect.x - Commands.DisplayPos.x, 0.0f);
                const Real32 MinY = Max(Command.ClipRect.y - Commands.DisplayPos.y, 0.0f);
                const Real32 MaxX = Min(Command.ClipRect.z - Commands.DisplayPos.x, Commands.DisplaySize.x);
                const Real32 MaxY = Min(Command.ClipRect.w - Commands.DisplayPos.y, Commands.DisplaySize.y);

                if (MaxX <= MinX || MaxY <= MinY)
                {
                    continue;
                }

                const ImTextureID     Identifier = Command.GetTexID();
                const Graphic::Object Texture    = static_cast<Graphic::Object>(Identifier);
                const Bool            Layered    = (Identifier & kLayeredFlag) != 0;

                Ref<Graphic::Command> GfxCommand = mGraphics->AllocateInFlightCommand();

                // Devices without base-vertex support ignore vertex base offset.
                const UInt32    Base     = VtxOffset + Command.VtxOffset;
                Graphic::Stream Vertices = VtxSlice.GetStream();

                if (!SupportsVertexBaseOffset)
                {
                    Vertices.Offset += Base * sizeof(ImDrawVert);
                }

                GfxCommand.Scissor = Graphic::Scissor(
                    static_cast<UInt16>(MinX),
                    static_cast<UInt16>(MinY),
                    static_cast<UInt16>(MaxX - MinX),
                    static_cast<UInt16>(MaxY - MinY));
                GfxCommand.Pipeline = mPipelines[Enum::Cast(Layered ? Kind::Layered : Kind::Flat)];
                GfxCommand.Vertices.Append(Vertices);
                GfxCommand.Indices = IdxSlice.GetStream();
                GfxCommand.Uniforms[Enum::Cast(Graphic::Frequency::Frame)] = Camera;
                GfxCommand.Textures.Append(Texture);
                GfxCommand.Samplers.Append(mSampler);

                GfxCommand.Parameters =
                {
                    .Count     = Command.ElemCount,
                    .Base      = SupportsVertexBaseOffset ? static_cast<SInt32>(Base) : 0,
                    .Offset    = Command.IdxOffset + IdxOffset,
                    .Instances = 1
                };
            }

            VtxOffset += CommandList->VtxBuffer.Size;
            IdxOffset += CommandList->IdxBuffer.Size;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::ObtainPipelines()
    {
        const Graphic::Technique::Key Layered = mTechnique->ResolveByName("Layered");
        const Graphic::Technique::Key Encoded = (mColorspace == Colorspace::sRGB) ? mTechnique->ResolveByName("sRGB") : 0;

        mPipelines[Enum::Cast(Kind::Flat)]    = mTechnique->Obtain(* mGraphics, Encoded);
        mPipelines[Enum::Cast(Kind::Layered)] = mTechnique->Obtain(* mGraphics, Encoded | Layered);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::CreateTexture(Ptr<ImTextureData> Texture)
    {
        const UInt32           Size = Texture->Height * Texture->Width * Texture->BytesPerPixel;
        Graphic::TextureFormat Format;

        switch (Texture->Format)
        {
        case ImTextureFormat_RGBA32:
            Format = Graphic::TextureFormat::RGBA8UIntNorm;
            break;
        case ImTextureFormat_Alpha8:
            Format = Graphic::TextureFormat::R8UIntNorm;
            break;
        default:
            ZY_ASSERT(false, "Unsupported ImGui texture format");
            return;
        }

        const Graphic::Object Handle = mGraphics->CreateTexture(
            Graphic::TextureLayout::Texture2D,
            Format,
            Graphic::Storage::Stream,
            Graphic::Usage::Sample,
            Texture->Width,
            Texture->Height,
            1,
            1,
            Graphic::Multisample::X1,
            Blob::Borrow<Byte>(static_cast<ConstPtr<Byte>>(Texture->GetPixels()), Size));
        Texture->SetTexID(Handle);
        Texture->SetStatus(ImTextureStatus_OK);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::DeleteTexture(Ptr<ImTextureData> Texture)
    {
        if (const Graphic::Object Handle = Texture->GetTexID(); Handle)
        {
            mGraphics->DeleteTexture(Handle);

            // Invalidate texture ID.
            Texture->SetTexID(ImTextureID_Invalid);
        }
        Texture->SetStatus(ImTextureStatus_Destroyed);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::UpdateTexture(Ptr<ImTextureData> Texture)
    {
        const UInt32 Pitch = Texture->GetPitch();

        for (const auto [X, Y, W, H] : Texture->Updates)
        {
            const UInt32 Size = (H - 1) * Pitch + W * Texture->BytesPerPixel;

            mGraphics->UpdateTexture(
                Texture->GetTexID(),
                0,
                0,
                X,
                Y,
                W,
                H,
                Pitch,
                Blob::Borrow<Byte>(static_cast<ConstPtr<Byte>>(Texture->GetPixelsAt(X, Y)), Size));
        }
        Texture->SetStatus(ImTextureStatus_OK);
    }
}