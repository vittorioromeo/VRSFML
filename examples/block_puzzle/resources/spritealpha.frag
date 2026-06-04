layout(location = 2) uniform sampler2D za_u_texture;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    float alpha  = texture(za_u_texture, za_v_texCoord).a;
    za_fragColor = vec4(1.0, 1.0, 1.0, alpha);
}
