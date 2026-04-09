layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 4) uniform vec2 sf_u_invTextureSize;
uniform vec2 u_viewOrigin;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;
out vec2 v_worldPos;

void main()
{
    gl_Position   = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color    = sf_a_color;
    sf_v_texCoord = sf_a_texCoord * sf_u_invTextureSize;
    v_worldPos    = u_viewOrigin + sf_a_position;
}
