layout(location = 2) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

out vec4 sf_fragColor;

uniform float u_time;
uniform float u_seed;
uniform float u_distortionStrength;
uniform float u_shimmerStrength;

vec3 rotateHueRGB(vec3 color, float angleDegrees)
{
    const float sqrt3 = 1.73205080757;

    float angleRadians = radians(angleDegrees);
    float c            = cos(angleRadians);
    float s            = sin(angleRadians);

    float k0 = (1.0 + 2.0 * c) / 3.0;
    float k1 = (1.0 - c - sqrt3 * s) / 3.0;
    float k2 = (1.0 - c + sqrt3 * s) / 3.0;

    vec3 rotated;
    rotated.r = k0 * color.r + k1 * color.g + k2 * color.b;
    rotated.g = k2 * color.r + k0 * color.g + k1 * color.b;
    rotated.b = k1 * color.r + k2 * color.g + k0 * color.b;

    return clamp(rotated, 0.0, 1.0);
}

void main()
{
    const vec2 flagTarget = vec2(1.0 / 255.0);
    const vec2 epsilon    = vec2(0.001);

    vec2 localUv = sf_v_texCoord;
    vec2 centeredUv = localUv * 2.0 - 1.0;
    float radius = length(centeredUv);

    float edgeMask = 1.0 - smoothstep(0.52, 1.08, radius);
    float wave0 = sin(localUv.y * 24.0 + u_time * 4.8 + u_seed);
    float wave1 = cos(localUv.x * 19.0 - u_time * 3.9 + u_seed * 1.37);
    float wave2 = sin((localUv.x + localUv.y) * 16.0 + u_time * 5.7 - u_seed * 0.63);

    vec2 distortion = vec2(wave0 + wave2 * 0.55, wave1 - wave2 * 0.45) *
                      (0.0075 * u_distortionStrength) * edgeMask;

    vec4 texColor = texture(sf_u_texture, clamp(localUv + distortion, vec2(0.0), vec2(1.0)));

    if (texColor.a < 0.01)
        discard;

    bool hueDriven = all(lessThanEqual(abs(sf_v_color.rg - flagTarget), epsilon));

    vec3 finalColor;
    float alpha;

    if (hueDriven)
    {
        finalColor = rotateHueRGB(texColor.rgb, float(sf_v_color.b) * 360.0);
        alpha      = sf_v_color.a * texColor.a;
    }
    else
    {
        vec4 shaded = sf_v_color * texColor;
        finalColor  = shaded.rgb;
        alpha       = shaded.a;
    }

    float shimmer = 0.5 + 0.5 * sin((localUv.x - localUv.y) * 28.0 + u_time * 7.2 + u_seed * 1.91);
    float phasePulse = 0.5 + 0.5 * sin(radius * 18.0 - u_time * 5.1 + u_seed * 0.77);

    vec2 phaseUv = localUv + distortion * 8.0;
    float riftWave0 = sin(phaseUv.y * 36.0 - u_time * 4.6 + u_seed * 1.4);
    float riftWave1 = sin((phaseUv.x + phaseUv.y * 0.55) * 48.0 + u_time * 3.1 - u_seed * 0.9);
    float riftField = riftWave0 * 0.65 + riftWave1 * 0.35;

    float riftThreshold = mix(1.1, 0.45, u_distortionStrength);
    float cutMask = smoothstep(riftThreshold - 0.18, riftThreshold + 0.03, riftField) * edgeMask;
    float cutRim = (smoothstep(riftThreshold - 0.24, riftThreshold - 0.06, riftField) -
                    smoothstep(riftThreshold - 0.06, riftThreshold + 0.08, riftField)) * edgeMask;

    vec3 shimmerTint = mix(vec3(0.55, 0.85, 1.0), vec3(0.95, 0.8, 1.0), shimmer);
    finalColor += shimmerTint * ((0.08 + 0.12 * phasePulse) * u_shimmerStrength * edgeMask);
    finalColor += mix(vec3(0.65, 0.95, 1.0), vec3(1.0, 0.92, 1.0), phasePulse) *
                  (0.28 * cutRim * u_shimmerStrength);

    alpha *= 0.94 + 0.06 * phasePulse;
    alpha *= 1.0 - cutMask * (0.82 + 0.12 * phasePulse);

    if (alpha < 0.01)
        discard;

    sf_fragColor = vec4(clamp(finalColor, 0.0, 1.0), alpha);
}
