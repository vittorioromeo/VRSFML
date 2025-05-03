layout(location = 1) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = texture(sf_u_texture, sf_v_texCoord);
}
