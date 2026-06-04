#include "default_uniforms.glsl"

uniform float blur_radius;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    vec2 offx = vec2(blur_radius, 0.0);
    vec2 offy = vec2(0.0, blur_radius);

    // clang-format off
    vec4 pixel = texture(za_u_texture, za_v_texCoord)               * 4.0 +
                 texture(za_u_texture, za_v_texCoord - offx)        * 2.0 +
                 texture(za_u_texture, za_v_texCoord + offx)        * 2.0 +
                 texture(za_u_texture, za_v_texCoord - offy)        * 2.0 +
                 texture(za_u_texture, za_v_texCoord + offy)        * 2.0 +
                 texture(za_u_texture, za_v_texCoord - offx - offy) * 1.0 +
                 texture(za_u_texture, za_v_texCoord - offx + offy) * 1.0 +
                 texture(za_u_texture, za_v_texCoord + offx - offy) * 1.0 +
                 texture(za_u_texture, za_v_texCoord + offx + offy) * 1.0;
    // clang-format on

    za_fragColor = za_v_color * (pixel / 16.0);
}
