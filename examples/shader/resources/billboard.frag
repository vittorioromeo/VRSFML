#version 150

uniform sampler2D texture;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    gl_FragColor = texture2D(texture, v_texCoord.st);
}
