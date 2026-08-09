// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Uniforms
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

cbuffer cb_Global : register(b0)
{
    float4x4 u_Camera;
};

#ifdef    ENABLE_TEXTURE_ARRAY

cbuffer cb_Instance : register(b3)
{
    uint u_Slice;
};

#endif // ENABLE_TEXTURE_ARRAY

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Attributes
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct vs_Input
{
    float2 Position  : SLOT0;
    float2 Texture   : SLOT1;
    float4 Color     : SLOT2;
};

struct ps_Input
{
    float4 Position : SV_POSITION;
    float2 Texture  : TEXCOORD0;
    float4 Color    : COLOR0;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef   VERTEX_SHADER

ps_Input main(vs_Input Input)
{
    ps_Input Result;

    Result.Position = mul(u_Camera, float4(Input.Position, 0.0f, 1.0f));
    Result.Texture  = Input.Texture;
    Result.Color    = Input.Color;

    return Result;
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

#ifdef    ENABLE_TEXTURE_ARRAY
Texture2DArray t_Albedo  : register(t0);
#else
Texture2D      t_Albedo  : register(t0);
#endif // ENABLE_TEXTURE_ARRAY

SamplerState   s_Albedo  : register(s0);

#ifdef    ENABLE_SRGB_TARGET

float3 ToLinear(float3 Color)
{
    const float3 Lower = Color / 12.92;
    const float3 Upper = pow((Color + 0.055) / 1.055, 2.4);

    return lerp(Lower, Upper, step(0.04045, Color));
}

#endif // ENABLE_SRGB_TARGET

float4 main(ps_Input Input) : SV_Target
{
#ifdef    ENABLE_TEXTURE_ARRAY
    float4 Texel = t_Albedo.Sample(s_Albedo, float3(Input.Texture, float(u_Slice)));
#else
    float4 Texel = t_Albedo.Sample(s_Albedo, Input.Texture);
#endif // ENABLE_TEXTURE_ARRAY

#ifdef    ENABLE_SRGB_TARGET
    const float4 Color = float4(ToLinear(Input.Color.rgb), Input.Color.a);
#else
    const float4 Color = Input.Color;
#endif // ENABLE_SRGB_TARGET

    return Color * Texel;
}

#endif // FRAGMENT_SHADER