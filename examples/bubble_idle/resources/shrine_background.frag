layout(location = 2) uniform sampler2D sf_u_texture;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;
in vec2 v_worldPos;

out vec4 sf_fragColor;

uniform float u_time;
uniform vec2 u_shrineCenter;
uniform float u_shrineRange;
uniform float u_shrineTintR;
uniform float u_shrineTintG;
uniform float u_shrineTintB;
uniform float u_shrineTintA;
uniform float u_distortionStrength;
uniform float u_tintStrength;
uniform float u_effectStrength;

void main()
{
    vec2 diff = v_worldPos - u_shrineCenter;
    float dist = length(diff);

    if (dist >= u_shrineRange)
        discard;

    float normalizedDist = dist / max(u_shrineRange, 0.001);
    float radialMask = 1.0 - smoothstep(0.2, 1.0, normalizedDist);

    vec2 dir = dist > 0.001 ? diff / dist : vec2(0.0, 1.0);
    vec2 tangent = vec2(-dir.y, dir.x);
    float effectStrength = clamp(u_effectStrength, 0.0, 1.0);

    float swirl = sin(normalizedDist * 18.0 - u_time * 3.1) * 0.5 + 0.5;
    float ripple0 = sin(v_worldPos.y * 0.06 + u_time * 2.3);
    float ripple1 = cos(v_worldPos.x * 0.05 - u_time * 1.9);
    float ripple2 = sin((v_worldPos.x + v_worldPos.y) * 0.04 + u_time * 2.7);

    vec2 texel = 1.0 / vec2(textureSize(sf_u_texture, 0));
    vec2 distortionDir = dir * (0.65 + 0.35 * ripple0) + tangent * (ripple1 + swirl * 1.35);
    vec2 distortion = distortionDir * (u_distortionStrength * texel * radialMask * effectStrength);
    vec2 shimmerOffset = tangent * texel * (6.0 + 10.0 * swirl) * radialMask * effectStrength;

    vec4 baseColor = texture(sf_u_texture, sf_v_texCoord);
    vec2 uv0 = clamp(sf_v_texCoord + distortion, vec2(0.0), vec2(1.0));
    vec2 uv1 = clamp(sf_v_texCoord + distortion * 1.45 + shimmerOffset, vec2(0.0), vec2(1.0));
    vec2 uv2 = clamp(sf_v_texCoord - distortion * 1.15 - shimmerOffset * 0.75, vec2(0.0), vec2(1.0));

    vec3 distortedColor = vec3(texture(sf_u_texture, uv1).r,
                               texture(sf_u_texture, uv0).g,
                               texture(sf_u_texture, uv2).b);

    vec3 tint = vec3(u_shrineTintR, u_shrineTintG, u_shrineTintB);
    float tintMask = radialMask * effectStrength * (0.55 + 0.45 * swirl);
    float phaseBand = 0.5 + 0.5 * sin(normalizedDist * 24.0 - u_time * 5.2 + ripple2 * 2.0);
    float shimmer = 0.5 + 0.5 * sin(u_time * 6.0 + v_worldPos.x * 0.08 - v_worldPos.y * 0.05);

    vec3 phasedColor = mix(baseColor.rgb, distortedColor, 0.85);
    phasedColor += tint * (0.1 + 0.22 * phaseBand) * tintMask;
    phasedColor = mix(phasedColor, phasedColor * 0.45 + tint * 1.9, u_tintStrength * tintMask);
    phasedColor += tint * 0.08 * shimmer * radialMask;

    sf_fragColor = mix(vec4(tint, u_shrineTintA), vec4(phasedColor, radialMask * effectStrength * 0.95), 0.75);
}
