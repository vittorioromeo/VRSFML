layout(location = 2) uniform sampler2D sf_u_texture;

////////////////////////////////////////////////////////////
in vec4 sf_v_color;
in vec2 sf_v_texCoord;

out vec4 sf_fragColor;

////////////////////////////////////////////////////////////
uniform float u_vibrance;   // e.g., 0.0 (none) to 1.0 (max boost)
uniform float u_saturation; // e.g., 1.0 (no change), >1.0 (more saturated)
uniform float u_lightness;  // e.g., 1.0 (no change), >1.0 (brighter)
uniform float u_sharpness;  // 0.0 means no sharpening, 1.0 means full effect
uniform float u_blur;       // 0.0 means no blur, 1.0 means strong blur

////////////////////////////////////////////////////////////
vec3 adjustSaturation(vec3 color, float sat)
{
    float gray = dot(color, vec3(0.299, 0.587, 0.114));

    // Adjust u_saturation by mixing the original color with its grayscale equivalent.
    return mix(vec3(gray), color, sat);
}

////////////////////////////////////////////////////////////
vec3 adjustVibrance(vec3 color, float vib)
{
    float average = (color.r + color.g + color.b) / 3.0;
    vec3  boosted = color + (color - average) * vib;

    return clamp(boosted, 0.0, 1.0);
}

////////////////////////////////////////////////////////////
vec3 adjustLightness(vec3 color, float light)
{
    return color * light;
}

////////////////////////////////////////////////////////////
vec4 getAdjustedSample(vec2 coord)
{
    vec4 sampleColor = texture(sf_u_texture, coord);
    vec3 c           = sampleColor.rgb;

    c = adjustVibrance(c, u_vibrance);
    c = adjustSaturation(c, u_saturation);
    c = adjustLightness(c, u_lightness);

    return vec4(c, sampleColor.a);
}

////////////////////////////////////////////////////////////
vec4 getBlurredSample(vec2 coord)
{
    vec2 texel       = 1.0 / vec2(textureSize(sf_u_texture, 0));
    float radius     = mix(1.0, 3.5, clamp(u_blur, 0.0, 1.0));
    vec2 axisStep    = texel * radius;
    vec2 axisStepFar = axisStep * 2.0;
    vec2 diagStep    = axisStep * 0.85;

    vec4 blur = getAdjustedSample(coord) * 0.19648255;

    blur += getAdjustedSample(coord + vec2(axisStep.x, 0.0)) * 0.12331767;
    blur += getAdjustedSample(coord - vec2(axisStep.x, 0.0)) * 0.12331767;
    blur += getAdjustedSample(coord + vec2(0.0, axisStep.y)) * 0.12331767;
    blur += getAdjustedSample(coord - vec2(0.0, axisStep.y)) * 0.12331767;

    blur += getAdjustedSample(coord + vec2(diagStep.x, diagStep.y)) * 0.07784700;
    blur += getAdjustedSample(coord + vec2(diagStep.x, -diagStep.y)) * 0.07784700;
    blur += getAdjustedSample(coord + vec2(-diagStep.x, diagStep.y)) * 0.07784700;
    blur += getAdjustedSample(coord - vec2(diagStep.x, diagStep.y)) * 0.07784700;

    blur += getAdjustedSample(coord + vec2(axisStepFar.x, 0.0)) * 0.01807500;
    blur += getAdjustedSample(coord - vec2(axisStepFar.x, 0.0)) * 0.01807500;
    blur += getAdjustedSample(coord + vec2(0.0, axisStepFar.y)) * 0.01807500;
    blur += getAdjustedSample(coord - vec2(0.0, axisStepFar.y)) * 0.01807500;

    return blur;
}

////////////////////////////////////////////////////////////
void main()
{
    vec4 center = getAdjustedSample(sf_v_texCoord);
    vec4 blur   = getBlurredSample(sf_v_texCoord);

    vec4 result = mix(center, blur, clamp(u_blur, 0.0, 1.0));

    if (u_sharpness > 0.0)
        result.rgb = clamp(result.rgb + u_sharpness * (center.rgb - blur.rgb), 0.0, 1.0);

    sf_fragColor = vec4(result.rgb, result.a * sf_v_color.a);
}
