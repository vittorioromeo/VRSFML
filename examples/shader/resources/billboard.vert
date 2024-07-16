#version 150

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
    gl_Position = projMatrix * viewMatrix * vec4(position, 0.0, 1.0);
    v_texCoord  = (textMatrix * vec4(texCoord, 0.0, 1.0)).xy;
    v_color     = color;
}
