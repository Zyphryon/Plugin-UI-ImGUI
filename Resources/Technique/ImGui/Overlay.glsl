// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Uniforms
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

layout(std140, binding = 0) uniform cb_Global
{
    mat4 u_Camera;
};

#ifdef    ENABLE_TEXTURE_ARRAY

layout(std140, binding = 3) uniform cb_Instance
{
    uint u_Slice;
};

#endif // ENABLE_TEXTURE_ARRAY

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef   VERTEX_SHADER

in vec2 a_Position;
in vec2 a_Texture;
in vec4 a_Color;

out vec2 v_Texture;
out vec4 v_Color;

void main()
{
    gl_Position = u_Camera * vec4(a_Position, 0.0, 1.0);
    v_Texture    = a_Texture;
    v_Color      = a_Color;
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 out_Color;

#ifdef    ENABLE_TEXTURE_ARRAY
uniform sampler2DArray t_Albedo;
#else
uniform sampler2D      t_Albedo;
#endif // ENABLE_TEXTURE_ARRAY

in vec2 v_Texture;
in vec4 v_Color;

#ifdef    ENABLE_SRGB_TARGET

vec3 ToLinear(vec3 Color)
{
    vec3 Lower = Color / 12.92;
    vec3 Upper = pow((Color + 0.055) / 1.055, vec3(2.4));

    return mix(Lower, Upper, step(vec3(0.04045), Color));
}

#endif // ENABLE_SRGB_TARGET

void main()
{
#ifdef    ENABLE_SRGB_TARGET
    vec4 Color = vec4(ToLinear(v_Color.rgb), v_Color.a);
#else
    vec4 Color = v_Color;
#endif // ENABLE_SRGB_TARGET

#ifdef    ENABLE_TEXTURE_ARRAY
    out_Color = Color * texture(t_Albedo, vec3(v_Texture, float(u_Slice)));
#else
    out_Color = Color * texture(t_Albedo, v_Texture);
#endif // ENABLE_TEXTURE_ARRAY
}

#endif // FRAGMENT_SHADER