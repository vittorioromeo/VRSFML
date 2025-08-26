#include "SFML/Graphics/Text.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"

// Other 1st party headers
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"

#include "SFML/System/LifetimeDependee.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/String.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] bool equalsApprox(const float left, const float right, const float epsilon)
{
    return sf::base::fabs(left - right) <= epsilon;
}

////////////////////////////////////////////////////////////
[[nodiscard]] bool equalsApprox(const sf::FloatRect& left, const sf::FloatRect& right, const float epsilon)
{
    return equalsApprox(left.position.x, right.position.x, epsilon) &&
           equalsApprox(left.position.y, right.position.y, epsilon) &&
           equalsApprox(left.size.x, right.size.x, epsilon) && equalsApprox(left.size.y, right.size.y, epsilon);
}

} // namespace

TEST_CASE("[Graphics] sf::Text" * doctest::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Text, sf::Font&&, sf::String, unsigned int));
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Text, const sf::Font&&, sf::String, unsigned int));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Text));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Text));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Text));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Text));
    }

    const auto font = sf::Font::openFromFile("Graphics/tuffy.ttf").value();

    SECTION("Construction")
    {
        SECTION("Font constructor")
        {
            const sf::Text text(font, {});
            CHECK(text.getString() == "");
            CHECK(&text.getFont() == &font);
            CHECK(text.getCharacterSize() == 30);
            CHECK(text.getLetterSpacing() == 1.f);
            CHECK(text.getLineSpacing() == 1.f);
            CHECK(text.getStyle() == sf::Text::Style::Regular);
            CHECK(text.getFillColor() == sf::Color::White);
            CHECK(text.getOutlineColor() == sf::Color::Black);
            CHECK(text.getOutlineThickness() == 0);
            CHECK(text.getLineAlignment() == sf::Text::LineAlignment::Default);
            CHECK(text.getShapedGlyphs().empty());
            CHECK(text.getClusterGrouping() == sf::Text::ClusterGrouping::Character);
            CHECK(text.getLocalBounds() == sf::FloatRect());
            CHECK(text.getGlobalBounds() == sf::FloatRect());
        }

        SECTION("Font and string constructor")
        {
            const sf::Text text(font, {.string = "abcdefghijklmnopqrstuvwxyz"});
            CHECK(text.getString() == "abcdefghijklmnopqrstuvwxyz");
            CHECK(&text.getFont() == &font);
            CHECK(text.getCharacterSize() == 30);
            CHECK(text.getLetterSpacing() == 1.f);
            CHECK(text.getLineSpacing() == 1.f);
            CHECK(text.getStyle() == sf::Text::Style::Regular);
            CHECK(text.getFillColor() == sf::Color::White);
            CHECK(text.getOutlineColor() == sf::Color::Black);
            CHECK(text.getOutlineThickness() == 0);
            CHECK(text.getLineAlignment() == sf::Text::LineAlignment::Default);
            CHECK_FALSE(text.getShapedGlyphs().empty());
            CHECK(text.getClusterGrouping() == sf::Text::ClusterGrouping::Character);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({1, 8}, {358, 28}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({1, 8}, {358, 28}), 1.f));
        }

        SECTION("Font, string, and character size constructor")
        {
            const sf::Text text(font, {.string = "abcdefghijklmnopqrstuvwxyz", .characterSize = 24});
            CHECK(text.getString() == "abcdefghijklmnopqrstuvwxyz");
            CHECK(&text.getFont() == &font);
            CHECK(text.getCharacterSize() == 24);
            CHECK(text.getLetterSpacing() == 1.f);
            CHECK(text.getLineSpacing() == 1.f);
            CHECK(text.getStyle() == sf::Text::Style::Regular);
            CHECK(text.getFillColor() == sf::Color::White);
            CHECK(text.getOutlineColor() == sf::Color::Black);
            CHECK(text.getOutlineThickness() == 0);
            CHECK(text.getLineAlignment() == sf::Text::LineAlignment::Default);
            CHECK_FALSE(text.getShapedGlyphs().empty());
            CHECK(text.getClusterGrouping() == sf::Text::ClusterGrouping::Character);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({1, 7}, {292, 22}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({1, 7}, {292, 22}), 1.f));
        }
    }

    SECTION("Set/get string")
    {
        sf::Text text(font, {});
        text.setString("abcdefghijklmnopqrstuvwxyz");
        CHECK(text.getString() == "abcdefghijklmnopqrstuvwxyz");
    }

    SECTION("Set/get font")
    {
        const auto otherFont = sf::Font::openFromFile("Graphics/tuffy.ttf").value();

        sf::Text text(font, {});
        text.setFont(otherFont);
        CHECK(&text.getFont() == &otherFont);
    }

    SECTION("Set/get character size")
    {
        sf::Text text(font, {});
        text.setCharacterSize(48);
        CHECK(text.getCharacterSize() == 48);
    }

    SECTION("Set/get line spacing")
    {
        sf::Text text(font, {});
        text.setLineSpacing(42);
        CHECK(text.getLineSpacing() == 42);
    }

    SECTION("Set/get letter spacing")
    {
        sf::Text text(font, {});
        text.setLetterSpacing(15);
        CHECK(text.getLetterSpacing() == 15);
    }

    SECTION("Set/get style")
    {
        sf::Text text(font, {});
        text.setStyle(sf::Text::Style::Bold | sf::Text::Style::Italic);
        CHECK(text.getStyle() == (sf::Text::Style::Bold | sf::Text::Style::Italic));
    }

    SECTION("Set/get fill color")
    {
        sf::Text text(font, {.string = "Fill color"});
        text.setFillColor(sf::Color::Red);
        CHECK(text.getFillColor() == sf::Color::Red);
    }

    SECTION("Set/get outline color")
    {
        sf::Text text(font, {.string = "Outline color"});
        text.setOutlineColor(sf::Color::Green);
        CHECK(text.getOutlineColor() == sf::Color::Green);
    }

    SECTION("Set/get outline thickness")
    {
        sf::Text text(font, {});
        text.setOutlineThickness(3.14f);
        CHECK(text.getOutlineThickness() == 3.14f);
    }

    SECTION("Set get line alignment")
    {
        sf::Text text(font, {.string = "QWERTY"});
        text.position = {50, 25};
        text.setLineAlignment(sf::Text::LineAlignment::Center);
        CHECK(text.getLineAlignment() == sf::Text::LineAlignment::Center);
        text.setLineAlignment(sf::Text::LineAlignment::Right);
        CHECK(text.getLineAlignment() == sf::Text::LineAlignment::Right);
    }

    SECTION("Set/get cluster grouping")
    {
        sf::Text text(font, {});
        text.setClusterGrouping(sf::Text::ClusterGrouping::Grapheme);
        CHECK(text.getClusterGrouping() == sf::Text::ClusterGrouping::Grapheme);
    }

    SECTION("Get bounds")
    {
        sf::Text text(font, {.string = "Test", .characterSize = 18u});
        text.position = {100, 200};
        CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({1, 5}, {32, 13}), 1.f));
        CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({101, 205}, {32, 13}), 1.f));

        SECTION("Add underline")
        {
            text.setStyle(sf::Text::Style::Underlined);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({1, 5}, {32, 13}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({101, 205}, {32, 13}), 1.f));
        }

        SECTION("Add strikethrough")
        {
            text.setStyle(sf::Text::Style::StrikeThrough);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({1, 5}, {32, 13}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({101, 205}, {32, 13}), 1.f));
        }

        SECTION("Change rotation")
        {
            text.rotation = sf::degrees(180);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({1, 5}, {32, 13}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), (sf::FloatRect({67, 182}, {32, 13})), 1.f));
        }

        SECTION("Change alignment")
        {
            text.setLineAlignment(sf::Text::LineAlignment::Center);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({-15, 5}, {32, 13}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({85, 205}, {32, 13}), 1.f));
            text.setLineAlignment(sf::Text::LineAlignment::Right);
            CHECK(equalsApprox(text.getLocalBounds(), sf::FloatRect({-31, 5}, {32, 13}), 1.f));
            CHECK(equalsApprox(text.getGlobalBounds(), sf::FloatRect({69, 205}, {32, 13}), 1.f));
        }
    }

#ifdef SFML_ENABLE_LIFETIME_TRACKING
    SECTION("Lifetime tracking")
    {
        SECTION("Return local from function")
        {
            const auto badFunction = []
            {
                const auto localFont = sf::Font::openFromFile("Graphics/tuffy.ttf").value();
                return sf::Text(localFont, {});
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard{"Font"};
            CHECK(!guard.fatalErrorTriggered("Font"));

            badFunction();

            CHECK(guard.fatalErrorTriggered("Font"));
        }

        SECTION("Move struct holding both dependee and dependant")
        {
            struct BadStruct
            {
                BadStruct() :
                    memberFont{sf::Font::openFromFile("Graphics/tuffy.ttf").value()},
                    memberText{memberFont, {}}
                {
                }

                sf::Font memberFont;
                sf::Text memberText;
            };

            const sf::priv::LifetimeDependee::TestingModeGuard guard{"Font"};
            CHECK(!guard.fatalErrorTriggered("Font"));

            sf::base::Optional<BadStruct> badStruct0;
            badStruct0.emplace();
            CHECK(!guard.fatalErrorTriggered("Font"));

            const BadStruct badStruct1 = SFML_BASE_MOVE(badStruct0.value());
            CHECK(!guard.fatalErrorTriggered("Font"));

            badStruct0.reset();
            CHECK(guard.fatalErrorTriggered("Font"));
        }

        SECTION("Optionals and move")
        {
            sf::base::Optional<sf::Text> t0;
            sf::base::Optional<sf::Text> t1;

            t0.emplace(sf::Text{font, {}});
            t1.emplace(sf::Text{font, {}});

            t1 = SFML_BASE_MOVE(t0);
            t0.reset();
        }

        SECTION("Dependee move assignment")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"Font"};
            CHECK(!guard.fatalErrorTriggered("Font"));

            auto sb0 = sf::Font::openFromFile("Graphics/tuffy.ttf").value();
            CHECK(!guard.fatalErrorTriggered("Font"));

            sf::Text s0(sb0, {});
            CHECK(!guard.fatalErrorTriggered("Font"));

            sb0 = sf::Font::openFromFile("Graphics/tuffy.ttf").value();
            CHECK(!guard.fatalErrorTriggered("Font"));
        }
    }
#endif
}
