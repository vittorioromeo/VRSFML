layout(location = 1) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    float alpha  = texture(sf_u_texture, sf_v_texCoord).a;
    sf_fragColor = vec4(1.0, 1.0, 1.0, alpha);
}
