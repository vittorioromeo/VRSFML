#version 310 es

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 2) uniform sampler2D sf_u_texture;
uniform float     blink_alpha;

layout(location = 0) in vec4 sf_v_color;
layout(location = 1) in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    vec4 pixel = sf_v_color;
    pixel.a    = blink_alpha;

    sf_fragColor = pixel;
}
