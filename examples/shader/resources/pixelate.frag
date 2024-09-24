layout(location = 2) uniform sampler2D sf_u_texture;
uniform float pixel_threshold;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    float factor = 1.0 / (pixel_threshold + 0.001);
    vec2  pos    = floor(sf_v_texCoord.xy * factor + 0.5) / factor;
    sf_fragColor = texture(sf_u_texture, pos) * sf_v_color;
}
