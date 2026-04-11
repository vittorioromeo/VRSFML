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
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/VAOHandle.hpp"
#include "SFML/Graphics/VBOHandle.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/WindowContext.hpp"

#include "SFML/System/Priv/Vec2Base.hpp"

#include <Doctest.hpp>

TEST_CASE("[Graphics] Render Tests" * doctest::skip(skipDisplayTests))
{
    CHECK(!sf::WindowContext::isInstalled());
    CHECK(!sf::GraphicsContext::isInstalled());

    auto graphicsContext = sf::GraphicsContext::create().value();

    CHECK(sf::WindowContext::isInstalled());
    CHECK(sf::GraphicsContext::isInstalled());

    SECTION("Stencil Tests")
    {
        auto renderTexture = sf::RenderTexture::create({100, 100}, {.depthBits = 0, .stencilBits = 8}).value();

        renderTexture.clear(sf::Color::Red, sf::StencilValue{127u});

        const sf::RectangleShape shape1{{.fillColor = sf::Color::Green, .size = {100.f, 100.f}}};
        const sf::RectangleShape shape2{{.fillColor = sf::Color::Blue, .size = {100.f, 100.f}}};

        SECTION("Stencil-Only")
        {
            renderTexture.draw(shape1,
                               sf::RenderStates{.stencilMode = {
                                                    .stencilComparison      = sf::StencilComparison::Always,
                                                    .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                    .stencilOnly            = true,
                                                    .stencilReference       = sf::StencilValue{1u},
                                                    .stencilMask            = sf::StencilValue{0xFFu},
                                                }});
            renderTexture.display();
            CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
        }

        SECTION("Comparisons")
        {
            SECTION("Always")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{1u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Equal")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{126u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);

                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Greater")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Greater,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{126u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Greater,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Greater,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{128u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("GreaterEqual")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::GreaterEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{126u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::GreaterEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
                renderTexture.draw(shape2,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::GreaterEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{128u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();

                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Blue);
            }

            SECTION("Less")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Less,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{128u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Less,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Less,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{126u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("LessEqual")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::LessEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{128u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::LessEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
                renderTexture.draw(shape2,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::LessEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{126u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();

                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Blue);
            }

            SECTION("Never")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Never,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
            }

            SECTION("NotEqual")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::NotEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::NotEqual,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{128u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }
        }

        SECTION("Updating")
        {
            SECTION("Decrement")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Decrement,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Decrement,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{126u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Increment")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Increment,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Increment,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{128u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Invert")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Invert,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Invert,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{0x80u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Keep")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Keep,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Replace")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = sf::StencilValue{255u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{255u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Zero")
            {
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Always,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Zero,
                                                        .stencilOnly            = true,
                                                        .stencilReference       = sf::StencilValue{127u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.draw(shape1,
                                   sf::RenderStates{.stencilMode = {
                                                        .stencilComparison      = sf::StencilComparison::Equal,
                                                        .stencilUpdateOperation = sf::StencilUpdateOperation::Zero,
                                                        .stencilOnly            = false,
                                                        .stencilReference       = sf::StencilValue{0u},
                                                        .stencilMask            = sf::StencilValue{0xFFu},
                                                    }});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }
        }
    }

    SECTION("Auto-batch generation counter tests")
    {
        auto renderTexture = sf::RenderTexture::create({100, 100}).value();

        const sf::RectangleShape shape{{.fillColor = sf::Color::Red, .size = {100.f, 100.f}}};

        SECTION("Shader uniform mutation breaks batch")
        {
            constexpr auto fragSource = R"glsl(
layout(location = 2) uniform sampler2D sf_u_texture;
layout(location = 7) uniform float blink_alpha;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = sf_v_color * blink_alpha;
}
)glsl";

            auto shader = sf::Shader::loadFromMemory({.fragmentCode = fragSource}).value();

            const auto loc = shader.getUniformLocation("blink_alpha").value();
            shader.setUniform(loc, 1.f);

            const sf::RenderStates states{.shader = &shader};

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
            constexpr auto fragSource = R"glsl(
layout(location = 2) uniform sampler2D sf_u_texture;
layout(location = 7) uniform float blink_alpha;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = sf_v_color * blink_alpha;
}
)glsl";

            auto shader = sf::Shader::loadFromMemory({.fragmentCode = fragSource}).value();

            const auto loc = shader.getUniformLocation("blink_alpha").value();

            const sf::RenderStates states{.shader = &shader};

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

            // Wrong order: mutate, then flush — the pending batch is
            // drawn with the ALREADY-MUTATED uniform (corruption).
            // The generation counter must detect this and break the batch.
            renderTexture.clear();
            shader.setUniform(loc, 1.f);
            renderTexture.draw(shape, states); // batch 1: intended alpha=1
            shader.setUniform(loc, 0.5f);      // oops — mutated before flush
            renderTexture.flush();             // draws batch 1 with alpha=0.5 (wrong!)
            renderTexture.draw(shape, states); // batch 2: alpha=0.5
            CHECK(renderTexture.display().drawCalls == 2u);
        }

        SECTION("Flush before display is redundant (display already flushes)")
        {
            constexpr auto fragSource = R"glsl(
layout(location = 2) uniform sampler2D sf_u_texture;
layout(location = 7) uniform float blink_alpha;

in vec4 sf_v_color;
in vec2 sf_v_texCoord;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = sf_v_color * blink_alpha;
}
)glsl";

            auto shader = sf::Shader::loadFromMemory({.fragmentCode = fragSource}).value();

            const auto loc = shader.getUniformLocation("blink_alpha").value();

            const sf::RenderStates states{.shader = &shader};

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
            constexpr auto vertexSource = R"glsl(
layout(location = 0) uniform vec3 sf_u_mvpRow0;
layout(location = 1) uniform vec3 sf_u_mvpRow1;
layout(location = 3) uniform vec2 sf_u_invTextureSize;

layout(location = 0) in vec2 sf_a_position;
layout(location = 1) in vec4 sf_a_color;
layout(location = 2) in vec2 sf_a_texCoord;

layout(location = 3) in vec2 instance_offset;
layout(location = 4) in vec4 instance_color;

out vec4 v_color;

void main()
{
    const vec2 worldPos = instance_offset + (sf_a_position * vec2(20.0, 20.0));

    gl_Position = vec4(dot(sf_u_mvpRow0, vec3(worldPos, 1.0)),
                       dot(sf_u_mvpRow1, vec3(worldPos, 1.0)),
                       0.0,
                       1.0);

    v_color = instance_color;
}
)glsl";

            constexpr auto fragmentSource = R"glsl(
in vec4 v_color;

layout(location = 0) out vec4 sf_fragColor;

void main()
{
    sf_fragColor = v_color;
}
)glsl";

            struct InstanceData
            {
                sf::Vec2f offset;
                sf::Color color;
            };

            auto shader = sf::Shader::loadFromMemory({.vertexCode = vertexSource, .fragmentCode = fragmentSource}).value();

            sf::VAOHandle vaoHandle;
            sf::VBOHandle instanceVBO;

            const auto drawInstance = [&](InstanceData instance)
            {
                auto setupAttribs = [&](sf::InstanceAttributeBinder& binder)
                {
                    binder.uploadContiguousData(instanceVBO, &instance);
                    binder.setupField<&InstanceData::offset>(3);
                    binder.setupField<&InstanceData::color>(4);
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
    }
}
