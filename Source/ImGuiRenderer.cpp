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

    void ImGuiRenderer::Initialize(Ref<ZyEngine::Subsystem::Host> Host, Colorspace Space)
    {
        mGraphics   = Host.GetService<ZyGraphic::Service>();
        mColorspace = Space;

        ConstRetainer<ZyContent::Service> Content = Host.GetService<ZyContent::Service>();

        // One technique declares every kind and colour space as a feature, and preloads each combination of them.
        mTechnique = Content->Load<ZyGraphic::Technique>("Embedded://Technique/ImGui/Overlay.vfx");

        // Loading only queues the request, so resolve the variants once the technique has uploaded them all.
        Content->Subscribe(mTechnique->GetKey(), [this](Ref<ZyContent::Resource>)
        {
            ObtainPipelines();
        });

        mSampler = mGraphics->ObtainSampler(ZyGraphic::Sampler {
            .Filter = ZyGraphic::TextureFilter::Linear
        });

        ConstRef<ZyGraphic::Capabilities> Capabilities = mGraphics->GetDescription().Capabilities;

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
                    if (Texture->UnusedFrames >= ZyGraphic::kMaxFrames)
                    {
                        DeleteTexture(Texture);
                    }
                    break;
                default:
                    break;
                }
            }
        }

        ZyGraphic::Transient<ImDrawVert> VtxSlice = mGraphics->AllocateInFlightVertices<ImDrawVert>(Commands.TotalVtxCount);
        ZyGraphic::Transient<ImDrawIdx>  IdxSlice = mGraphics->AllocateInFlightIndices<ImDrawIdx>(Commands.TotalIdxCount);

        const Matrix4x4 Projection = Matrix4x4::CreateOrthographic(
                Commands.DisplayPos.x,
                Commands.DisplayPos.x + Commands.DisplaySize.x,
                Commands.DisplayPos.y + Commands.DisplaySize.y,
                Commands.DisplayPos.y,
                -1.0f,
                +1.0f);

        // Every draw reads the same camera block, so it is written once and its stream handed to each.
        const ZyGraphic::Stream Camera = mGraphics->AllocateInFlightUniforms<Matrix4x4>(ConstSpan<Matrix4x4>(Projection));

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
                const ZyGraphic::Object Texture    = static_cast<ZyGraphic::Object>(Identifier);
                const Bool            Layered    = (Identifier & kLayeredFlag) != 0;

                Ref<ZyGraphic::Command> GfxCommand = mGraphics->AllocateInFlightCommand();

                // Devices without base-vertex support ignore vertex base offset.
                const UInt32    Base     = VtxOffset + Command.VtxOffset;
                ZyGraphic::Stream Vertices = VtxSlice.GetStream();

                if (!SupportsVertexBaseOffset)
                {
                    Vertices.Offset += Base * sizeof(ImDrawVert);
                }

                GfxCommand.Scissor = ZyGraphic::Scissor(
                    static_cast<UInt16>(MinX),
                    static_cast<UInt16>(MinY),
                    static_cast<UInt16>(MaxX - MinX),
                    static_cast<UInt16>(MaxY - MinY));
                GfxCommand.Pipeline = mPipelines[ZyEnum::Cast(Layered ? Kind::Layered : Kind::Flat)];
                GfxCommand.Vertices.Append(Vertices);
                GfxCommand.Indices = IdxSlice.GetStream();
                GfxCommand.Uniforms[ZyEnum::Cast(ZyGraphic::Frequency::Frame)] = Camera;
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
        const ZyGraphic::Technique::Key Layered = mTechnique->ResolveByName("Layered");

        const Bool Decodes = (mColorspace == Colorspace::sRGB || mColorspace == Colorspace::sRGBUnmanaged);
        const Bool Encodes = (mColorspace == Colorspace::sRGBUnmanaged);

        const ZyGraphic::Technique::Key Decoded = Decodes ? mTechnique->ResolveByName("sRGB") : 0;
        const ZyGraphic::Technique::Key Encoded = Encodes ? mTechnique->ResolveByName("Encode") : 0;

        mPipelines[ZyEnum::Cast(Kind::Flat)]    = mTechnique->Obtain(* mGraphics, Decoded | Encoded);
        mPipelines[ZyEnum::Cast(Kind::Layered)] = mTechnique->Obtain(* mGraphics, Decoded | Encoded | Layered);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::CreateTexture(Ptr<ImTextureData> Texture)
    {
        const UInt32           Size = Texture->Height * Texture->Width * Texture->BytesPerPixel;
        ZyGraphic::TextureFormat Format;

        switch (Texture->Format)
        {
        case ImTextureFormat_RGBA32:
            Format = ZyGraphic::TextureFormat::RGBA8UIntNorm;
            break;
        case ImTextureFormat_Alpha8:
            Format = ZyGraphic::TextureFormat::R8UIntNorm;
            break;
        default:
            ZY_ASSERT(false, "Unsupported ImGui texture format");
            return;
        }

        Blob Pixels = Blob::Allocate<Byte>(Size);
        Pixels.Copy<Byte>(static_cast<ConstPtr<Byte>>(Texture->GetPixels()), Size);

        const ZyGraphic::Object Handle = mGraphics->CreateTexture(
            ZyGraphic::TextureLayout::Texture2D,
            Format,
            ZyGraphic::Storage::Stream,
            ZyGraphic::Usage::Sample,
            Texture->Width,
            Texture->Height,
            1,
            1,
            ZyGraphic::Multisample::X1,
            Move(Pixels));
        Texture->SetTexID(Handle);
        Texture->SetStatus(ImTextureStatus_OK);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void ImGuiRenderer::DeleteTexture(Ptr<ImTextureData> Texture)
    {
        if (const ZyGraphic::Object Handle = Texture->GetTexID(); Handle)
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

            Blob Pixels = Blob::Allocate<Byte>(Size);
            Pixels.Copy<Byte>(static_cast<ConstPtr<Byte>>(Texture->GetPixelsAt(X, Y)), Size);

            mGraphics->UpdateTexture(
                Texture->GetTexID(),
                0,
                0,
                X,
                Y,
                W,
                H,
                Pitch,
                Move(Pixels));
        }
        Texture->SetStatus(ImTextureStatus_OK);
    }
}