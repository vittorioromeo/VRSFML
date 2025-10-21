layout(location = 0) out vec4 sf_fragColor;
layout(location = 1) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

uniform vec2  u_blurDirection;    // (1.0, 0.0) for horizontal, (0.0, 1.0) for vertical
uniform float u_blurRadiusPixels; // How many pixels the blur kernel extends

void main()
{
    vec4  blurredColor = vec4(0.0);
    float totalWeight  = 0.0;

    float sigma      = max(u_blurRadiusPixels / 2.0, 0.1);
    float twoSigmaSq = 2.0 * sigma * sigma;

    float centerWeight = 1.0;
    blurredColor += texture(sf_u_texture, sf_v_texCoord) * centerWeight;
    totalWeight += centerWeight;

    vec2 texelSize     = vec2(1.0) / textureSize(sf_u_texture, 0); // TODO P1: move to uniform
    vec2 stepDirection = u_blurDirection * texelSize;

    for (int i = 1; i <= int(ceil(u_blurRadiusPixels)); ++i)
    {
        float distance = float(i);
        float weight   = exp(-(distance * distance) / twoSigmaSq);

        vec2 offset = stepDirection * distance;

        blurredColor += texture(sf_u_texture, sf_v_texCoord + offset) * weight;
        blurredColor += texture(sf_u_texture, sf_v_texCoord - offset) * weight;

        totalWeight += 2.0 * weight;
    }

    sf_fragColor = blurredColor / totalWeight;
}
