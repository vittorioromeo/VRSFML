uniform float wave_phase;
uniform vec2  wave_amplitude;

layout(location = 0) uniform vec3 za_u_mvpRow0;
layout(location = 1) uniform vec3 za_u_mvpRow1;
layout(location = 3) uniform vec2 za_u_invTextureSize;

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

out vec4 za_v_color;
out vec2 za_v_texCoord;

void main()
{
    vec3 pos = vec3(za_a_position, 1.0);

    pos.x += cos(pos.y * 0.02 + wave_phase * 3.8) * wave_amplitude.x +
             sin(pos.y * 0.02 + wave_phase * 6.3) * wave_amplitude.x * 0.3;

    pos.y += sin(pos.x * 0.02 + wave_phase * 2.4) * wave_amplitude.y +
             cos(pos.x * 0.02 + wave_phase * 5.2) * wave_amplitude.y * 0.3;

    gl_Position   = vec4(dot(za_u_mvpRow0, pos), dot(za_u_mvpRow1, pos), 0.0, 1.0);
    za_v_texCoord = za_a_texCoord * za_u_invTextureSize;
    za_v_color    = za_a_color;
}
