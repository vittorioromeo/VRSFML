#include "default_uniforms.glsl"

layout(location = 7) uniform float blink_alpha;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    vec4 pixel = za_v_color;
    pixel.a    = blink_alpha;

    za_fragColor = pixel;
}
