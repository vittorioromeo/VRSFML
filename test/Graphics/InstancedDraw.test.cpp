#include "GraphicsUtil.hpp"
#include "WindowUtil.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawInstancedIndexedVerticesSettings.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/InstanceAttributeBinder.hpp"
#include "SFML/Graphics/InstancedQuad.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/VAOHandle.hpp"
#include "SFML/Graphics/VBOHandle.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include <Doctest.hpp>


// On Emscripten, WebGL contexts cannot share resources. SFML's
// `GLSharedContextGuard` is a no-op there. These tests verify that
// shaders, textures, and buffers created in that environment still
// function correctly (uniform uploads, texture sampling, vertex data).

TEST_CASE("[Graphics] Shared-resource rendering" * doctest::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();
    auto renderTexture   = sf::RenderTexture::create({100, 100}).value();

    SECTION("Custom shader uniforms are uploaded correctly")
    {
        // Verifies that shader programs compiled via GLSharedContextGuard
        // accept uniform uploads. This broke on Emscripten when the guard
        // compiled the shader on a different WebGL context.
        constexpr auto vs = R"glsl(
layout(location = 0) uniform vec3 sf_u_mvpRow0;
layout(location = 1) uniform vec3 sf_u_mvpRow1;
layout(location = 3) uniform vec2 sf_u_invTextureSize;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

out vec4 v_color;

void main()
{
    gl_Position = vec4(sf_a_position * 2.0, 0.0, 1.0);
    // Encode MVP row0 as color: expected (0.02, 0, -1) -> (0.51, 0.5, 0.0)
    v_color = vec4(sf_u_mvpRow0 * 0.5 + 0.5, 1.0);
}
)glsl";

        constexpr auto fs = R"glsl(
in vec4 v_color;
layout(location = 0) out vec4 sf_fragColor;
void main() { sf_fragColor = v_color; }
)glsl";

        auto shader = sf::Shader::loadFromMemory({.vertexCode = vs, .fragmentCode = fs}).value();

        sf::VAOHandle vao;

        renderTexture.clear(sf::Color::Black);
        renderTexture.drawInstancedIndexedVertices({.vaoHandle     = vao,
                                                    .vertexSpan    = sf::instancedQuadVertices,
                                                    .indexSpan     = sf::instancedQuadIndices,
                                                    .instanceCount = 1u,
                                                    .primitiveType = sf::PrimitiveType::Triangles},
                                                   [](sf::InstanceAttributeBinder&) {},
                                                   {.view = sf::View::fromScreenSize({100.f, 100.f}), .shader = &shader});
        renderTexture.display();

        const auto pixel = renderTexture.getTexture().copyToImage().getPixel({50, 50});

        // b channel: -1 * 0.5 + 0.5 = 0.0 -> ~0. If uniforms are broken it's ~128.
        CHECK(pixel.r > 120u);
        CHECK(pixel.b < 10u);
    }

    SECTION("Texture created via shared context is usable for rendering")
    {
        // Textures are created on the shared context via GLSharedContextGuard.
        // Verify they can be sampled in a draw call on the main context.
        auto redImage = sf::Image::create({2, 2}, sf::Color::Red).value();
        auto texture  = sf::Texture::loadFromImage(redImage).value();

        sf::RectangleShape rect{{.position = {10.f, 10.f}, .size = {80.f, 80.f}}};

        renderTexture.clear(sf::Color::Black);
        renderTexture.draw(rect, {.view = sf::View::fromScreenSize({100.f, 100.f}), .texture = &texture});
        renderTexture.display();

        const auto pixel = renderTexture.getTexture().copyToImage().getPixel({50, 50});
        CHECK(pixel.r > 200u);
        CHECK(pixel.g < 30u);
        CHECK(pixel.b < 30u);
    }

    SECTION("VBO/EBO created via shared context deliver correct vertex data")
    {
        // VAOHandle/VBOHandle create GL buffers via GLSharedContextGuard.
        // Verify the instanced draw reads correct vertex positions and
        // per-instance data from those buffers.
        constexpr auto vs = R"glsl(
layout(location = 0) uniform vec3 sf_u_mvpRow0;
layout(location = 1) uniform vec3 sf_u_mvpRow1;
layout(location = 3) uniform vec2 sf_u_invTextureSize;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

layout(location = 5) in vec2 instance_offset;
layout(location = 6) in vec4 instance_color;

out vec4 v_color;

void main()
{
    vec2 worldPos = instance_offset + (sf_a_position * vec2(20.0, 20.0));

    gl_Position = vec4(dot(sf_u_mvpRow0, vec3(worldPos, 1.0)),
                       dot(sf_u_mvpRow1, vec3(worldPos, 1.0)),
                       0.0,
                       1.0);

    v_color = instance_color;
}
)glsl";

        constexpr auto fs = R"glsl(
in vec4 v_color;
layout(location = 0) out vec4 sf_fragColor;
void main() { sf_fragColor = v_color; }
)glsl";

        auto shader = sf::Shader::loadFromMemory({.vertexCode = vs, .fragmentCode = fs}).value();

        struct InstanceData
        {
            sf::Vec2f offset;
            sf::Color color;
        };

        sf::VAOHandle vaoHandle;
        sf::VBOHandle instanceVBO;

        const auto drawInstance = [&](InstanceData instance)
        {
            auto setupAttribs = [&](sf::InstanceAttributeBinder& binder)
            {
                binder.uploadContiguousData(instanceVBO, &instance);
                binder.setupField<&InstanceData::offset>(5);
                binder.setupField<&InstanceData::color>(6);
            };

            renderTexture.drawInstancedIndexedVertices(
                {
                    .vaoHandle     = vaoHandle,
                    .vertexSpan    = sf::instancedQuadVertices,
                    .indexSpan     = sf::instancedQuadIndices,
                    .instanceCount = 1u,
                    .primitiveType = sf::PrimitiveType::Triangles,
                },
                setupAttribs,
                {.view = sf::View::fromScreenSize({100.f, 100.f}), .shader = &shader});
        };

        renderTexture.clear(sf::Color::Black);

        drawInstance({.offset = {10.f, 10.f}, .color = sf::Color::Green});
        drawInstance({.offset = {60.f, 10.f}, .color = sf::Color::Red});

        renderTexture.display();

        const auto image = renderTexture.getTexture().copyToImage();

        CHECK(image.getPixel({15, 15}) == sf::Color::Green);
        CHECK(image.getPixel({65, 15}) == sf::Color::Red);
    }

    SECTION("SOA instanced draw with separate VBOs per field")
    {
        constexpr auto vs = R"glsl(
layout(location = 0) uniform vec3 sf_u_mvpRow0;
layout(location = 1) uniform vec3 sf_u_mvpRow1;
layout(location = 3) uniform vec2 sf_u_invTextureSize;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

layout(location = 5) in vec2 instance_offset;
layout(location = 6) in vec4 instance_color;

out vec4 v_color;

void main()
{
    vec2 worldPos = instance_offset + (sf_a_position * vec2(20.0, 20.0));

    gl_Position = vec4(dot(sf_u_mvpRow0, vec3(worldPos, 1.0)),
                       dot(sf_u_mvpRow1, vec3(worldPos, 1.0)),
                       0.0,
                       1.0);

    v_color = instance_color;
}
)glsl";

        constexpr auto fs = R"glsl(
in vec4 v_color;
layout(location = 0) out vec4 sf_fragColor;
void main() { sf_fragColor = v_color; }
)glsl";

        auto shader = sf::Shader::loadFromMemory({.vertexCode = vs, .fragmentCode = fs}).value();

        sf::VAOHandle vaoHandle;
        sf::VBOHandle offsetVBO;
        sf::VBOHandle colorVBO;

        const auto drawInstance = [&](const sf::Vec2f offset, const sf::Color color)
        {
            const sf::Vec2f instanceOffsetData[]{offset};
            const sf::Color instanceColorData[]{color};

            auto setupAttribs = [&](sf::InstanceAttributeBinder& binder)
            {
                binder.uploadContiguousData(offsetVBO, instanceOffsetData);
                binder.setupFlat<sf::Vec2f>(5);

                binder.uploadContiguousData(colorVBO, instanceColorData);
                binder.setupFlat<sf::Color>(6);
            };

            renderTexture.drawInstancedIndexedVertices(
                {
                    .vaoHandle     = vaoHandle,
                    .vertexSpan    = sf::instancedQuadVertices,
                    .indexSpan     = sf::instancedQuadIndices,
                    .instanceCount = 1u,
                    .primitiveType = sf::PrimitiveType::Triangles,
                },
                setupAttribs,
                {.view = sf::View::fromScreenSize({100.f, 100.f}), .shader = &shader});
        };

        renderTexture.clear(sf::Color::Black);

        drawInstance({10.f, 10.f}, sf::Color::Green);
        drawInstance({60.f, 10.f}, sf::Color::Red);

        renderTexture.display();

        const auto image = renderTexture.getTexture().copyToImage();

        CHECK(image.getPixel({15, 15}) == sf::Color::Green);
        CHECK(image.getPixel({65, 15}) == sf::Color::Red);
    }
}
