// The render target's resolution (used for scaling)
layout(location = 7) uniform vec2 resolution;

// The billboards' size
layout(location = 8) uniform vec2 size;

// Input is the passed point cloud
layout(points) in;

// The output will consist of triangle strips with four vertices each
layout(triangle_strip, max_vertices = 4) out;

// Output texture coordinates
out vec2 sf_v_texCoord;

out vec4 sf_v_color; // Pass through to fragment

// Main entry point
void main()
{
    // Calculate the half width/height of the billboards
    vec2 half_size = size / 2.f;

    // Scale the size based on resolution (1 would be full width/height)
    half_size /= resolution;

    // Iterate over all vertices
    for (int i = 0; i < gl_in.length(); ++i)
    {
        // Retrieve the passed vertex position
        vec2 pos = gl_in[i].gl_Position.xy;

        // Bottom left vertex
        gl_Position   = vec4(pos - half_size, 0.f, 1.f);
        sf_v_texCoord = vec2(1.f, 1.f);
        EmitVertex();

        // Bottom right vertex
        gl_Position   = vec4(pos.x + half_size.x, pos.y - half_size.y, 0.f, 1.f);
        sf_v_texCoord = vec2(0.f, 1.f);
        EmitVertex();

        // Top left vertex
        gl_Position   = vec4(pos.x - half_size.x, pos.y + half_size.y, 0.f, 1.f);
        sf_v_texCoord = vec2(1.f, 0.f);
        EmitVertex();

        // Top right vertex
        gl_Position   = vec4(pos + half_size, 0.f, 1.f);
        sf_v_texCoord = vec2(0.f, 0.f);
        EmitVertex();

        // And finalize the primitive
        EndPrimitive();
    }
}
