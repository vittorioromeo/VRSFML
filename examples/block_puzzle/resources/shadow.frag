layout(location = 0) out vec4 sf_fragColor;
layout(location = 1) uniform sampler2D sf_u_texture;
uniform vec4 u_shadowColor; // e.g., (0.0, 0.0, 0.0, 0.5)

in vec4 sf_v_color;    // Vertex color (can be ignored or used to tint shadow)
in vec2 sf_v_texCoord; // Normalized tex coord for the quad

void main()
{
    float shadowAlpha = texture(sf_u_texture, sf_v_texCoord).a;
    sf_fragColor      = vec4(u_shadowColor.rgb, u_shadowColor.a * shadowAlpha);
}
