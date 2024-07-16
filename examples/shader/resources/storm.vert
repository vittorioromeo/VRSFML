uniform vec2 storm_position;
uniform float storm_total_radius;
uniform float storm_inner_radius;

uniform mat4 projMatrix;
uniform mat4 textMatrix;
uniform mat4 viewMatrix;

attribute vec4 color;
attribute vec2 position;
attribute vec2 texCoord;

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    vec2 newPosition = position;
    
    vec2 offset = newPosition.xy - storm_position;

    float len = length(offset);
    if (len < storm_total_radius)
    {
        float push_distance = storm_inner_radius + len / storm_total_radius * (storm_total_radius - storm_inner_radius);
        newPosition.xy = storm_position + normalize(offset) * push_distance;
    }

    gl_Position = projMatrix * viewMatrix * vec4(newPosition, 0.0, 1.0);
    v_texCoord = (textMatrix * vec4(texCoord, 0.0, 1.0)).xy;
    v_color = color;
}
