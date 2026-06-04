#include "default_uniforms.glsl"

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    za_fragColor = texture(za_u_texture, za_v_texCoord);
}
