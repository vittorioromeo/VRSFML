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
    const vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    const vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    const float d = q.x - min(q.w, q.y);
    const float e = 1.0e-10;

    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

////////////////////////////////////////////////////////////
vec3 hsv2rgb(vec3 c)
{
    const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    const vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

////////////////////////////////////////////////////////////
void main()
{
    const vec4 texColor = texture(sf_u_texture, sf_v_texCoord);

    if (texColor.a < 0.01)
        discard;

    const vec2 flagTarget = vec2(1.0 / 255.0);
    const vec2 epsilon    = vec2(0.001);
    const bool hueDriven  = all(lessThanEqual(abs(sf_v_color.rg - flagTarget), epsilon));

    if (!hueDriven)
    {
        sf_fragColor = sf_v_color * texColor;
        return;
    }

    vec3 finalColor = texColor.rgb;

    if (u_bubbleEffect)
    {
        // Convert atlas UVs to local UVs within the sub-texture
        const vec2 localUV = (sf_v_texCoord * vec2(textureSize(sf_u_texture, 0)) - u_subTexOrigin) / u_subTexSize;

        // Center the UV coordinates
        vec2 centeredUV = localUV - vec2(0.5);

        // Aspect ratio correction
        centeredUV.x *= u_subTexSize.x / u_subTexSize.y;

        // Compute radial distance from center
        const float distance = length(centeredUV);

        // Calculate proper background UVs
        const vec2 bgUV = v_worldPos / textureSize(u_backgroundTexture, 0);

        // Create a dynamic distortion pattern using time and color
        const float distortionTime   = u_time * 0.2 + sf_v_color.b * 10.0;
        const vec2  angle            = distortionTime * 0.5 + vec2(bgUV.y * 3.5, bgUV.x * 5.5);
        const vec2  sinCosDistortion = vec2(sin(angle.x), cos(angle.y)) * u_distorsionStrength;

        // Compute a lens-like radial distortion using bubble texture coordinates
        const vec2 lensDistortion = centeredUV * (distance * u_lensDistortion); // Adjust this factor as needed

        // Combine the distortions
        const vec2 totalDistortion = sinCosDistortion * 0.75 + lensDistortion;

        // New sample coordinate for the background texture
        const vec2 sampleCoord = bgUV + totalDistortion * 0.08;

        // Apply a simple blur by averaging nearby samples
        const vec3 sampleCenter = texture(u_backgroundTexture, sampleCoord).rgb;
        const vec3 sampleRight  = texture(u_backgroundTexture, sampleCoord + vec2(1.0 / u_resolution.x, 0)).rgb;
        const vec3 sampleLeft   = texture(u_backgroundTexture, sampleCoord - vec2(1.0 / u_resolution.x, 0)).rgb;
        const vec3 sampleUp     = texture(u_backgroundTexture, sampleCoord + vec2(0, 1.0 / u_resolution.y)).rgb;
        const vec3 sampleDown   = texture(u_backgroundTexture, sampleCoord - vec2(0, 1.0 / u_resolution.y)).rgb;
        const vec3 bgColor      = (sampleCenter + sampleRight + sampleLeft + sampleUp + sampleDown) / 5.0;

        const float edgeFactor = distance;

        // Generate iridescent color pattern
        vec3 iridescence = 0.25 + 0.5 * cos(u_time * 2.0 + distance * 25.0 + sf_v_color.b * 100 + vec3(0, 2, 4)) +
                           (0.45 * sin(u_time * 3.0 + (sf_v_color.b * 100 + v_worldPos.y * 500)));
        iridescence *= edgeFactor * u_edgeFactorStrength; // Stronger at edges

        // Combine effects
        const float alphaFalloff     = smoothstep(u_edgeFactorMin, u_edgeFactorMax, distance);
        const float bubbleVisibility = texColor.a * alphaFalloff * 1.2;

        finalColor = mix(bgColor + vec3(u_bubbleLightness),                // Distorted background
                         finalColor + iridescence * u_iridescenceStrength, // Bubble color with iridescence
                         clamp(bubbleVisibility, 0.0, 1.0)                 // Alpha falloff
        );

        // Add subtle surface noise
        const vec2  screenUV = gl_FragCoord.xy / u_resolution;
        const float noise    = fract(sin(dot(screenUV, vec2(12.9898, 78.233)) * 43758.5453));
        finalColor += noise * 0.01 * edgeFactor;
    }

    // Apply hue shift
    vec3 hsv   = rgb2hsv(finalColor);
    hsv.x      = mod(hsv.x + sf_v_color.b, 360.0);
    finalColor = hsv2rgb(hsv);

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
