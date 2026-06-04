#include "default_uniforms.glsl"

layout(location = 4) uniform float edge_threshold;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    const float offset = 1.0 / 512.0;

    vec2 offx = vec2(offset, 0.0);
    vec2 offy = vec2(0.0, offset);

    // clang-format off
    vec4 hEdge = texture(za_u_texture, za_v_texCoord - offy)        * -2.0 +
                 texture(za_u_texture, za_v_texCoord + offy)        *  2.0 +
                 texture(za_u_texture, za_v_texCoord - offx - offy) * -1.0 +
                 texture(za_u_texture, za_v_texCoord - offx + offy) *  1.0 +
                 texture(za_u_texture, za_v_texCoord + offx - offy) * -1.0 +
                 texture(za_u_texture, za_v_texCoord + offx + offy) *  1.0;
    // clang-format on

    // clang-format off
    vec4 vEdge = texture(za_u_texture, za_v_texCoord - offx)        *  2.0 +
                 texture(za_u_texture, za_v_texCoord + offx)        * -2.0 +
                 texture(za_u_texture, za_v_texCoord - offx - offy) *  1.0 +
                 texture(za_u_texture, za_v_texCoord - offx + offy) * -1.0 +
                 texture(za_u_texture, za_v_texCoord + offx - offy) *  1.0 +
                 texture(za_u_texture, za_v_texCoord + offx + offy) * -1.0;
    // clang-format on

    vec3 result = sqrt(hEdge.rgb * hEdge.rgb + vEdge.rgb * vEdge.rgb);

    float edge = length(result);

    vec4 pixel = za_v_color * texture(za_u_texture, za_v_texCoord);

    if (edge > (edge_threshold * 8.0))
        pixel.rgb = vec3(0.0, 0.0, 0.0);
    else
        pixel.a = edge_threshold;

    za_fragColor = pixel;
}
