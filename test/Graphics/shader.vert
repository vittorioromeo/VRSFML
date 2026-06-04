layout(location = 0) uniform vec3 za_u_mvpRow0;
layout(location = 1) uniform vec3 za_u_mvpRow1;
layout(location = 3) uniform vec2 za_u_invTextureSize;

layout(location = 4) uniform vec2 storm_position;
layout(location = 5) uniform float storm_total_radius;
layout(location = 6) uniform float storm_inner_radius;

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

out vec4 za_v_color;
out vec2 za_v_texCoord;

void main()
{
    vec2 newPosition = za_a_position;

    vec2 offset = newPosition.xy - storm_position;

    float len = length(offset);
    if (len < storm_total_radius)
    {
        float push_distance = storm_inner_radius + len / storm_total_radius * (storm_total_radius - storm_inner_radius);
        newPosition.xy      = storm_position + normalize(offset) * push_distance;
    }

    gl_Position   = vec4(dot(za_u_mvpRow0, vec3(newPosition, 1.0)), dot(za_u_mvpRow1, vec3(newPosition, 1.0)), 0.0, 1.0);
    za_v_texCoord = za_a_texCoord * za_u_invTextureSize;
    za_v_color    = za_a_color;
}
