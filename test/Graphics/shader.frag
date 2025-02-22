layout(location = 1) uniform sampler2D sf_u_texture;
layout(location = 6) uniform float blink_alpha;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    vec4 pixel = sf_v_color;
    pixel.a    = blink_alpha;

    sf_fragColor = pixel;
}
