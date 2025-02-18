#include "SFML/Graphics/Shader.hpp"

// Other 1st party headers
#include "SFML/Graphics/GraphicsContext.hpp"

#include "SFML/System/FileInputStream.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Macros.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

namespace
{
constexpr auto vertexSource = R"glsl(

layout(location = 0) uniform mat4 sf_u_mvpMatrix;

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
    sf_v_texCoord = sf_a_texCoord;
    sf_v_color    = sf_a_color;
}

)glsl";

constexpr auto geometrySource = R"glsl(

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

layout(location = 1) uniform vec3 sf_u_texParams;

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

)glsl";

constexpr auto fragmentSource = R"glsl(

layout(location = 2) uniform sampler2D sf_u_texture;
layout(location = 6) uniform float     blink_alpha;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    vec4 pixel = sf_v_color;
    pixel.a    = blink_alpha;

    sf_fragColor = pixel;
}

)glsl";

#ifdef SFML_RUN_DISPLAY_TESTS
constexpr bool skipShaderFullTest = false;
#else
constexpr bool skipShaderFullTest = true;
#endif

} // namespace

TEST_CASE("[Graphics] sf::Shader" * doctest::skip(skipShaderFullTest))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::Shader));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Shader));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Shader));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Shader));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Shader));
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            auto       movedShader = sf::Shader::loadFromFile("Graphics/shader.vert", sf::Shader::Type::Vertex).value();
            const auto shader      = SFML_BASE_MOVE(movedShader);
            CHECK(shader.getNativeHandle() != 0);
        }

        SECTION("Assignment")
        {
            auto movedShader = sf::Shader::loadFromFile("Graphics/shader.vert", sf::Shader::Type::Vertex).value();
            auto shader      = sf::Shader::loadFromFile("Graphics/shader.frag", sf::Shader::Type::Fragment).value();
            shader           = SFML_BASE_MOVE(movedShader);
            CHECK(shader.getNativeHandle() != 0);
        }
    }

    SECTION("loadFromFile()")
    {
        SECTION("One shader")
        {
            CHECK(!sf::Shader::loadFromFile("does-not-exist.vert", sf::Shader::Type::Vertex).hasValue());

            const auto vertexShader = sf::Shader::loadFromFile("Graphics/shader.vert", sf::Shader::Type::Vertex);
            CHECK(vertexShader.hasValue());
            if (vertexShader.hasValue())
                CHECK(static_cast<bool>(vertexShader->getNativeHandle()));

            const auto fragmentShader = sf::Shader::loadFromFile("Graphics/shader.frag", sf::Shader::Type::Fragment);
            CHECK(fragmentShader.hasValue());
            if (fragmentShader.hasValue())
                CHECK(static_cast<bool>(fragmentShader->getNativeHandle()));
        }

        SECTION("Two shaders")
        {
            CHECK(!sf::Shader::loadFromFile("does-not-exist.vert", "Graphics/shader.frag").hasValue());
            CHECK(!sf::Shader::loadFromFile("Graphics/shader.vert", "does-not-exist.frag").hasValue());

            const auto shader = sf::Shader::loadFromFile("Graphics/shader.vert", "Graphics/shader.frag");
            CHECK(shader.hasValue());
            if (shader.hasValue())
                CHECK(static_cast<bool>(shader->getNativeHandle()));
        }

        SECTION("Three shaders")
        {
            CHECK(!sf::Shader::loadFromFile("does-not-exist.vert", "Graphics/shader.geom", "Graphics/shader.frag").hasValue());

            CHECK(!sf::Shader::loadFromFile("Graphics/shader.vert", "does-not-exist.geom", "Graphics/shader.frag").hasValue());

            CHECK(!sf::Shader::loadFromFile("Graphics/shader.vert", "Graphics/shader.geom", "does-not-exist.frag").hasValue());

            const auto shader = sf::Shader::loadFromFile("Graphics/shader.vert",
                                                         "Graphics/shader.geom",
                                                         "Graphics/shader.frag");

            CHECK(shader.hasValue() == sf::Shader::isGeometryAvailable());
            if (shader.hasValue())
                CHECK(static_cast<bool>(shader->getNativeHandle()) == sf::Shader::isGeometryAvailable());
        }

        SECTION("One shader with non-ASCII filename")
        {
            CHECK(sf::Shader::loadFromFile(U"Graphics/shader-ń.vert", sf::Shader::Type::Vertex).hasValue());
            CHECK(sf::Shader::loadFromFile(U"Graphics/shader-🐌.vert", sf::Shader::Type::Vertex).hasValue());
        }
    }

    SECTION("loadFromMemory()")
    {
        CHECK(sf::Shader::loadFromMemory(vertexSource, sf::Shader::Type::Vertex).hasValue());
        CHECK(sf::Shader::loadFromMemory(geometrySource, sf::Shader::Type::Geometry).hasValue() ==
              sf::Shader::isGeometryAvailable());
        CHECK(sf::Shader::loadFromMemory(fragmentSource, sf::Shader::Type::Fragment).hasValue());
        CHECK(sf::Shader::loadFromMemory(vertexSource, fragmentSource).hasValue());

        const auto shader = sf::Shader::loadFromMemory(vertexSource, geometrySource, fragmentSource);
        CHECK(shader.hasValue() == sf::Shader::isGeometryAvailable());
        if (shader.hasValue())
            CHECK(static_cast<bool>(shader->getNativeHandle()));
    }

    SECTION("loadFromStream()")
    {
        auto vertexShaderStream   = sf::FileInputStream::open("Graphics/shader.vert").value();
        auto fragmentShaderStream = sf::FileInputStream::open("Graphics/shader.frag").value();
        auto geometryShaderStream = sf::FileInputStream::open("Graphics/shader.geom").value();

        auto emptyStream = sf::FileInputStream::open("Graphics/invalid_shader.vert").value();

        SECTION("One shader")
        {
            CHECK(!sf::Shader::loadFromStream(emptyStream, sf::Shader::Type::Vertex).hasValue());
            CHECK(sf::Shader::loadFromStream(vertexShaderStream, sf::Shader::Type::Vertex).hasValue());
            CHECK(sf::Shader::loadFromStream(fragmentShaderStream, sf::Shader::Type::Fragment).hasValue());
        }

        SECTION("Two shaders")
        {
            CHECK(!sf::Shader::loadFromStream(emptyStream, fragmentShaderStream).hasValue());
            CHECK(!sf::Shader::loadFromStream(vertexShaderStream, emptyStream).hasValue());
            CHECK(sf::Shader::loadFromStream(vertexShaderStream, fragmentShaderStream).hasValue());
        }

        SECTION("Three shaders")
        {
            CHECK(!sf::Shader::loadFromStream(emptyStream, geometryShaderStream, fragmentShaderStream).hasValue());
            CHECK(!sf::Shader::loadFromStream(vertexShaderStream, emptyStream, fragmentShaderStream).hasValue());
            CHECK(!sf::Shader::loadFromStream(vertexShaderStream, geometryShaderStream, emptyStream).hasValue());

            const auto shader = sf::Shader::loadFromStream(vertexShaderStream, geometryShaderStream, fragmentShaderStream);

            CHECK(shader.hasValue() == sf::Shader::isGeometryAvailable());
            if (shader.hasValue())
                CHECK(static_cast<bool>(shader->getNativeHandle()) == sf::Shader::isGeometryAvailable());
        }
    }
}
