#include "default_uniforms.glsl"

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

out vec4 za_v_color;
out vec2 za_v_texCoord;

void main()
{
    vec3 pos = vec3(za_a_position, 1.0);

    gl_Position = vec4(dot(za_u_mvpRow0, pos), dot(za_u_mvpRow1, pos), 0.0, 1.0);
    za_v_texCoord = za_a_texCoord * za_u_invTextureSize;
    za_v_color    = za_a_color;
}
