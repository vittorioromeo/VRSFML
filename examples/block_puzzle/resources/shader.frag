layout(location = 1) uniform sampler2D sf_u_texture;
uniform float u_time;
uniform bool u_waveEnabled;

////////////////////////////////////////////////////////////
in vec4 sf_v_color;
in vec2 sf_v_texCoord;
in vec2 v_worldPos;

out vec4 sf_fragColor;

////////////////////////////////////////////////////////////
// Rotates hue in RGB space
// color: input vec3 color (components in [0,1])
// angle_degrees: rotation angle in degrees
////////////////////////////////////////////////////////////
vec3 rotateHueRGB(vec3 color, float angle_degrees)
{
    const float SQRT3 = 1.73205080757; // sqrt(3.0)

    float angle_radians = radians(angle_degrees);
    float c             = cos(angle_radians);
    float s             = sin(angle_radians);

    float k0 = (1.0 + 2.0 * c) / 3.0;
    float k1 = (1.0 - c - SQRT3 * s) / 3.0;
    float k2 = (1.0 - c + SQRT3 * s) / 3.0;

    vec3 newColor;
    newColor.r = k0 * color.r + k1 * color.g + k2 * color.b;
    newColor.g = k2 * color.r + k0 * color.g + k1 * color.b;
    newColor.b = k1 * color.r + k2 * color.g + k0 * color.b;

    newColor = clamp(newColor, 0.0, 1.0);
    return newColor;
}

////////////////////////////////////////////////////////////
void main()
{
    vec2 waveCoord = sf_v_texCoord;

    if (u_waveEnabled)
    {
        // Control wave properties
        float waveFrequency = 10.0;  // How many waves across the width
        float waveAmplitude = 0.02;  // How much to distort vertically
        float waveSpeed     = 0.001; // Speed of wave motion

        // Apply horizontal wave effect based on Y coordinate and time
        waveCoord.y += sin(waveCoord.x * waveFrequency + u_time * waveSpeed) * waveAmplitude;
        waveCoord.x += cos(waveCoord.y * waveFrequency + u_time * waveSpeed) * waveAmplitude;
    }

    // Sample texture at distorted coordinates
    vec4 texColor = texture(sf_u_texture, waveCoord);

    if (texColor.a < 0.01)
        discard;

    const vec2 flagTarget = vec2(1.0 / 255.0);
    const vec2 epsilon    = vec2(0.001);
    bool       hueDriven  = all(lessThanEqual(abs(sf_v_color.rg - flagTarget), epsilon));

    if (!hueDriven)
    {
        sf_fragColor = sf_v_color * texColor;
        return;
    }

    vec3 finalColor = rotateHueRGB(texColor.rgb, float(sf_v_color.b) * 360.0);
    sf_fragColor    = vec4(finalColor, sf_v_color.a * texColor.a);
}
