uniform float wave_phase;
uniform vec2 wave_amplitude;

uniform mat4 projMatrix;
uniform mat4 textMatrix;
uniform mat4 viewMatrix;

attribute vec4 color;
attribute vec2 position;
attribute vec2 texCoord;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    vec2 newPosition = position;

    newPosition.x += cos(newPosition.y * 0.02 + wave_phase * 3.8) * wave_amplitude.x
                + sin(newPosition.y * 0.02 + wave_phase * 6.3) * wave_amplitude.x * 0.3;

    newPosition.y += sin(newPosition.x * 0.02 + wave_phase * 2.4) * wave_amplitude.y
                + cos(newPosition.x * 0.02 + wave_phase * 5.2) * wave_amplitude.y * 0.3;

    gl_Position = projMatrix * viewMatrix * vec4(newPosition, 0.0, 1.0);
    v_texCoord = (textMatrix * vec4(texCoord, 0.0, 1.0)).xy;
    v_color = color;
}
