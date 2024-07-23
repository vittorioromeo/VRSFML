#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;
uniform float     pixel_threshold;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    float factor = 1.0 / (pixel_threshold + 0.001);
    vec2  pos    = floor(sf_v_texCoord.xy * factor + 0.5) / factor;
    gl_FragColor = texture2D(sf_u_texture, pos) * sf_v_color;
}
