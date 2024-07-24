#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

out vec4 sf_fragColor;

void main()
{
    sf_fragColor = texture(sf_u_texture, sf_v_texCoord.st);
}
