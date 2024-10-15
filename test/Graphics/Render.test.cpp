#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Texture.hpp"

#include "SFML/Window/WindowContext.hpp"

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>

TEST_CASE("[Graphics] Render Tests" * doctest::skip(skipDisplayTests))
{
    CHECK(sf::WindowContext::getInstalled() == nullptr);
    CHECK(sf::GraphicsContext::getInstalled() == nullptr);

    sf::GraphicsContext graphicsContext;

    CHECK(sf::WindowContext::getInstalled() == &static_cast<sf::WindowContext&>(graphicsContext));
    CHECK(sf::GraphicsContext::getInstalled() == &graphicsContext);

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
                               /* texture */ nullptr,
                               sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                             sf::StencilUpdateOperation::Keep,
                                                             sf::StencilValue{1u},
                                                             sf::StencilValue{0xFFu},
                                                             true}});
            renderTexture.display();
            CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
        }

        SECTION("Comparisons")
        {
            SECTION("Always")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{1u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Equal")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{126u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Greater")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Greater,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{126u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Greater,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Greater,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{128u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("GreaterEqual")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::GreaterEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{126u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::GreaterEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
                renderTexture.draw(shape2,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::GreaterEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{128u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Blue);
            }

            SECTION("Less")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Less,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{128u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Less,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Less,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{126u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("LessEqual")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::LessEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{128u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::LessEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
                renderTexture.draw(shape2,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::LessEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{126u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Blue);
            }

            SECTION("Never")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Never,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
            }

            SECTION("NotEqual")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::NotEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Red);
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::NotEqual,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{128u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }
        }

        SECTION("Updating")
        {
            SECTION("Decrement")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Decrement,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 true}});
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Decrement,
                                                                 sf::StencilValue{126u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Increment")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Increment,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 true}});
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Increment,
                                                                 sf::StencilValue{128u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Invert")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Invert,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 true}});
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Invert,
                                                                 sf::StencilValue{0x80u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Keep")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 true}});
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Keep,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Replace")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Replace,
                                                                 sf::StencilValue{255u},
                                                                 sf::StencilValue{0xFFu},
                                                                 true}});
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Replace,
                                                                 sf::StencilValue{255u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }

            SECTION("Zero")
            {
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Always,
                                                                 sf::StencilUpdateOperation::Zero,
                                                                 sf::StencilValue{127u},
                                                                 sf::StencilValue{0xFFu},
                                                                 true}});
                renderTexture.draw(shape1,
                                   /* texture */ nullptr,
                                   sf::RenderStates{.stencilMode{sf::StencilComparison::Equal,
                                                                 sf::StencilUpdateOperation::Zero,
                                                                 sf::StencilValue{0u},
                                                                 sf::StencilValue{0xFFu},
                                                                 false}});
                renderTexture.display();
                CHECK(renderTexture.getTexture().copyToImage().getPixel({50, 50}) == sf::Color::Green);
            }
        }
    }
}
