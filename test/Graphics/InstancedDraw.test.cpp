#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/DrawInstancedIndexedVerticesSettings.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/InstanceAttributeBinder.hpp"
#include "Zancle/Graphics/InstancedQuad.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/RectangleShape.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderTexture.hpp"
#include "Zancle/Graphics/Shader.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/VAOHandle.hpp"
#include "Zancle/Graphics/VBOHandle.hpp"
#include "Zancle/Graphics/View.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"


// On Emscripten, WebGL contexts cannot share resources. Zancle's
// `GLSharedContextGuard` is a no-op there. These tests verify that
// shaders, textures, and buffers created in that environment still
// function correctly (uniform uploads, texture sampling, vertex data).

TEST_CASE("[Graphics] Shared-resource rendering" * tst::skip(skipDisplayTests))
{
    auto graphicsContext = za::GraphicsContext::create().value();
    auto renderTexture   = za::RenderTexture::create({100, 100}).value();

    SECTION("Custom shader uniforms are uploaded correctly")
    {
        // Verifies that shader programs compiled via GLSharedContextGuard
        // accept uniform uploads. This broke on Emscripten when the guard
        // compiled the shader on a different WebGL context.
        constexpr auto vs = R"glsl(
layout(location = 0) uniform vec3 za_u_mvpRow0;
layout(location = 1) uniform vec3 za_u_mvpRow1;
layout(location = 3) uniform vec2 za_u_invTextureSize;

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

out vec4 v_color;

void main()
{
    gl_Position = vec4(za_a_position * 2.0, 0.0, 1.0);
    // Encode MVP row0 as color: expected (0.02, 0, -1) -> (0.51, 0.5, 0.0)
    v_color = vec4(za_u_mvpRow0 * 0.5 + 0.5, 1.0);
}
)glsl";

        constexpr auto fs = R"glsl(
in vec4 v_color;
layout(location = 0) out vec4 za_fragColor;
void main() { za_fragColor = v_color; }
)glsl";

        auto shader = za::Shader::loadFromMemory({.vertexCode = vs, .fragmentCode = fs}).value();

        za::VAOHandle vao;

        renderTexture.clear(za::Color::Black);
        renderTexture.drawInstancedIndexedVertices({.vaoHandle     = vao,
                                                    .vertexSpan    = za::instancedQuadVertices,
                                                    .indexSpan     = za::instancedQuadIndices,
                                                    .instanceCount = 1u,
                                                    .primitiveType = za::PrimitiveType::Triangles},
                                                   [](za::InstanceAttributeBinder&) {},
                                                   {.view = za::View::fromScreenSize({100.f, 100.f}), .shader = &shader});
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
        auto redImage = za::Image::create({2, 2}, za::Color::Red).value();
        auto texture  = za::Texture::loadFromImage(redImage).value();

        za::RectangleShape rect{{.position = {10.f, 10.f}, .size = {80.f, 80.f}}};

        renderTexture.clear(za::Color::Black);
        renderTexture.draw(rect, {.view = za::View::fromScreenSize({100.f, 100.f}), .texture = &texture});
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
layout(location = 0) uniform vec3 za_u_mvpRow0;
layout(location = 1) uniform vec3 za_u_mvpRow1;
layout(location = 3) uniform vec2 za_u_invTextureSize;

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

layout(location = 5) in vec2 instance_offset;
layout(location = 6) in vec4 instance_color;

out vec4 v_color;

void main()
{
    vec2 worldPos = instance_offset + (za_a_position * vec2(20.0, 20.0));

    gl_Position = vec4(dot(za_u_mvpRow0, vec3(worldPos, 1.0)),
                       dot(za_u_mvpRow1, vec3(worldPos, 1.0)),
                       0.0,
                       1.0);

    v_color = instance_color;
}
)glsl";

        constexpr auto fs = R"glsl(
in vec4 v_color;
layout(location = 0) out vec4 za_fragColor;
void main() { za_fragColor = v_color; }
)glsl";

        auto shader = za::Shader::loadFromMemory({.vertexCode = vs, .fragmentCode = fs}).value();

        struct InstanceData
        {
            za::Vec2f offset;
            za::Color color;
        };

        za::VAOHandle vaoHandle;
        za::VBOHandle instanceVBO;

        const auto drawInstance = [&](InstanceData instance)
        {
            auto setupAttribs = [&](za::InstanceAttributeBinder& binder)
            {
                binder.uploadContiguousData(instanceVBO, &instance);
                binder.setupField<&InstanceData::offset>(5);
                binder.setupField<&InstanceData::color>(6);
            };

            renderTexture.drawInstancedIndexedVertices(
                {
                    .vaoHandle     = vaoHandle,
                    .vertexSpan    = za::instancedQuadVertices,
                    .indexSpan     = za::instancedQuadIndices,
                    .instanceCount = 1u,
                    .primitiveType = za::PrimitiveType::Triangles,
                },
                setupAttribs,
                {.view = za::View::fromScreenSize({100.f, 100.f}), .shader = &shader});
        };

        renderTexture.clear(za::Color::Black);

        drawInstance({.offset = {10.f, 10.f}, .color = za::Color::Green});
        drawInstance({.offset = {60.f, 10.f}, .color = za::Color::Red});

        renderTexture.display();

        const auto image = renderTexture.getTexture().copyToImage();

        CHECK(image.getPixel({15, 15}) == za::Color::Green);
        CHECK(image.getPixel({65, 15}) == za::Color::Red);
    }

    SECTION("SOA instanced draw with separate VBOs per field")
    {
        constexpr auto vs = R"glsl(
layout(location = 0) uniform vec3 za_u_mvpRow0;
layout(location = 1) uniform vec3 za_u_mvpRow1;
layout(location = 3) uniform vec2 za_u_invTextureSize;

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

layout(location = 5) in vec2 instance_offset;
layout(location = 6) in vec4 instance_color;

out vec4 v_color;

void main()
{
    vec2 worldPos = instance_offset + (za_a_position * vec2(20.0, 20.0));

    gl_Position = vec4(dot(za_u_mvpRow0, vec3(worldPos, 1.0)),
                       dot(za_u_mvpRow1, vec3(worldPos, 1.0)),
                       0.0,
                       1.0);

    v_color = instance_color;
}
)glsl";

        constexpr auto fs = R"glsl(
in vec4 v_color;
layout(location = 0) out vec4 za_fragColor;
void main() { za_fragColor = v_color; }
)glsl";

        auto shader = za::Shader::loadFromMemory({.vertexCode = vs, .fragmentCode = fs}).value();

        za::VAOHandle vaoHandle;
        za::VBOHandle offsetVBO;
        za::VBOHandle colorVBO;

        const auto drawInstance = [&](const za::Vec2f offset, const za::Color color)
        {
            const za::Vec2f instanceOffsetData[]{offset};
            const za::Color instanceColorData[]{color};

            auto setupAttribs = [&](za::InstanceAttributeBinder& binder)
            {
                binder.uploadContiguousData(offsetVBO, instanceOffsetData);
                binder.setupFlat<za::Vec2f>(5);

                binder.uploadContiguousData(colorVBO, instanceColorData);
                binder.setupFlat<za::Color>(6);
            };

            renderTexture.drawInstancedIndexedVertices(
                {
                    .vaoHandle     = vaoHandle,
                    .vertexSpan    = za::instancedQuadVertices,
                    .indexSpan     = za::instancedQuadIndices,
                    .instanceCount = 1u,
                    .primitiveType = za::PrimitiveType::Triangles,
                },
                setupAttribs,
                {.view = za::View::fromScreenSize({100.f, 100.f}), .shader = &shader});
        };

        renderTexture.clear(za::Color::Black);

        drawInstance({10.f, 10.f}, za::Color::Green);
        drawInstance({60.f, 10.f}, za::Color::Red);

        renderTexture.display();

        const auto image = renderTexture.getTexture().copyToImage();

        CHECK(image.getPixel({15, 15}) == za::Color::Green);
        CHECK(image.getPixel({65, 15}) == za::Color::Red);
    }
}
