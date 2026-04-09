layout(location = 0) uniform vec3 sf_u_mvpRow0;
layout(location = 1) uniform vec3 sf_u_mvpRow1;
layout(location = 2) uniform sampler2D sf_u_texture;
layout(location = 3) uniform vec2 sf_u_invTextureSize;
layout(location = 4) uniform sampler2D u_backgroundTexture; // Background texture for bubble distortion
layout(location = 5) uniform float u_time;                  // Time for animations
layout(location = 6) uniform bool u_bubbleEffect;
layout(location = 7) uniform vec2 u_resolution;             // Screen resolution

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

////////////////////////////////////////////////////////////
out vec4 sf_v_color;
out vec2 sf_v_texCoord;
out vec2 v_worldPos; // Position without MVP matrix multiplication

////////////////////////////////////////////////////////////
void main()
{
    vec3 pos = vec3(sf_a_position, 1.0);

    gl_Position = vec4(dot(sf_u_mvpRow0, pos), dot(sf_u_mvpRow1, pos), 0.0, 1.0);
    sf_v_color    = sf_a_color;
    sf_v_texCoord = sf_a_texCoord * sf_u_invTextureSize;

    v_worldPos = sf_a_position;
}
