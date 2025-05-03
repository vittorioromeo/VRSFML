layout(location = 1) uniform sampler2D sf_u_texture;
// uniform float blurStrength;   // Controls the Gaussian blur standard deviation.
// uniform float shadowOpacity;  // Overall shadow opacity multiplier (0.0 to 1.0).

////////////////////////////////////////////////////////////
in vec4 sf_v_color;
in vec2 sf_v_texCoord;
in vec2 v_worldPos;

out vec4 sf_fragColor;

////////////////////////////////////////////////////////////
void main()
{
    const float blurStrength  = 25.0;
    const float shadowOpacity = 0.25;

    vec2 texSize = vec2(textureSize(sf_u_texture, 0));
    vec2 uv      = sf_v_texCoord;

    vec4  sum         = vec4(0.0);
    float totalWeight = 0.0;

    // 3x3 Gaussian blur kernel.
    for (int x = -3; x <= 3; ++x)
    {
        for (int y = -3; y <= 3; ++y)
        {
            vec2  offset = vec2(float(x) * 3.0, float(y) * 3.0) / texSize;
            float weight = exp(-float(x * x + y * y) / (2.0 * blurStrength * blurStrength));
            sum += texture2D(sf_u_texture, uv + offset) * weight;
            totalWeight += weight;
        }
    }

    // Compute the blurred alpha.
    vec4 blurred = sum / totalWeight;

    // Output black color with blurred alpha, modulated by shadowOpacity.
    sf_fragColor = vec4(0.0, 0.0, 0.0, blurred.a * shadowOpacity);
}
