uniform float wave_phase;
uniform vec2  wave_amplitude;

layout(location = 0) uniform mat4 sf_u_modelViewProjectionMatrix;
layout(location = 1) uniform mat4 sf_u_textureMatrix;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    vec2 newPosition = sf_a_position;

    newPosition.x += cos(newPosition.y * 0.02 + wave_phase * 3.8) * wave_amplitude.x +
                     sin(newPosition.y * 0.02 + wave_phase * 6.3) * wave_amplitude.x * 0.3;

    newPosition.y += sin(newPosition.x * 0.02 + wave_phase * 2.4) * wave_amplitude.y +
                     cos(newPosition.x * 0.02 + wave_phase * 5.2) * wave_amplitude.y * 0.3;

    gl_Position   = sf_u_modelViewProjectionMatrix * vec4(newPosition, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
}
