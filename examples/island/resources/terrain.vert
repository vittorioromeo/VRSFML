#version 310 es

#ifdef GL_ES
precision mediump float;
#endif

out vec3 normal;

layout(location = 0) uniform mat4 sf_u_modelViewProjectionMatrix;
layout(location = 1) uniform mat4 sf_u_textureMatrix;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

layout(location = 0) out vec4 sf_v_color;
layout(location = 1) out vec2 sf_v_texCoord;

void main()
{
    gl_Position   = sf_u_modelViewProjectionMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
    normal        = vec3(sf_a_texCoord.xy, 1.0);
}
