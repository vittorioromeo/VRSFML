layout(location = 0) uniform mat4 sf_u_mvpMatrix;
layout(location = 1) uniform vec3 sf_u_texParams;

layout(location = 3) uniform vec2 storm_position;
layout(location = 4) uniform float storm_total_radius;
layout(location = 5) uniform float storm_inner_radius;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 sf_v_color;
out vec2 sf_v_texCoord;

void main()
{
    vec2 newPosition = sf_a_position;

    vec2 offset = newPosition.xy - storm_position;

    float len = length(offset);
    if (len < storm_total_radius)
    {
        float push_distance = storm_inner_radius + len / storm_total_radius * (storm_total_radius - storm_inner_radius);
        newPosition.xy      = storm_position + normalize(offset) * push_distance;
    }

    gl_Position   = sf_u_mvpMatrix * vec4(newPosition, 0.0, 1.0);
    sf_v_texCoord = vec2(sf_u_texParams[0] * sf_a_texCoord.x, 
                         sf_u_texParams[1] * sf_a_texCoord.y + sf_u_texParams[2]);
    sf_v_color    = sf_a_color;
}
