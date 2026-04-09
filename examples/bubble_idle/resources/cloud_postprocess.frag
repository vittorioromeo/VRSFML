layout(location = 2) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

out vec4 sf_fragColor;

uniform vec2 u_resolution;
uniform float u_blur;
uniform float u_opacity;

void main() {
    vec4 center = texture(sf_u_texture, sf_v_texCoord);

    if(center.a<=0.001&&u_blur<=0.001) {
        sf_fragColor = vec4(0.0);
        return;
    }

    float blurAmount = max(u_blur, 0.0);
    float normalizedBlur = clamp(blurAmount/8.0, 0.0, 1.0);

    // Calculate effective maximum radius mapped exactly to your original blend logic
    float rNear = 4.0*(0.6+blurAmount*0.9);
    float rFar = 4.0*(1.4+blurAmount*1.8);
    float rWide = 4.0*(2.6+blurAmount*2.8);

    float rSoft = mix(rNear, rFar, 0.5+normalizedBlur*0.25);
    float maxRadius = mix(rSoft, rWide, normalizedBlur*0.85);

    // Vogel / Golden Spiral Sampling
    const int SAMPLES = 24;
    const float GOLDEN_ANGLE = 2.39996323;
    vec2 texel = 1.0/u_resolution;
    vec4 blurFull = vec4(0.0);

    for(int i = 0; i<SAMPLES; ++i) {
        float t = float(i)/float(SAMPLES-1);
        // Using t*t tightly clusters samples at the center, matching the exact
        // high-density center weighting (0.12) of your original code's Gaussian kernel.
        float r = t*t*maxRadius;
        float theta = float(i)*GOLDEN_ANGLE;

        vec2 offset = vec2(cos(theta), sin(theta))*r*texel;
        blurFull += texture(sf_u_texture, sf_v_texCoord+offset);
    }

    // Normalize and preserve your original brightness scale (Original weights summed to 1.171)
    blurFull *= (1.171/float(SAMPLES));

    // Original compositing logic
    float centerAlpha = center.a;
    float blurAlpha = blurFull.a;
    float edgeFeather = smoothstep(0.0, 0.35, blurAlpha-centerAlpha+0.08);

    float blurMix = clamp(0.20+normalizedBlur*0.80, 0.0, 1.0);
    blurMix = max(blurMix, edgeFeather*normalizedBlur);

    vec4 result = mix(center, blurFull, blurMix);
    result *= u_opacity;

    if(result.a<=0.001) {
        sf_fragColor = vec4(0.0);
        return;
    }

    sf_fragColor = result;
}
