#version 150

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 sf_u_projectionMatrix;
uniform mat4 sf_u_modelViewMatrix;
uniform mat4 sf_u_textureMatrix;

in vec2 sf_a_position;
in vec4 sf_a_color;
in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position   = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
}
