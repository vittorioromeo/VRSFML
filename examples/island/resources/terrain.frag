uniform float lightFactor;

layout(location = 2) uniform sampler2D za_u_texture;

in vec4 za_v_color;
in vec2 za_v_texCoord;
in vec3 normal;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    vec3  lightPosition = vec3(-1.0, 1.0, 1.0);
    vec3  eyePosition   = vec3(0.0, 0.0, 1.0);
    vec3  halfVector    = normalize(lightPosition + eyePosition);
    float intensity     = lightFactor + (1.0 - lightFactor) * dot(normalize(normal), normalize(halfVector));
    za_fragColor        = za_v_color * vec4(intensity, intensity, intensity, 1.0);
}
