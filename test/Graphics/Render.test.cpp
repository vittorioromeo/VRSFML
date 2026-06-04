#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"
#include "Zancle/GLUtils/GLCheck.hpp"
#include "Zancle/GLUtils/Glad.hpp"
#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/DrawInstancedIndexedVerticesSettings.hpp"
#include "Zancle/Graphics/DrawableBatch.hpp"
#include "Zancle/Graphics/Glsl.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/Image.hpp"
#include "Zancle/Graphics/InstanceAttributeBinder.hpp"
#include "Zancle/Graphics/InstancedQuad.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/RectangleShape.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderTexture.hpp"
#include "Zancle/Graphics/Shader.hpp"
#include "Zancle/Graphics/StencilMode.hpp"
#include "Zancle/Graphics/Texture.hpp"
#include "Zancle/Graphics/VAOHandle.hpp"
#include "Zancle/Graphics/VBOHandle.hpp"
#include "Zancle/Graphics/Vertex.hpp"
#include "Zancle/Graphics/VertexBuffer.hpp"
#include "Zancle/Graphics/View.hpp"
#include "Zancle/System/Err.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"
#include "Zancle/System/Thread.hpp"
#include "Zancle/Window/ContextSettings.hpp"
#include "Zancle/Window/WindowContext.hpp"


// Mirrors the trick in test/Window/Context.test.cpp: forces visibility of
// `priv::GlContext`'s protected `getId()` so a test-only `TestContext`
// helper can spin up additional GL contexts and switch between them.
// `WindowContext` already declares `friend TestContext;` (a forward-decl in
// the global namespace) for `createGlContext` access.
#define protected public
#include "../src/Zancle/GLUtils/GlContext.hpp"
#undef protected


////////////////////////////////////////////////////////////
// Test-only helper that owns a fresh GL context distinct from the shared
// graphics context. Used to build true multi-context regression coverage.
struct TestContext
{
    decltype(za::WindowContext::createGlContext(za::ContextSettings{})) glContext;

    TestContext() : glContext(za::WindowContext::createGlContext(za::ContextSettings{}))
    {
        if (!za::WindowContext::setActiveThreadLocalGlContext(*glContext, true))
            za::priv::errMsg("Failed to activate TestContext on construction");
    }

    ~TestContext()
    {
        if (glContext != nullptr && !za::WindowContext::setActiveThreadLocalGlContext(*glContext, false))
            za::priv::errMsg("Failed to deactivate TestContext on destruction");
    }

    TestContext(const TestContext&)                = delete;
    TestContext& operator=(const TestContext&)     = delete;
    TestContext(TestContext&&) noexcept            = default;
    TestContext& operator=(TestContext&&) noexcept = default;

    [[nodiscard]] bool setActive(bool active) const
    {
        return za::WindowContext::setActiveThreadLocalGlContext(*glContext, active);
    }

    [[nodiscard]] unsigned int getId() const
    {
        return glContext->getId();
    }
};


namespace
{
constexpr auto blinkAlphaFragSource = R"glsl(
layout(location = 2) uniform sampler2D za_u_texture;
layout(location = 7) uniform float blink_alpha;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    za_fragColor = za_v_color * blink_alpha;
}
)glsl";

constexpr auto instancedVertexSource = R"glsl(
layout(location = 0) uniform vec3 za_u_mvpRow0;
layout(location = 1) uniform vec3 za_u_mvpRow1;
layout(location = 3) uniform vec2 za_u_invTextureSize;

layout(location = 0) in vec2 za_a_position;
layout(location = 1) in vec4 za_a_color;
layout(location = 2) in vec2 za_a_texCoord;

layout(location = 3) in vec2 instance_offset;
layout(location = 4) in vec4 instance_color;

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

constexpr auto instancedFragmentSource = R"glsl(
in vec4 v_color;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    za_fragColor = v_color;
}
)glsl";

} // namespace


TEST_CASE("[Graphics] Render Tests" * tst::skip(skipDisplayTests))
{
    auto graphicsContext = za::GraphicsContext::create().value();

    SECTION("Stencil Tests")
    {
        auto renderTexture = za::RenderTexture::create({100, 100}, {.depthBits = 0, .stencilBits = 8}).value();

        renderTexture.clear(za::Color::Red, za::StencilValue{127u});

        const za::RectangleShape shape1{{.fillColor = za::Color::Green, .size = {100.f, 100.f}}};
        const za::RectangleShape shape2{{.fillColor = za::Color::Blue, .size = {100.f, 100.f}}};

        SECTION("Stencil-Only")
        {
            renderTexture.draw(shape1,
                               za::RenderStates{.stencilMode = {
                                                    .stencilComparison      = za::StencilComparison::Always,
                                                    .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                    .stencilOnly            = true,
                                                    .stencilReference       = za::StencilValue{1u},
                                                    .stencilMask            = za::StencilValue{0xFFu},
                                                }});
            renderTexture.display();
            CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
        }

        SECTION("Comparisons")
        {
            SECTION("Always")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{1u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Equal")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{126u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);

                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Greater")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Greater,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{126u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Greater,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Greater,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{128u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("GreaterEqual")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::GreaterEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{126u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::GreaterEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
                renderTexture.draw(shape2,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::GreaterEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{128u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();

                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Blue);
            }

            SECTION("Less")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Less,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{128u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Less,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Less,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{126u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("LessEqual")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::LessEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{128u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::LessEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
                renderTexture.draw(shape2,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::LessEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{126u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();

                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Blue);
            }

            SECTION("Never")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Never,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
            }

            SECTION("NotEqual")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::NotEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Red);
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::NotEqual,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{128u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }
        }

        SECTION("Updating")
        {
            SECTION("Decrement")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Decrement,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Decrement,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{126u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Increment")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Increment,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Increment,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{128u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Invert")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Invert,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Invert,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{0x80u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Keep")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Replace")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = za::StencilValue{255u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Replace,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{255u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }

            SECTION("Zero")
            {
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Always,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Zero,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = za::StencilValue{127u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   za::RenderStates{.stencilMode = {
                                                        .stencilComparison      = za::StencilComparison::Equal,
                                                        .stencilUpdateOperation = za::StencilUpdateOperation::Zero,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = za::StencilValue{0u},
                                                        .stencilMask            = za::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == za::Color::Green);
            }
        }
    }

    SECTION("Auto-batch generation counter tests")
    {
        auto renderTexture = za::RenderTexture::create({100, 100}).value();

        const za::RectangleShape shape{{.fillColor = za::Color::Red, .size = {100.f, 100.f}}};

        SECTION("Shader uniform mutation breaks batch")
        {
            auto shader = za::Shader::loadFromMemory({.fragmentCode = blinkAlphaFragSource}).value();

            const auto loc = shader.getUniformLocation("blink_alpha").value();
            shader.setUniform(loc, 1.f);

            const za::RenderStates states{.shader = &shader};

            // Two draws without mutation: single batch, 1 draw call
            renderTexture.clear();
            renderTexture.draw(shape, states);
            renderTexture.draw(shape, states);
            CHECK(renderTexture.display().drawCalls == 1u);

            // Two draws with uniform mutation in between: batch broken, 2 draw calls
            renderTexture.clear();
            renderTexture.draw(shape, states);
            shader.setUniform(loc, 0.5f);
            renderTexture.draw(shape, states);
            CHECK(renderTexture.display().drawCalls == 2u);
        }

        SECTION("Manual flush before uniform mutation preserves correct rendering")
        {
            auto shader = za::Shader::loadFromMemory({.fragmentCode = blinkAlphaFragSource}).value();

            const auto loc = shader.getUniformLocation("blink_alpha").value();

            const za::RenderStates states{.shader = &shader};

            // Correct order: flush, then mutate, then draw
            // The flush drains the pending batch with the OLD uniform value,
            // so each batch sees its own intended uniform.
            renderTexture.clear();
            shader.setUniform(loc, 1.f);
            renderTexture.draw(shape, states); // batch 1: alpha=1
            renderTexture.flush();             // drain batch 1 with alpha=1
            shader.setUniform(loc, 0.5f);      // now safe to mutate
            renderTexture.draw(shape, states); // batch 2: alpha=0.5
            CHECK(renderTexture.display().drawCalls == 2u);

            // Wrong order: mutate, then flush -- the pending batch is
            // drawn with the ALREADY-MUTATED uniform (corruption).
            // The generation counter must detect this and break the batch.
            renderTexture.clear();
            shader.setUniform(loc, 1.f);
            renderTexture.draw(shape, states); // batch 1: intended alpha=1
            shader.setUniform(loc, 0.5f);      // oops -- mutated before flush
            renderTexture.flush();             // draws batch 1 with alpha=0.5 (wrong!)
            renderTexture.draw(shape, states); // batch 2: alpha=0.5
            CHECK(renderTexture.display().drawCalls == 2u);
        }

        SECTION("Flush before display is redundant (display already flushes)")
        {
            auto shader = za::Shader::loadFromMemory({.fragmentCode = blinkAlphaFragSource}).value();

            const auto loc = shader.getUniformLocation("blink_alpha").value();

            const za::RenderStates states{.shader = &shader};

            // With redundant flush before display: uniform set before draw,
            // then flush (no-op since display would do it), then display.
            renderTexture.clear();
            shader.setUniform(loc, 1.f);
            renderTexture.draw(shape, states);
            renderTexture.flush();
            CHECK(renderTexture.display().drawCalls == 1u);

            // Without flush: display handles it identically.
            renderTexture.clear();
            shader.setUniform(loc, 0.5f);
            renderTexture.draw(shape, states);
            CHECK(renderTexture.display().drawCalls == 1u);
        }

        SECTION("Repeated instanced draws keep base vertex attributes bound to the shared quad VBO")
        {
            struct InstanceData
            {
                za::Vec2f offset;
                za::Color color;
            };

            auto shader = za::Shader::loadFromMemory(
                              {.vertexCode = instancedVertexSource, .fragmentCode = instancedFragmentSource})
                              .value();

            za::VAOHandle vaoHandle;
            za::VBOHandle instanceVBO;

            const auto drawInstance = [&](InstanceData instance)
            {
                auto setupAttribs = [&](za::InstanceAttributeBinder& binder)
                {
                    binder.uploadContiguousData(instanceVBO, &instance);
                    binder.setupField<&InstanceData::offset>(3);
                    binder.setupField<&InstanceData::color>(4);
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
            auto shader = za::Shader::loadFromMemory(
                              {.vertexCode = instancedVertexSource, .fragmentCode = instancedFragmentSource})
                              .value();

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
                    binder.setupFlat<za::Vec2f>(3);

                    binder.uploadContiguousData(colorVBO, instanceColorData);
                    binder.setupFlat<za::Color>(4);
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

        SECTION("Mixed instanced and immediate draws preserve GL_ARRAY_BUFFER for the active VAO")
        {
            // Regression: `setupDraw`'s non-rebind branch (taken for
            // consecutive same-VAO draws when the cache is enabled) must
            // ensure GL_ARRAY_BUFFER is bound to the active VAO's shared
            // VBO before `streamVerticesToGPU` runs. The instanced binder
            // path leaves a per-instance VBO bound; without proper
            // restoration, the next same-VAO draw's `glBufferData` would
            // overwrite the per-instance VBO storage instead of the shared
            // mesh VBO, corrupting the next draw and (on desktop where
            // per-instance VBOs are persistently mapped) tripping
            // `GL_INVALID_OPERATION`.
            //
            // The test stresses transitions across all interesting
            // cache-state combinations:
            //   instanced -> instanced (same VAO, non-rebind branch)
            //   instanced -> immediate (different VAO, rebind branch)
            //   immediate -> instanced (different VAO, rebind branch)
            auto rt = za::RenderTexture::create({100, 100}).value();

            auto shader = za::Shader::loadFromMemory(
                              {.vertexCode = instancedVertexSource, .fragmentCode = instancedFragmentSource})
                              .value();

            za::VAOHandle vaoHandle;
            za::VBOHandle instanceVBO;

            struct InstanceData
            {
                za::Vec2f offset;
                za::Color color;
            };

            const auto drawInstanced = [&](za::Vec2f offset, za::Color color)
            {
                const InstanceData instance{offset, color};

                auto setupAttribs = [&](za::InstanceAttributeBinder& binder)
                {
                    binder.uploadContiguousData(instanceVBO, &instance);
                    binder.setupField<&InstanceData::offset>(3);
                    binder.setupField<&InstanceData::color>(4);
                };

                rt.drawInstancedIndexedVertices(
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

            rt.clear(za::Color::Black);

            // Two consecutive instanced draws on the same VAOHandle. After
            // the first, GL_ARRAY_BUFFER points at the instance VBO; the
            // second must rebind the VAO's shared mesh VBO before
            // `streamVerticesToGPU` runs.
            drawInstanced({15.f, 15.f}, za::Color::Green);
            drawInstanced({40.f, 15.f}, za::Color::Red);

            // Immediate non-instanced draw (different VAO -> rebind branch).
            rt.draw(za::RectangleShape{{.position = {55.f, 5.f}, .fillColor = za::Color::Blue, .size = {20.f, 20.f}}});

            // Back to instanced (different VAO -> rebind branch).
            drawInstanced({15.f, 40.f}, za::Color::Yellow);

            // Another consecutive instanced draw (same VAO -> non-rebind
            // branch must once again restore GL_ARRAY_BUFFER).
            drawInstanced({40.f, 40.f}, za::Color::Magenta);

            // Final transitions: instanced -> immediate -> instanced again.
            rt.draw(za::RectangleShape{{.position = {55.f, 30.f}, .fillColor = za::Color::Cyan, .size = {20.f, 20.f}}});
            drawInstanced({15.f, 65.f}, za::Color{255u, 128u, 0u, 255u}); // orange

            rt.display();

            const auto img = rt.getTexture().copyToImage();
            CHECK(img.getPixel({15, 15}) == za::Color::Green);
            CHECK(img.getPixel({40, 15}) == za::Color::Red);
            CHECK(img.getPixel({65, 15}) == za::Color::Blue);
            CHECK(img.getPixel({15, 40}) == za::Color::Yellow);
            CHECK(img.getPixel({40, 40}) == za::Color::Magenta);
            CHECK(img.getPixel({65, 40}) == za::Color::Cyan);
            CHECK(img.getPixel({15, 65}) == za::Color{255u, 128u, 0u, 255u});
        }

#ifndef ZA_OPENGL_ES
        SECTION("Persistent GPU batches can reserve before clear when reused across frames")
        {
            auto batchRenderTexture = za::RenderTexture::create({100, 100}).value();

            za::PersistentGPUDrawableBatch batch;
            const za::RectangleShape rect{{.position = {10.f, 10.f}, .fillColor = za::Color::Green, .size = {30.f, 30.f}}};

            batchRenderTexture.clear(za::Color::Black);
            batch.add(rect);
            batchRenderTexture.draw(batch);
            batchRenderTexture.display();

            glCheck(glFinish());

            // Mirrors the examples: reserve for the next frame before the
            // batch is cleared and refilled.
            batch.reserveQuads(64u);
            batch.clear();
            batch.add(rect);

            batchRenderTexture.clear(za::Color::Black);
            batchRenderTexture.draw(batch);
            batchRenderTexture.display();

            const auto image = batchRenderTexture.getTexture().copyToImage();
            CHECK(image.getPixel({20, 20}) == za::Color::Green);
        }

        SECTION("Persistent GPU batches can be refilled from a worker thread after clear")
        {
            auto batchRenderTexture = za::RenderTexture::create({100, 100}).value();

            za::PersistentGPUDrawableBatch batch;
            const za::RectangleShape rect{{.position = {10.f, 10.f}, .fillColor = za::Color::Green, .size = {30.f, 30.f}}};

            batch.add(rect);

            batchRenderTexture.clear(za::Color::Black);
            batchRenderTexture.draw(batch);
            batchRenderTexture.display();

            batch.clear();

            za::Thread worker{[&batch, &rect] { batch.add(rect); }};
            worker.join();

            batchRenderTexture.clear(za::Color::Black);
            batchRenderTexture.draw(batch);
            batchRenderTexture.display();

            const auto image = batchRenderTexture.getTexture().copyToImage();
            CHECK(image.getPixel({20, 20}) == za::Color::Green);
        }
#endif
    }

    SECTION("State-cache regression tests")
    {
        // Custom shader that emits the value of a single uniform as the fragment
        // color. Decouples the rendered color from the geometry so we can prove
        // which `setUniform` value the right program actually received.
        constexpr auto solidColorFragSource = R"glsl(
layout(location = 7) uniform vec4 u_color;

in vec4 za_v_color;
in vec2 za_v_texCoord;

layout(location = 0) out vec4 za_fragColor;

void main()
{
    za_fragColor = u_color;
}
)glsl";

        SECTION("resetGLStates reapplies viewport after raw GL viewport mutation")
        {
            // Regression: `resetGLStatesImpl` previously did not reapply
            // `cache.lastView`, so a raw `glViewport` between draws (or an
            // ImGui backend) could leave the GL viewport mismatched with what
            // SFML's cache claims is bound. The next draw with the same view
            // would skip `applyView` (cache hit on `lastView`) and render at
            // the wrong viewport.
            auto rt = za::RenderTexture::create({100, 100}).value();

            const za::RectangleShape leftRect{
                {.position = {10.f, 10.f}, .fillColor = za::Color::Green, .size = {20.f, 20.f}}};
            const za::RectangleShape rightRect{
                {.position = {60.f, 60.f}, .fillColor = za::Color::Red, .size = {20.f, 20.f}}};

            rt.clear(za::Color::Black);
            rt.draw(leftRect);
            rt.flush(); // commit the draw so cache.lastView is set on the live view

            // Mutate the GL viewport behind SFML's back. Without the fix, the
            // next `resetGLStates` + draw will inherit this 50x50 viewport.
            glCheck(glViewport(0, 0, 50, 50));

            rt.resetGLStates();

            // Same view as before -- without the fix, `applyView` is skipped
            // because `lastView == usedView`, so the rightRect would land
            // outside the (still-50x50) viewport and not render.
            rt.draw(rightRect);
            rt.display();

            const auto img = rt.getTexture().copyToImage();
            CHECK(img.getPixel({15, 15}) == za::Color::Green);
            CHECK(img.getPixel({65, 65}) == za::Color::Red);
        }

        SECTION("setUniform applies to the right shader when many shaders interleave")
        {
            // Regression: `Shader::UniformBinder` now reads the bound program
            // from a thread_local cache instead of querying GL. Stress the
            // cache by mutating uniforms on multiple shaders interleaved.
            auto shaderA = za::Shader::loadFromMemory({.fragmentCode = solidColorFragSource}).value();
            auto shaderB = za::Shader::loadFromMemory({.fragmentCode = solidColorFragSource}).value();
            auto shaderC = za::Shader::loadFromMemory({.fragmentCode = solidColorFragSource}).value();

            const auto locA = shaderA.getUniformLocation("u_color").value();
            const auto locB = shaderB.getUniformLocation("u_color").value();
            const auto locC = shaderC.getUniformLocation("u_color").value();

            // Many interleaved mutations -- each `setUniform` constructs a
            // `UniformBinder` that reads from / writes to the program cache.
            for (int i = 0; i < 8; ++i)
            {
                shaderA.setUniform(locA, za::Glsl::Vec4{1.f, 0.f, 0.f, 1.f}); // red
                shaderB.setUniform(locB, za::Glsl::Vec4{0.f, 1.f, 0.f, 1.f}); // green
                shaderC.setUniform(locC, za::Glsl::Vec4{0.f, 0.f, 1.f, 1.f}); // blue
            }

            auto                     rt = za::RenderTexture::create({60, 20}).value();
            const za::RectangleShape full{{.size = {20.f, 20.f}}};

            rt.clear(za::Color::Black);
            rt.draw(full, za::RenderStates{.shader = &shaderA});
            rt.draw(za::RectangleShape{{.position = {20.f, 0.f}, .size = {20.f, 20.f}}},
                    za::RenderStates{.shader = &shaderB});
            rt.draw(za::RectangleShape{{.position = {40.f, 0.f}, .size = {20.f, 20.f}}},
                    za::RenderStates{.shader = &shaderC});
            rt.display();

            const auto img = rt.getTexture().copyToImage();
            CHECK(img.getPixel({10, 10}) == za::Color::Red);
            CHECK(img.getPixel({30, 10}) == za::Color::Green);
            CHECK(img.getPixel({50, 10}) == za::Color::Blue);
        }

        SECTION("Destroying a shader does not poison the program cache")
        {
            // Regression: `destroyProgramIfNeeded` clears the cache if the
            // destroyed program was the cached one. Without this, GL handle
            // reuse could lead `useProgram(reusedHandle)` to skip the bind
            // on a cache hit and leave the wrong (deleted) program current.
            auto                     rt = za::RenderTexture::create({40, 40}).value();
            const za::RectangleShape full{{.size = {40.f, 40.f}}};

            {
                auto shaderTemp = za::Shader::loadFromMemory({.fragmentCode = solidColorFragSource}).value();
                shaderTemp.setUniform(shaderTemp.getUniformLocation("u_color").value(), za::Glsl::Vec4{1.f, 0.f, 0.f, 1.f});

                rt.clear(za::Color::Black);
                rt.draw(full, za::RenderStates{.shader = &shaderTemp});
                rt.display();
                // shaderTemp's program is now in the cache as the last bound program.
            }
            // shaderTemp destroyed -- cache must be invalidated for that handle.

            // A new shader may receive the same GL handle; either way, the
            // cache must not skip the rebind.
            auto shaderNew = za::Shader::loadFromMemory({.fragmentCode = solidColorFragSource}).value();
            shaderNew.setUniform(shaderNew.getUniformLocation("u_color").value(), za::Glsl::Vec4{0.f, 1.f, 0.f, 1.f});

            rt.clear(za::Color::Black);
            rt.draw(full, za::RenderStates{.shader = &shaderNew});
            rt.display();

            CHECK(rt.getTexture().copyToImage().getPixel({20, 20}) == za::Color::Green);
        }

#ifndef ZA_SYSTEM_EMSCRIPTEN
        // Emscripten/WebGL does not support multiple GL contexts.
        SECTION("Shader cache is invalidated when active GL context changes")
        {
            // Regression: `readCurrentProgramOrQuery` uses `currentProgramCacheContextId`
            // to detect when the cache was populated under a different GL
            // context, falling back to a query in that case.
            //
            // To exercise this, spin up an explicit secondary GL context so
            // the active-context id genuinely changes between calls.
            // RenderTextures alone don't trigger it because they share the
            // active context rather than carrying their own.
            auto rt = za::RenderTexture::create({40, 40}).value();

            auto       shader = za::Shader::loadFromMemory({.fragmentCode = solidColorFragSource}).value();
            const auto loc    = shader.getUniformLocation("u_color").value();

            const za::RectangleShape full{{.size = {40.f, 40.f}}};

            // 1) Prime the cache on the shared graphics context by drawing
            //    once. After this, `currentProgramCacheValue` holds shader's
            //    program id, tagged with the shared context's id.
            shader.setUniform(loc, za::Glsl::Vec4{1.f, 0.f, 0.f, 1.f});
            rt.clear(za::Color::Black);
            rt.draw(full, za::RenderStates{.shader = &shader});
            rt.display();

            // 2) Activate a fresh GL context. In this context, no shader is
            //    bound -- `glGetIntegerv(GL_CURRENT_PROGRAM)` would return 0.
            //    But the cache still holds the shader's program id from the
            //    previous context. Without the per-context tag check, the
            //    next `setUniform` would think shader is already bound and
            //    skip the rebind, calling `glUniform4f` on whatever (or
            //    nothing) is current in this context.
            {
                TestContext fresh;
                CHECK(fresh.getId() != 0u);

                // 3) Touch a uniform in the fresh context. With the fix, the
                //    UniformBinder reads the cache, sees the context tag
                //    mismatch, queries GL (which returns 0), so saves 0 and
                //    binds the shader. Without the fix, it would trust the
                //    stale cache value and skip the bind.
                shader.setUniform(loc, za::Glsl::Vec4{0.f, 1.f, 0.f, 1.f});
            }
            // TestContext destructor reactivates the shared context. Note
            // that on the shared context, the cache value is now stale in
            // a different way -- but that's the same context-tag-mismatch
            // path, so the fix handles it equally.

            // 4) Draw again on the shared context, this time with green.
            //    With the fix: re-tag fires, fallback query runs, shader
            //    rebinds correctly, uniform applies.
            shader.setUniform(loc, za::Glsl::Vec4{0.f, 1.f, 0.f, 1.f});
            rt.clear(za::Color::Black);
            rt.draw(full, za::RenderStates{.shader = &shader});
            rt.display();

            CHECK(rt.getTexture().copyToImage().getPixel({20, 20}) == za::Color::Green);
        }
#endif

        SECTION("Multiple RenderTextures with different views render correctly")
        {
            // Regression: cross-context viewport state. Each RenderTexture
            // has its own view and corresponding viewport. After the fix,
            // `resetGLStatesImpl` reapplies `cache.lastView` per target, so
            // switching contexts mid-frame doesn't leave the GL viewport
            // pointing at the previous target's rectangle.
            auto rtSmall = za::RenderTexture::create({40, 40}).value();
            auto rtLarge = za::RenderTexture::create({80, 80}).value();

            const za::RectangleShape rectSmall{
                {.position = {10.f, 10.f}, .fillColor = za::Color::Green, .size = {20.f, 20.f}}};
            const za::RectangleShape rectLarge{
                {.position = {30.f, 30.f}, .fillColor = za::Color::Red, .size = {20.f, 20.f}}};

            // Interleaved draws: each pass forces a setActive() between the
            // two RTs. If the viewport tracking regresses, one of these
            // produces blank pixels where geometry should be.
            for (int i = 0; i < 3; ++i)
            {
                rtSmall.clear(za::Color::Black);
                rtSmall.draw(rectSmall);
                rtSmall.display();

                rtLarge.clear(za::Color::Black);
                rtLarge.draw(rectLarge);
                rtLarge.display();
            }

            CHECK(rtSmall.getTexture().copyToImage().getPixel({20, 20}) == za::Color::Green);
            CHECK(rtLarge.getTexture().copyToImage().getPixel({40, 40}) == za::Color::Red);
        }

        SECTION("VertexBuffer draws correctly when interleaved with regular draws")
        {
            // Regression: `RenderTarget::draw(VertexBuffer&)` previously
            // force-zeroed `cache.lastVaoGroup` before the DrawGuard,
            // forcing a full VAO rebind. The light cache-hit path in
            // `applyDrawCacheStates` should be sufficient: after
            // `bindGLObjects(m_impl->vaoGroup)` at the end of the function
            // restores the standard VAO's attribs, subsequent draws (regular
            // or another VertexBuffer) must produce correct geometry.
            //
            // Mixed sequence below stresses the handoff:
            //   regular draw -> VB draw -> regular draw -> VB draw -> regular draw
            auto rt = za::RenderTexture::create({120, 30}).value();
            rt.clear(za::Color::Black);

            // 1) Regular draw to prime the cache.
            rt.draw(za::RectangleShape{{.position = {0.f, 0.f}, .fillColor = za::Color::Green, .size = {20.f, 30.f}}});

            // 2) VertexBuffer draw at x=[20, 40).
            za::VertexBuffer vb1{za::PrimitiveType::TriangleStrip};
            REQUIRE(vb1.create(4u));
            const za::Vertex v1[]{
                {{20.f, 0.f}, za::Color::Red, {0.f, 0.f}},
                {{40.f, 0.f}, za::Color::Red, {0.f, 0.f}},
                {{20.f, 30.f}, za::Color::Red, {0.f, 0.f}},
                {{40.f, 30.f}, za::Color::Red, {0.f, 0.f}},
            };
            REQUIRE(vb1.update(v1));
            rt.draw(vb1);

            // 3) Regular draw at x=[40, 60), checks the standard VAO
            //    attribs were restored after the VB draw.
            rt.draw(za::RectangleShape{{.position = {40.f, 0.f}, .fillColor = za::Color::Blue, .size = {20.f, 30.f}}});

            // 4) Another VertexBuffer draw immediately after a regular draw.
            za::VertexBuffer vb2{za::PrimitiveType::TriangleStrip};
            REQUIRE(vb2.create(4u));
            const za::Vertex v2[]{
                {{60.f, 0.f}, za::Color::Yellow, {0.f, 0.f}},
                {{80.f, 0.f}, za::Color::Yellow, {0.f, 0.f}},
                {{60.f, 30.f}, za::Color::Yellow, {0.f, 0.f}},
                {{80.f, 30.f}, za::Color::Yellow, {0.f, 0.f}},
            };
            REQUIRE(vb2.update(v2));
            rt.draw(vb2);

            // 5) Final regular draw at x=[80, 120) -- same standard VAO
            //    must again be in a valid state.
            rt.draw(za::RectangleShape{{.position = {80.f, 0.f}, .fillColor = za::Color::Magenta, .size = {40.f, 30.f}}});

            rt.display();

            const auto img = rt.getTexture().copyToImage();
            CHECK(img.getPixel({10, 15}) == za::Color::Green);
            CHECK(img.getPixel({30, 15}) == za::Color::Red);
            CHECK(img.getPixel({50, 15}) == za::Color::Blue);
            CHECK(img.getPixel({70, 15}) == za::Color::Yellow);
            CHECK(img.getPixel({100, 15}) == za::Color::Magenta);
        }

#ifndef ZA_SYSTEM_EMSCRIPTEN
        // Emscripten/WebGL does not support multiple GL contexts.
        SECTION("Persistent GL states are reset when an RT moves to a context already owned by another RT")
        {
            // Regression: when `setActive(true)` is called on a context whose
            // slot in `contextRenderTargetMap` already holds a *different*
            // RT id (not `invalidId`), `RenderTarget::setActive` clears
            // `cache.enable` but leaves `cache.glStatesSet` untrue. The next
            // `setupDraw` therefore skips `resetGLStatesImpl`, and persistent
            // states owned only by that helper (`GL_CULL_FACE`,
            // `GL_DEPTH_TEST`, `GL_COLOR_MASK`, the active texture unit)
            // inherit whatever the previous occupant or raw GL code left in
            // the context.
            //
            // We exercise the case using `GL_CULL_FACE` + `glCullFace(GL_FRONT_AND_BACK)`,
            // which culls every triangle until something disables it.
            auto rtA = za::RenderTexture::create({40, 40}).value();
            auto rtB = za::RenderTexture::create({40, 40}).value();

            const za::RectangleShape full{{.fillColor = za::Color::Green, .size = {40.f, 40.f}}};

            // 1) Prime rtA on the shared graphics context. After this draw
            //    rtA's `glStatesSet` is true, slot[shared] = rtA.id, and the
            //    shared context's GL state is the one `resetGLStatesImpl`
            //    establishes (cull off, etc.).
            rtA.clear(za::Color::Red);
            rtA.draw(full);
            rtA.display();

            {
                TestContext fresh;
                CHECK(fresh.getId() != 0u);

                // 2) Activate rtB on `fresh`. slot[fresh] starts invalid, so
                //    rtB's `glStatesSet` is forced false → `setupDraw` runs
                //    `resetGLStatesImpl` on `fresh` → `fresh`'s persistent
                //    GL state is sane. After this, slot[fresh] = rtB.id.
                rtB.clear(za::Color::Black);
                rtB.draw(full);
                rtB.display();

                // 3) Corrupt `fresh`'s persistent state behind SFML's back.
                //    `GL_CULL_FACE` is touched only by `resetGLStatesImpl`
                //    in the SFML pipeline, so it survives until that helper
                //    runs again. `glCullFace(GL_FRONT_AND_BACK)` makes the
                //    cull discard every triangle regardless of winding.
                glCheck(glEnable(GL_CULL_FACE));
                glCheck(glCullFace(GL_FRONT_AND_BACK));

                // 4) Activate rtA on `fresh`. setActive sees slot[fresh] = rtB.id,
                //    which is neither `invalidId` nor `rtA.id`. With the bug,
                //    only `cache.enable` is reset; rtA's `glStatesSet` stays
                //    true (carried over from step 1), so `setupDraw` skips
                //    `resetGLStatesImpl` and the cull-everything state leaks
                //    into the draw. The expected fix forces `glStatesSet =
                //    false` on this branch (or tags it with the context id),
                //    so the draw resets persistent state and the green quad
                //    actually rasterizes.
                rtA.clear(za::Color::Red);
                rtA.draw(full);
                rtA.display();

                // Restore `fresh`'s GL state before TestContext tears down.
                glCheck(glDisable(GL_CULL_FACE));
                glCheck(glCullFace(GL_BACK));
            }

            // With the fix: green wrote to the FBO. With the bug: every
            // triangle was culled, so the pixel is still the red clear value.
            CHECK(rtA.getTexture().copyToImage().getPixel({20, 20}) == za::Color::Green);
        }
#endif
    }
}
