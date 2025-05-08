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
            CHECK(text.findCharacterPos(0) == sf::Vec2f{});
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
            CHECK(text.findCharacterPos(0) == sf::Vec2f{});
            CHECK(text.getLocalBounds() == sf::FloatRect({1, 8}, {357, 28}));
            CHECK(text.getGlobalBounds() == sf::FloatRect({1, 8}, {357, 28}));
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
            CHECK(text.findCharacterPos(0) == sf::Vec2f{});
            CHECK(text.getLocalBounds() == sf::FloatRect({1, 7}, {290, 22}));
            CHECK(text.getGlobalBounds() == sf::FloatRect({1, 7}, {290, 22}));
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
        sf::Text   text(font, {});
        const auto otherFont = sf::Font::openFromFile("Graphics/tuffy.ttf").value();
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

    SECTION("findCharacterPos()")
    {
        sf::Text text(font, {.string = "\tabcdefghijklmnopqrstuvwxyz \n"});
        text.position = {120, 240};
        CHECK(text.findCharacterPos(0) == sf::Vec2f{120, 240});
        CHECK(text.findCharacterPos(1) == sf::Vec2f{156, 240});
        CHECK(text.findCharacterPos(2) == sf::Vec2f{170, 240});
        CHECK(text.findCharacterPos(3) == sf::Vec2f{185, 240});
        CHECK(text.findCharacterPos(4) == sf::Vec2f{198, 240});

        // Indices that are too large are capped at maximum valid index
        CHECK(text.findCharacterPos(1000) == sf::Vec2f{120, 277});
    }

    SECTION("Get bounds")
    {
        sf::Text text(font, {.string = "Test", .characterSize = 18u});
        text.position = {100, 200};
        CHECK(text.getLocalBounds() == sf::FloatRect({1, 5}, {33, 13}));
        CHECK(text.getGlobalBounds() == sf::FloatRect({101, 205}, {33, 13}));

        SECTION("Add underline")
        {
            text.setStyle(sf::Text::Style::Underlined);
            CHECK(text.getLocalBounds() == sf::FloatRect({1, 5}, {33, 13}));
            CHECK(text.getGlobalBounds() == sf::FloatRect({101, 205}, {33, 13}));
        }

        SECTION("Add strikethrough")
        {
            text.setStyle(sf::Text::Style::StrikeThrough);
            CHECK(text.getLocalBounds() == sf::FloatRect({1, 5}, {33, 13}));
            CHECK(text.getGlobalBounds() == sf::FloatRect({101, 205}, {33, 13}));
        }

        SECTION("Change rotation")
        {
            text.rotation = sf::degrees(180);
            CHECK(text.getLocalBounds() == sf::FloatRect({1, 5}, {33, 13}));
            CHECK(text.getGlobalBounds() == Approx(sf::FloatRect({66, 182}, {33, 13})));
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

            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            badFunction();

            CHECK(guard.fatalErrorTriggered());
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

            const sf::priv::LifetimeDependee::TestingModeGuard guard;
            CHECK(!guard.fatalErrorTriggered());

            sf::base::Optional<BadStruct> badStruct0;
            badStruct0.emplace();
            CHECK(!guard.fatalErrorTriggered());

            const BadStruct badStruct1 = SFML_BASE_MOVE(badStruct0.value());
            CHECK(!guard.fatalErrorTriggered());

            badStruct0.reset();
            CHECK(guard.fatalErrorTriggered());
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
    }
#endif
}
