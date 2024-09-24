#version 310 es

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 2) uniform sampler2D sf_u_texture;

layout(location = 0) in vec4 sf_v_color;
layout(location = 1) in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = texture(sf_u_texture, sf_v_texCoord);
}
