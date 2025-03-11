layout(location = 1) uniform sampler2D sf_u_texture;

////////////////////////////////////////////////////////////
in vec4 sf_v_color;
in vec2 sf_v_texCoord;

out vec4 sf_fragColor;

////////////////////////////////////////////////////////////
uniform float u_vibrance;   // e.g., 0.0 (none) to 1.0 (max boost)
uniform float u_saturation; // e.g., 1.0 (no change), >1.0 (more saturated)
uniform float u_lightness;  // e.g., 1.0 (no change), >1.0 (brighter)
uniform float u_sharpness;  // 0.0 means no sharpening, 1.0 means full effect

////////////////////////////////////////////////////////////
vec3 adjustSaturation(vec3 color, float sat)
{
    const float gray = dot(color, vec3(0.299, 0.587, 0.114));

    // Adjust u_saturation by mixing the original color with its grayscale equivalent.
    return mix(vec3(gray), color, sat);
}

////////////////////////////////////////////////////////////
vec3 adjustVibrance(vec3 color, float vib)
{
    const float average = (color.r + color.g + color.b) / 3.0;
    const vec3  boosted = color + (color - average) * vib;

    return clamp(boosted, 0.0, 1.0);
}

////////////////////////////////////////////////////////////
vec3 adjustLightness(vec3 color, float light)
{
    return color * light;
}

////////////////////////////////////////////////////////////
vec3 getAdjustedColor(vec2 coord)
{
    vec3 c = texture(sf_u_texture, coord).rgb;

    c = adjustVibrance(c, u_vibrance);
    c = adjustSaturation(c, u_saturation);
    c = adjustLightness(c, u_lightness);

    return c;
}

////////////////////////////////////////////////////////////
void main()
{
    // Compute the adjusted color for the center pixel.
    const vec3 center = getAdjustedColor(sf_v_texCoord);

    // Initialize sharpened color as the adjusted center.
    vec3 result = center;

    // If u_sharpness is enabled, perform a 3x3 convolution on the adjusted colors.
    if (u_sharpness > 0.0)
    {
        // Determine texel size.
        const vec2 tex_offset = 1.0 / vec2(textureSize(sf_u_texture, 0));

        // Compute a simple blurred version using a 3x3 kernel with weights approximating a Gaussian.
        vec3 blur = getAdjustedColor(sf_v_texCoord) * 4.0;
        blur += getAdjustedColor(sf_v_texCoord + vec2(-tex_offset.x, 0.0)) * 2.0;
        blur += getAdjustedColor(sf_v_texCoord + vec2(tex_offset.x, 0.0)) * 2.0;
        blur += getAdjustedColor(sf_v_texCoord + vec2(0.0, -tex_offset.y)) * 2.0;
        blur += getAdjustedColor(sf_v_texCoord + vec2(0.0, tex_offset.y)) * 2.0;
        blur += getAdjustedColor(sf_v_texCoord + vec2(-tex_offset.x, -tex_offset.y));
        blur += getAdjustedColor(sf_v_texCoord + vec2(tex_offset.x, -tex_offset.y));
        blur += getAdjustedColor(sf_v_texCoord + vec2(-tex_offset.x, tex_offset.y));
        blur += getAdjustedColor(sf_v_texCoord + vec2(tex_offset.x, tex_offset.y));
        blur /= 16.0;

        // Unsharp mask: add back the difference between the original and blurred version.
        result = center + u_sharpness * (center - blur);
        result = clamp(result, 0.0, 1.0);
    }

    sf_fragColor = vec4(result, sf_v_color.a);
}
