layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 1) uniform sampler2D sf_u_texture;
layout(location = 2) uniform sampler2D u_backgroundTexture; // Background texture for bubble distortion
layout(location = 3) uniform float u_time;                  // Time for animations
layout(location = 4) uniform vec2 u_resolution;             // Screen resolution
layout(location = 6) uniform bool u_bubbleEffect;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;
out vec2 v_worldPos; // Position without MVP matrix multiplication

void main()
{
    gl_Position   = sf_u_mvpMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color    = sf_a_color;
    sf_v_texCoord = sf_a_texCoord / vec2(textureSize(sf_u_texture, 0));

    v_worldPos = sf_a_position;
}
