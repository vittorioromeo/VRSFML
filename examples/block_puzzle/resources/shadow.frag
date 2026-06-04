layout(location = 0) out vec4 za_fragColor;
layout(location = 2) uniform sampler2D za_u_texture;
uniform vec4 u_shadowColor; // e.g., (0.0, 0.0, 0.0, 0.5)

in vec4 za_v_color;    // Vertex color (can be ignored or used to tint shadow)
in vec2 za_v_texCoord; // Normalized tex coord for the quad

void main()
{
    float shadowAlpha = texture(za_u_texture, za_v_texCoord).a;
    za_fragColor      = vec4(u_shadowColor.rgb, u_shadowColor.a * shadowAlpha);
}
