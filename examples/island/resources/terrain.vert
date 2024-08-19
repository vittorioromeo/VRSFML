#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec3 normal;

uniform mat4 sf_u_textureMatrix;
uniform mat4 sf_u_modelViewProjectionMatrix;

in vec3 sf_a_position;
in vec4 sf_a_color;
in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    gl_Position   = sf_u_modelViewProjectionMatrix * vec4(sf_a_position, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
    normal        = vec3(sf_a_texCoord.xy, 1.0);
}
