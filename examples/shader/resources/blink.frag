uniform sampler2D texture;
uniform float blink_alpha;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    vec4 pixel = v_color;
    pixel.a = blink_alpha;

    gl_FragColor = pixel;
}
