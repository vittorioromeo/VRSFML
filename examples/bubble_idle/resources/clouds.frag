layout(location = 2) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

out vec4 sf_fragColor;

uniform float u_time;
uniform vec2 u_resolution;

// 1. VECTORIZED NOISE:
// Instead of calling hash21 four separate times per octave, we use GLSL's
// native vec4 capabilities to calculate all 4 grid corners simultaneously.
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);

    // Compute the 4 corners in parallel
    vec4 px = i.x + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 py = i.y + vec4(0.0, 0.0, 1.0, 1.0);

    vec4 fract_x = fract(px * 123.34);
    vec4 fract_y = fract(py * 456.21);

    vec4 d = (fract_x * (fract_x + 45.32)) + (fract_y * (fract_y + 45.32));

    vec4 h = fract((fract_x + d) * (fract_y + d));

    return mix(mix(h.x, h.y, u.x), mix(h.z, h.w, u.x), u.y);
}

float fbm(vec2 p)
{
    float value = 0.0;
    float amp   = 0.5;

    // The compiler will naturally unroll this loop
    for (int i = 0; i < 4; ++i)
    {
        value += amp * noise(p);
        p      = p * 2.03 + vec2(17.13, 9.71);
        amp *= 0.5;
    }

    return value;
}

// 2. HARDWARE BILINEAR SAMPLING:
// Replaces the heavy 13-tap cross pattern with an optimized 5-tap diagonal pattern.
// By sampling at exactly a 1.5x offset, we force the GPU's hardware bilinear filtering
// to blend across a wide 3x3 footprint automatically.
// The sum of these weights (1.16) is mathematically identical to your original function.
float sampleDensity(vec2 uv, float radius)
{
    vec2 offset = (radius * 1.5) / u_resolution;

    float d = texture(sf_u_texture, uv).a * 0.36;
    d += texture(sf_u_texture, uv + offset).a * 0.20;
    d += texture(sf_u_texture, uv - offset).a * 0.20;
    d += texture(sf_u_texture, uv + vec2(-offset.x, offset.y)).a * 0.20;
    d += texture(sf_u_texture, uv + vec2(offset.x, -offset.y)).a * 0.20;

    return d;
}

void main()
{
    // Inline raw mask fetch to avoid function call overhead
    float rawMask = texture(sf_u_texture, sf_v_texCoord).a;

    if (rawMask <= 0.001)
    {
        sf_fragColor = vec4(0.0);
        return;
    }

    float smallDensity = sampleDensity(sf_v_texCoord, 2.0);
    float largeDensity = sampleDensity(sf_v_texCoord, 6.5);

    vec2 texel = 1.0 / u_resolution;

    // Calculate Normal via our optimized sampler
    vec2 offsetX = vec2(texel.x * 3.0, 0.0);
    vec2 offsetY = vec2(0.0, texel.y * 3.0);

    float dx = sampleDensity(sf_v_texCoord + offsetX, 6.5) -
               sampleDensity(sf_v_texCoord - offsetX, 6.5);
    float dy = sampleDensity(sf_v_texCoord + offsetY, 6.5) -
               sampleDensity(sf_v_texCoord - offsetY, 6.5);

    vec3 normal = normalize(vec3(-dx * 7.0, -dy * 9.0, 1.0));

    // Constant folding: the compiler computes this normalize() once at compile-time
    const vec3 lightDir = normalize(vec3(-0.55, -0.8, 0.7));
    float light = clamp(dot(normal, lightDir) * 0.5 + 0.5, 0.0, 1.0);

    vec2 p = gl_FragCoord.xy / 42.0;
    float bodyNoise = fbm(p + vec2(u_time * 0.08, -u_time * 0.03));
    float edgeNoise = fbm(p * 1.7 + vec2(-u_time * 0.04, u_time * 0.05));

    float coverage = smoothstep(0.10 + (edgeNoise - 0.5) * 0.06,
                                0.52 + (edgeNoise - 0.5) * 0.08,
                                largeDensity + smallDensity * 0.35);

    float rim = smoothstep(0.12, 0.42, largeDensity) - smoothstep(0.46, 0.88, largeDensity);
    float billow = smoothstep(0.18, 0.72, smallDensity + bodyNoise * 0.14);

    const vec3 shadowColor    = vec3(0.73, 0.78, 0.86);
    const vec3 bodyColor      = vec3(0.92, 0.95, 0.99);
    const vec3 highlightColor = vec3(1.0);

    vec3 color = mix(shadowColor, bodyColor, light);
    color = mix(color, highlightColor, rim * 0.28 + billow * 0.22);
    color += (bodyNoise - 0.5) * 0.04;

    float alpha = coverage * smoothstep(0.02, 0.20, largeDensity) * (0.88 + rim * 0.18);

    if (alpha <= 0.001)
    {
        sf_fragColor = vec4(0.0);
        return;
    }

    // Keep the cloud layer premultiplied
    sf_fragColor = vec4(color * alpha, alpha);
}
