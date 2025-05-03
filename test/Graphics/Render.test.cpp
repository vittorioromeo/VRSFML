#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/ContextSettings.hpp"
#include "SFML/Window/WindowContext.hpp"

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>

TEST_CASE("[Graphics] Render Tests" * doctest::skip(skipDisplayTests))
{
    CHECK(!sf::WindowContext::isInstalled());
    CHECK(!sf::GraphicsContext::isInstalled());

    auto graphicsContext = sf::GraphicsContext::create().value();

    CHECK(sf::WindowContext::isInstalled());
    CHECK(sf::GraphicsContext::isInstalled());

    SECTION("Stencil Tests")
    {
        auto renderTexture = sf::RenderTexture::create({100, 100}, sf::ContextSettings{.depthBits = 0, .stencilBits = 8})
                                 .value();

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
}
