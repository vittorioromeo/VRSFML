layout(location = 1) uniform sampler2D sf_u_texture;
layout(location = 2) uniform sampler2D u_backgroundTexture; // Background texture for bubble distortion
layout(location = 3) uniform float u_time;                  // Time for animations
layout(location = 4) uniform vec2 u_resolution;             // Screen resolution
layout(location = 6) uniform bool u_bubbleEffect;

layout(location = 7) uniform float u_iridescenceStrength;
layout(location = 8) uniform float u_edgeFactorMin;
layout(location = 9) uniform float u_edgeFactorMax;
layout(location = 10) uniform float u_edgeFactorStrength;
layout(location = 11) uniform float u_distorsionStrength;

layout(location = 12) uniform vec2 u_subTexOrigin; // Bottom-left corner of the sub-texture (pixels)
layout(location = 13) uniform vec2 u_subTexSize;   // Size of the sub-texture (pixels)

layout(location = 14) uniform float u_bubbleLightness;
layout(location = 15) uniform float u_lensDistortion;

////////////////////////////////////////////////////////////
in vec4 sf_v_color;
in vec2 sf_v_texCoord;
in vec2 v_worldPos;

out vec4 sf_fragColor;

////////////////////////////////////////////////////////////
vec3 rgb2hsv(vec3 c)
{
    const vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4       p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4       q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float       d = q.x - min(q.w, q.y);
    const float e = 1.0e-10;

    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

////////////////////////////////////////////////////////////
vec3 hsv2rgb(vec3 c)
{
    const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3       p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);

    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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
    vec4 texColor = texture(sf_u_texture, sf_v_texCoord);

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

    vec3 finalColor = texColor.rgb;

    if (u_bubbleEffect)
    {
        // Convert atlas UVs to local UVs within the sub-texture
        vec2 localUV = (sf_v_texCoord * vec2(textureSize(sf_u_texture, 0)) - u_subTexOrigin) / u_subTexSize;

        // Center the UV coordinates
        vec2 centeredUV = localUV - vec2(0.5);

        // Aspect ratio correction
        centeredUV.x *= u_subTexSize.x / u_subTexSize.y;

        // Compute radial distance from center
        float distance = length(centeredUV);

        // Calculate proper background UVs
        vec2 bgUV = v_worldPos / float(textureSize(u_backgroundTexture, 0));

        // Create a dynamic distortion pattern using time and color
        float distortionTime   = u_time * 0.2 + float(sf_v_color.b) * 10.0;
        vec2  angle            = distortionTime * 0.5 + vec2(bgUV.y * 3.5, bgUV.x * 5.5);
        vec2  sinCosDistortion = vec2(sin(angle.x), cos(angle.y)) * u_distorsionStrength;

        // Compute a lens-like radial distortion using bubble texture coordinates
        vec2 lensDistortion = centeredUV * (distance * u_lensDistortion); // Adjust this factor as needed

        // Combine the distortions
        vec2 totalDistortion = sinCosDistortion * 0.75 + lensDistortion;

        // New sample coordinate for the background texture
        vec2 sampleCoord = bgUV + totalDistortion * 0.08;

        // Apply a simple blur by averaging nearby samples
        vec3 sampleCenter = texture(u_backgroundTexture, sampleCoord).rgb;
        vec3 sampleRight  = texture(u_backgroundTexture, sampleCoord + vec2(1.0 / u_resolution.x, 0)).rgb;
        vec3 sampleLeft   = texture(u_backgroundTexture, sampleCoord - vec2(1.0 / u_resolution.x, 0)).rgb;
        vec3 sampleUp     = texture(u_backgroundTexture, sampleCoord + vec2(0, 1.0 / u_resolution.y)).rgb;
        vec3 sampleDown   = texture(u_backgroundTexture, sampleCoord - vec2(0, 1.0 / u_resolution.y)).rgb;
        vec3 bgColor      = (sampleCenter + sampleRight + sampleLeft + sampleUp + sampleDown) / 5.0;

        float edgeFactor = distance;

        // Generate iridescent color pattern
        vec2 patternCoord = centeredUV * 10.0;
        vec3 iridescence  = 0.25 +
                           0.5 * cos(u_time * 2.0 + distance * 25.0 + float(sf_v_color.b) * 100.0 + vec3(0.0, 2.0, 4.0)) +
                           (0.45 * sin(u_time * 3.0 + float(sf_v_color.b) * 100.0 + patternCoord.y + patternCoord.x));
        iridescence *= edgeFactor * u_edgeFactorStrength; // Stronger at edges

        // Combine effects
        float alphaFalloff     = smoothstep(u_edgeFactorMin, u_edgeFactorMax, distance);
        float bubbleVisibility = texColor.a * alphaFalloff * 1.2;

        finalColor = mix(bgColor + vec3(u_bubbleLightness),                // Distorted background
                         finalColor + iridescence * u_iridescenceStrength, // Bubble color with iridescence
                         clamp(bubbleVisibility, 0.0, 1.0)                 // Alpha falloff
        );
    }

    // Apply hue shift
    finalColor = rotateHueRGB(finalColor, float(sf_v_color.b) * 360.0);

    // Proper alpha premultiplication
    if (u_bubbleEffect)
    {
        sf_fragColor = vec4(finalColor * texColor.a, texColor.a);
    }
    else
    {
        sf_fragColor = vec4(finalColor, sf_v_color.a * texColor.a);
    }
}
