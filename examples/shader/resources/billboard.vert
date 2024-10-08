layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 1) uniform vec3 sf_u_texParams;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    gl_Position   = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_texCoord = vec2(sf_u_texParams[0] * sf_a_texCoord.x, 
                         sf_u_texParams[1] * sf_a_texCoord.y + sf_u_texParams[2]);
    sf_v_color    = sf_a_color;
}
