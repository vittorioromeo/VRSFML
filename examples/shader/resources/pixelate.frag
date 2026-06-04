#include "default_uniforms.glsl"

uniform float pixel_threshold;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    float factor = 1.0 / (pixel_threshold + 0.001);
    vec2  pos    = floor(za_v_texCoord * factor + 0.5) / factor;
    za_fragColor = texture(za_u_texture, pos) * za_v_color;
}
