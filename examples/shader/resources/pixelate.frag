uniform sampler2D texture;
uniform float pixel_threshold;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    float factor = 1.0 / (pixel_threshold + 0.001);
    vec2 pos = floor(v_texCoord.xy * factor + 0.5) / factor;
    gl_FragColor = texture2D(texture, pos) * v_color;
}
