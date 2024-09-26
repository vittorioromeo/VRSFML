layout(location = 2) uniform sampler2D sf_u_texture;
layout(location = 3) uniform float edge_threshold;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    const float offset = 1.0 / 512.0;

    vec2 offx = vec2(offset, 0.0);
    vec2 offy = vec2(0.0, offset);

    // clang-format off
    vec4 hEdge = texture(sf_u_texture, sf_v_texCoord.xy - offy)        * -2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offy)        *  2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx - offy) * -1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx + offy) *  1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx - offy) * -1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx + offy) *  1.0;
    // clang-format on

    // clang-format off
    vec4 vEdge = texture(sf_u_texture, sf_v_texCoord.xy - offx)        *  2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx)        * -2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx - offy) *  1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx + offy) * -1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx - offy) *  1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx + offy) * -1.0;
    // clang-format on

    vec3 result = sqrt(hEdge.rgb * hEdge.rgb + vEdge.rgb * vEdge.rgb);

    float edge = length(result);

    vec4 pixel = sf_v_color * texture(sf_u_texture, sf_v_texCoord.xy);

    if (edge > (edge_threshold * 8.0))
        pixel.rgb = vec3(0.0, 0.0, 0.0);
    else
        pixel.a = edge_threshold;

    sf_fragColor = pixel;
}
