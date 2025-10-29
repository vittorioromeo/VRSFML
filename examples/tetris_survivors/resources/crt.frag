//================================================================================
// UNIFORMS & INPUTS
//================================================================================

layout(location = 1) uniform sampler2D sf_u_texture;

uniform vec2  u_inputSize;
uniform float u_time;

in vec2 sf_v_texCoord;
layout(location = 0) out vec4 sf_fragColor;

//================================================================================
// SHADER CONFIGURATION
//================================================================================

uniform float u_curvature = -0.02;

uniform float u_vignetteStrength    = 0.25;
uniform float u_vignetteInnerRadius = 0.5;
uniform float u_vignetteOuterRadius = 1.0;

uniform float u_scanlineStrength             = 0.7;
uniform float u_scanlineBrightnessModulation = 1.2;
uniform float u_scanlineScrollSpeed          = 55.0;
uniform float u_scanlineThickness            = 4.0;
uniform float u_scanlineHeight               = 1.0;

uniform float u_noiseStrength = 45;

// --- Phosphor Mask (The Grille) ---
// #define MASK_APERTURE_GRILLE
#define MASK_SLOT
uniform float u_maskStrength = 0.25;
uniform float u_maskScale    = 0.25;

uniform float u_bloomStrength = 0.5;
uniform float u_inputGamma    = 2.3;
uniform float u_outputGamma   = 2.6;
uniform float u_saturation    = 1.15;

//================================================================================
// HELPER FUNCTIONS (Unchanged)
//================================================================================

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 curve_uv(vec2 uv)
{
    uv          = uv * 2.0 - 1.0; // to -1..1 range
    vec2 offset = uv.yx / u_curvature;
    uv          = uv + uv * (dot(uv, uv) * -u_curvature);
    return uv * 0.5 + 0.5; // back to 0..1 range
}

// Applies saturation adjustment
vec3 adjust_saturation(vec3 col, float sat)
{
    vec3 gray = vec3(dot(col, vec3(0.299, 0.587, 0.114)));
    return mix(gray, col, sat);
}


//================================================================================
// MAIN
//================================================================================

void main()
{
    // 1. COORDINATE SETUP & u_curvature
    vec2 curvedUV = curve_uv(sf_v_texCoord);
    if (curvedUV.x < 0.0 || curvedUV.x > 1.0 || curvedUV.y < 0.0 || curvedUV.y > 1.0)
    {
        sf_fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // 2. FAKE BLOOM / GLOW
    vec3 bloom      = vec3(0.0);
    vec2 texel_size = 1.0 / u_inputSize;
    bloom += texture(sf_u_texture, curvedUV + vec2(0.0, -2.0 * texel_size.y)).rgb;
    bloom += texture(sf_u_texture, curvedUV + vec2(0.0, 2.0 * texel_size.y)).rgb;
    bloom += texture(sf_u_texture, curvedUV + vec2(-2.0 * texel_size.x, 0.0)).rgb;
    bloom += texture(sf_u_texture, curvedUV + vec2(2.0 * texel_size.x, 0.0)).rgb;
    bloom = pow(bloom * 0.25, vec3(4.0));
    bloom *= u_bloomStrength;

    // 3. MAIN TEXTURE SAMPLE & GAMMA CORRECTION
    vec3 color = texture(sf_u_texture, curvedUV).rgb;
    color += bloom;
    color = pow(color, vec3(u_inputGamma));

    // 4. DYNAMIC SCANLINES
    float scanline_coord = (sf_v_texCoord.y * u_inputSize.y / u_scanlineHeight) - (u_time * u_scanlineScrollSpeed);

    // Get a 0.0-1.0 ramp for each scanline
    float scanline_profile = fract(scanline_coord);

    // Create a parabolic brightness profile (0 -> 1 -> 0) from the ramp
    float scanline_intensity = 4.0 * scanline_profile * (1.0 - scanline_profile);

    // Apply thickness by squashing the parabola. Higher thickness = thinner bright line, thicker dark line.
    scanline_intensity = pow(scanline_intensity, u_scanlineThickness);

    // Brightness modulation (same as before)
    float brightness         = dot(color, vec3(0.2, 0.7, 0.1));
    float modulated_strength = u_scanlineStrength / (1.0 + brightness * u_scanlineBrightnessModulation);

    // Combine strength and intensity to get the final dimming factor
    float scanline_dim = mix(1.0 - modulated_strength, 1.0, scanline_intensity);

    color *= scanline_dim;

    // 5. PHOSPHOR MASK (THE GRILLE)
    vec3 mask = vec3(1.0);

#ifdef MASK_APERTURE_GRILLE
    // Scale the screen coordinate by our desired mask size
    float screen_x       = gl_FragCoord.x * u_maskScale;
    int   which_phosphor = int(mod(screen_x, 3.0));
    if (which_phosphor == 0)
        mask.gb = vec2(0.0);
    if (which_phosphor == 1)
        mask.rb = vec2(0.0);
    if (which_phosphor == 2)
        mask.rg = vec2(0.0);
#endif

#ifdef MASK_SLOT
    // Scale both coordinates for the slot mask
    float screen_x = gl_FragCoord.x * u_maskScale;
    float screen_y = gl_FragCoord.y * u_maskScale;
    if (mod(screen_y, 2.0) < 1.0)
    {
        int which_phosphor = int(mod(screen_x, 3.0));
        if (which_phosphor == 0)
            mask.gb = vec2(0.0);
        if (which_phosphor == 1)
            mask.rb = vec2(0.0);
        if (which_phosphor == 2)
            mask.rg = vec2(0.0);
    }
    else
    {
        int which_phosphor = int(mod(screen_x + 1.5, 3.0));
        if (which_phosphor == 0)
            mask.gb = vec2(0.0);
        if (which_phosphor == 1)
            mask.rb = vec2(0.0);
        if (which_phosphor == 2)
            mask.rg = vec2(0.0);
    }
#endif

    color *= mix(vec3(1.0), mask, u_maskStrength);

    // 6. FINAL TOUCHES
    float vignette_coord  = length((sf_v_texCoord - 0.5) * 2.0);
    float vignette_factor = smoothstep(u_vignetteInnerRadius, u_vignetteOuterRadius, vignette_coord);
    color *= 1.0 - (vignette_factor * u_vignetteStrength);

    float noise = (rand(gl_FragCoord.xy + u_time) - 0.5) * (u_noiseStrength / 10000.0);
    color += noise;

    color = adjust_saturation(color, u_saturation);
    color = pow(color, vec3(1.0 / u_outputGamma));

    color = clamp(color, 0.0, 1.0);

    sf_fragColor = vec4(color, 1.0);
}
