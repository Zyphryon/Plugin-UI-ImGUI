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

    void ImGuiRenderer::Initialize(Ref<Engine::Subsystem::Host> Host)
    {
        mGraphics = Host.GetService<Graphic::Service>();

        ConstRetainer<Content::Service> Content = Host.GetService<Content::Service>();

        for (const Kind Type : Enum::GetValues<Kind>())
        {
            Str Path = Str::Print<"Embedded://Technique/UI/ImGui-{0}.vfx">(Enum::GetName(Type));

            mTechniques[Enum::Cast(Type)] = Content->Load<Graphic::Technique>(Move(Path));
        }

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
        // Abort drawing if either technique has not finished loading or compiling.
        for (ConstRetainer<Graphic::Technique> Technique : mTechniques)
        {
            if (!Technique->HasCompleted())
            {
                return;
            }
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

        Graphic::Transient<Matrix4x4> UboSlice = mGraphics->AllocateInFlightUniforms<Matrix4x4>(1);
        UboSlice[0] = Matrix4x4::CreateOrthographic(
                Commands.DisplayPos.x,
                Commands.DisplayPos.x + Commands.DisplaySize.x,
                Commands.DisplayPos.y + Commands.DisplaySize.y,
                Commands.DisplayPos.y,
                -1.0f,
                +1.0f);

        // Captured up front, since the per-draw blocks allocated below share the same arena.
        const Graphic::Stream Camera = UboSlice.GetStream();

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

                const Graphic::Object Texture = static_cast<Graphic::Object>(Command.GetTexID());
                const UInt32          Slice   = static_cast<UInt32>(Command.GetTexID() >> 32);

                // Only array draws carry a block; the 2D technique never declares one to read.
                Graphic::Stream Instance;

                if (Slice > 0)
                {
                    Graphic::Transient<UInt32> Block = mGraphics->AllocateInFlightUniforms<UInt32>(1);
                    Block[0] = Slice - 1;

                    Instance = Block.GetStream();
                }

                Ref<Graphic::Command> GfxCommand = mGraphics->AllocateInFlightCommand();

                // Devices without base-vertex support ignore vertex base offset.
                const UInt32    Base     = VtxOffset + Command.VtxOffset;
                Graphic::Stream Vertices = VtxSlice.GetStream();

                if (!SupportsVertexBaseOffset)
                {
                    Vertices.Offset += Base * sizeof(ImDrawVert);
                }

                ConstRetainer<Graphic::Technique> Technique =
                    Slice > 0 ? mTechniques[Enum::Cast(Kind::Texture2DArray)]
                              : mTechniques[Enum::Cast(Kind::Texture2D)];

                GfxCommand.Scissor = Graphic::Scissor(
                    static_cast<UInt16>(MinX),
                    static_cast<UInt16>(MinY),
                    static_cast<UInt16>(MaxX - MinX),
                    static_cast<UInt16>(MaxY - MinY));
                GfxCommand.Pipeline = Technique->GetHandle();
                GfxCommand.Vertices.Append(Vertices);
                GfxCommand.Indices = IdxSlice.GetStream();
                GfxCommand.Uniforms[Enum::Cast(Graphic::Frequency::Frame)]    = Camera;
                GfxCommand.Uniforms[Enum::Cast(Graphic::Frequency::Instance)] = Instance;
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