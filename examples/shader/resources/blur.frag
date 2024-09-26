layout(location = 2) uniform sampler2D sf_u_texture;
uniform float     blur_radius;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    vec2 offx = vec2(blur_radius, 0.0);
    vec2 offy = vec2(0.0, blur_radius);

    // clang-format off
    vec4 pixel = texture(sf_u_texture, sf_v_texCoord.xy)               * 4.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx)        * 2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx)        * 2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offy)        * 2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offy)        * 2.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx - offy) * 1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy - offx + offy) * 1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx - offy) * 1.0 +
                 texture(sf_u_texture, sf_v_texCoord.xy + offx + offy) * 1.0;
    // clang-format on

    sf_fragColor = sf_v_color * (pixel / 16.0);
}
