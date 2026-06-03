#include "SystemUtil.hpp"

#include "SFML/Graphics/Text.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"

// Other 1st party headers
#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/TextData.hpp"
#include "SFML/Graphics/TextUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/LifetimeDependee.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Priv/Vec2Base.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Trait/IsConstructible.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Graphics] sf::Text" * tst::skip(skipDisplayTests))
{
    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Text, sf::Font&&, sf::Utf8String, unsigned int));
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Text, const sf::Font&&, sf::Utf8String, unsigned int));
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Text));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Text));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Text));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Text));
    }

    const auto font = sf::Font::openFromFile("tuffy.ttf").value();

    SECTION("Construction")
    {
        SECTION("Font constructor")
        {
            const sf::Text text(font, {});
            CHECK(text.getString() == "");
            CHECK(&text.getFont() == &font);
            CHECK(text.getCharacterSize() == 0);
            CHECK(text.getLetterSpacing() == 1.f);
            CHECK(text.getLineSpacing() == 1.f);
            CHECK(text.isBold() == false);
            CHECK(text.isItalic() == false);
            CHECK(text.isUnderlined() == false);
            CHECK(text.isStrikeThrough() == false);
            CHECK(text.getFillColor() == sf::Color::White);
            CHECK(text.getOutlineColor() == sf::Color::Black);
            CHECK(text.getOutlineThickness() == 0);
            CHECK(text.findCharacterPos(0) == sf::Vec2f{});
            CHECK(text.getLocalBounds() == sf::Rect2f());
            CHECK(text.getGlobalBounds() == sf::Rect2f());
        }

        SECTION("Font and string constructor")
        {
            const sf::Text text(font, {.string = "abcdefghijklmnopqrstuvwxyz", .characterSize = 30u});
            CHECK(text.getString() == "abcdefghijklmnopqrstuvwxyz");
            CHECK(&text.getFont() == &font);
            CHECK(text.getCharacterSize() == 30);
            CHECK(text.getLetterSpacing() == 1.f);
            CHECK(text.getLineSpacing() == 1.f);
            CHECK(text.isBold() == false);
            CHECK(text.isItalic() == false);
            CHECK(text.isUnderlined() == false);
            CHECK(text.isStrikeThrough() == false);
            CHECK(text.getFillColor() == sf::Color::White);
            CHECK(text.getOutlineColor() == sf::Color::Black);
            CHECK(text.getOutlineThickness() == 0);
            CHECK(text.findCharacterPos(0) == sf::Vec2f{});
            CHECK(text.getLocalBounds() == sf::Rect2f({1, 8}, {357, 28}));
            CHECK(text.getGlobalBounds() == sf::Rect2f({1, 8}, {357, 28}));
        }

        SECTION("Font, string, and character size constructor")
        {
            const sf::Text text(font, {.string = "abcdefghijklmnopqrstuvwxyz", .characterSize = 24});
            CHECK(text.getString() == "abcdefghijklmnopqrstuvwxyz");
            CHECK(&text.getFont() == &font);
            CHECK(text.getCharacterSize() == 24);
            CHECK(text.getLetterSpacing() == 1.f);
            CHECK(text.getLineSpacing() == 1.f);
            CHECK(text.isBold() == false);
            CHECK(text.isItalic() == false);
            CHECK(text.isUnderlined() == false);
            CHECK(text.isStrikeThrough() == false);
            CHECK(text.getFillColor() == sf::Color::White);
            CHECK(text.getOutlineColor() == sf::Color::Black);
            CHECK(text.getOutlineThickness() == 0);
            CHECK(text.findCharacterPos(0) == sf::Vec2f{});
            CHECK(text.getLocalBounds() == sf::Rect2f({1, 7}, {290, 22}));
            CHECK(text.getGlobalBounds() == sf::Rect2f({1, 7}, {290, 22}));
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
        const auto otherFont = sf::Font::openFromFile("tuffy.ttf").value();

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

    SECTION("Set/get bold")
    {
        sf::Text text(font, {});
        text.setBold(true);
        CHECK(text.isBold() == true);
    }

    SECTION("Set/get italic")
    {
        sf::Text text(font, {});
        text.setItalic(true);
        CHECK(text.isItalic() == true);
    }

    SECTION("Set/get underlined")
    {
        sf::Text text(font, {});
        text.setUnderlined(true);
        CHECK(text.isUnderlined() == true);
    }

    SECTION("Set/get strikeThrough")
    {
        sf::Text text(font, {});
        text.setStrikeThrough(true);
        CHECK(text.isStrikeThrough() == true);
    }

    SECTION("Set/get fill color")
    {
        sf::Text text(font, {.string = "Fill color", .characterSize = 30u});
        text.setFillColor(sf::Color::Red);
        CHECK(text.getFillColor() == sf::Color::Red);
    }

    SECTION("Set/get outline color")
    {
        sf::Text text(font, {.string = "Outline color", .characterSize = 30u});
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
        sf::Text text(font, {.string = "\tabcdefghijklmnopqrstuvwxyz \n", .characterSize = 30u});
        text.position = {120, 240};
        CHECK(text.findCharacterPos(0) == sf::Vec2f{120, 240});
        CHECK(text.findCharacterPos(1) == sf::Vec2f{156, 240});
        CHECK(text.findCharacterPos(2) == sf::Vec2f{170, 240});
        CHECK(text.findCharacterPos(3) == sf::Vec2f{185, 240});
        CHECK(text.findCharacterPos(4) == sf::Vec2f{198, 240});

        // Indices that are too large are capped at maximum valid index
        CHECK(text.findCharacterPos(1000) == sf::Vec2f{120, 277});
    }

    SECTION("TextUtils helpers")
    {
        using sf::TextUtils::precomputeTextQuadCount;

        SECTION("precomputeTextQuadCount with no styles")
        {
            // No underline / strikethrough -> just one quad per visible glyph
            CHECK(precomputeTextQuadCount("A", false, false) == 1u);
            CHECK(precomputeTextQuadCount("Abc", false, false) == 3u);

            // Whitespace is not a glyph
            CHECK(precomputeTextQuadCount(" ", false, false) == 0u);
            CHECK(precomputeTextQuadCount("\t", false, false) == 0u);
            CHECK(precomputeTextQuadCount("A B", false, false) == 2u);
            CHECK(precomputeTextQuadCount("A\tB", false, false) == 2u);

            // Newlines are not glyphs
            CHECK(precomputeTextQuadCount("\n", false, false) == 0u);
            CHECK(precomputeTextQuadCount("A\nB", false, false) == 2u);

            // Carriage returns are skipped
            CHECK(precomputeTextQuadCount("\r", false, false) == 0u);
            CHECK(precomputeTextQuadCount("A\rB", false, false) == 2u);
        }

        SECTION("precomputeTextQuadCount with underline")
        {
            // Single line: 1 line emitted at the end + glyph quads
            CHECK(precomputeTextQuadCount("A", true, false) == 2u);
            CHECK(precomputeTextQuadCount("Abc", true, false) == 4u);

            // Whitespace-only string still gets a trailing decoration line
            CHECK(precomputeTextQuadCount(" ", true, false) == 1u);

            // Two non-empty lines -> 2 underline lines + glyphs
            CHECK(precomputeTextQuadCount("A\nB", true, false) == 4u);

            // Consecutive newlines do not double-emit
            CHECK(precomputeTextQuadCount("A\n\nB", true, false) == 4u);

            // Trailing newline (line ends empty) -> only one underline (for the "A" line)
            CHECK(precomputeTextQuadCount("A\n", true, false) == 2u);
        }

        SECTION("precomputeTextQuadCount with strikethrough")
        {
            // Same shape as underline-only, but emitted via the strikethrough branch
            CHECK(precomputeTextQuadCount("A", false, true) == 2u);
            CHECK(precomputeTextQuadCount("A\nB", false, true) == 4u);
            CHECK(precomputeTextQuadCount("A\n\nB", false, true) == 4u);
        }

        SECTION("precomputeTextQuadCount with both decorations")
        {
            // Both underline and strikethrough emit one line each per non-empty line
            CHECK(precomputeTextQuadCount("A", true, true) == 3u);
            CHECK(precomputeTextQuadCount("A\nB", true, true) == 6u);
            CHECK(precomputeTextQuadCount("A\n\nB", true, true) == 6u);
        }

        SECTION("precomputeTextQuadCount TextData overload")
        {
            // The overload returns 0 for an empty string instead of asserting
            CHECK(precomputeTextQuadCount(sf::TextData{}) == 0u);

            CHECK(precomputeTextQuadCount(sf::TextData{.string = "Abc", .characterSize = 18u}) == 3u);
            CHECK(precomputeTextQuadCount(sf::TextData{.string = "Abc", .characterSize = 18u, .underlined = true}) == 4u);
            CHECK(precomputeTextQuadCount(sf::TextData{.string = "A\nB", .characterSize = 18u, .strikeThrough = true}) ==
                  4u);
            CHECK(precomputeTextQuadCount(
                      sf::TextData{.string = "A\nB", .characterSize = 18u, .underlined = true, .strikeThrough = true}) ==
                  6u);
        }

        SECTION("precomputeTextLocalBounds")
        {
            // Empty string -> empty rect (early return)
            CHECK(sf::TextUtils::precomputeTextLocalBounds(font, sf::TextData{}) == sf::Rect2f{});

            // Matches what `sf::Text::getLocalBounds` returns for the same inputs
            const sf::TextData td{.string = "Test", .characterSize = 18u};
            const sf::Text     text(font, td);
            CHECK(sf::TextUtils::precomputeTextLocalBounds(font, td) == text.getLocalBounds());
        }

        SECTION("precomputeTextGlobalBounds")
        {
            // Empty text -> empty rect transformed is still empty rect
            CHECK(sf::TextUtils::precomputeTextGlobalBounds(font, sf::TextData{}) == sf::Rect2f{});

            // Without any transform, global bounds equal local bounds
            const sf::TextData td{.string = "Test", .characterSize = 18u};
            CHECK(sf::TextUtils::precomputeTextGlobalBounds(font, td) == sf::TextUtils::precomputeTextLocalBounds(font, td));

            // With a translation, global bounds shift accordingly
            sf::TextData translated = td;
            translated.position     = {100.f, 200.f};

            const sf::Rect2f local         = sf::TextUtils::precomputeTextLocalBounds(font, translated);
            const sf::Rect2f globalShifted = sf::TextUtils::precomputeTextGlobalBounds(font, translated);
            CHECK(globalShifted.position == local.position + sf::Vec2f{100.f, 200.f});
            CHECK(globalShifted.size == local.size);

            // Matches what `sf::Text::getGlobalBounds` returns for the same inputs
            sf::Text text(font, td);
            text.position = {100.f, 200.f};
            CHECK(sf::TextUtils::precomputeTextGlobalBounds(font, translated) == text.getGlobalBounds());
        }

        SECTION("computeAnchorOrigin")
        {
            // For an empty string, bounds is {} so any anchor returns {0, 0}
            CHECK(sf::TextUtils::computeAnchorOrigin(font, sf::TextData{}, {0.5f, 0.5f}) == sf::Vec2f{});

            const sf::TextData td{.string = "Test", .characterSize = 18u};
            const sf::Rect2f   bounds = sf::TextUtils::precomputeTextLocalBounds(font, td);

            // {0, 0} -> top-left anchor (== bounds.position)
            CHECK(sf::TextUtils::computeAnchorOrigin(font, td, {0.f, 0.f}) == bounds.position);

            // {1, 1} -> bottom-right anchor
            CHECK(sf::TextUtils::computeAnchorOrigin(font, td, {1.f, 1.f}) == bounds.position + bounds.size);

            // {0.5, 0} -> top-center anchor (matches Rect2 helper)
            CHECK(sf::TextUtils::computeAnchorOrigin(font, td, {0.5f, 0.f}) == bounds.getTopCenter());

            // {0.5, 0.5} -> center anchor (matches Rect2 helper)
            CHECK(sf::TextUtils::computeAnchorOrigin(font, td, {0.5f, 0.5f}) == bounds.getCenter());
        }

        SECTION("anchored")
        {
            const sf::TextData td{.position = {100.f, 200.f}, .string = "Test", .characterSize = 18u};

            // Returned TextData carries through every input field; only `origin` changes
            const sf::TextData result = sf::TextUtils::anchored(font, td, {0.5f, 0.5f});
            CHECK(result.position == td.position);
            CHECK(result.string == td.string);
            CHECK(result.characterSize == td.characterSize);

            // The origin matches what `computeAnchorOrigin` would return for the same inputs
            CHECK(result.origin == sf::TextUtils::computeAnchorOrigin(font, td, {0.5f, 0.5f}));

            // Top-left anchor on an empty string yields the default-constructed origin
            CHECK(sf::TextUtils::anchored(font, sf::TextData{}, {0.f, 0.f}).origin == sf::Vec2f{});
        }
    }

    SECTION("Get bounds")
    {
        sf::Text text(font, {.string = "Test", .characterSize = 18u});
        text.position = {100, 200};
        CHECK(text.getLocalBounds() == sf::Rect2f({1, 5}, {33, 13}));
        CHECK(text.getGlobalBounds() == sf::Rect2f({101, 205}, {33, 13}));

        SECTION("Add underline")
        {
            text.setUnderlined(true);
            CHECK(text.getLocalBounds() == sf::Rect2f({1, 5}, {33, 13}));
            CHECK(text.getGlobalBounds() == sf::Rect2f({101, 205}, {33, 13}));
        }

        SECTION("Add strikethrough")
        {
            text.setStrikeThrough(true);
            CHECK(text.getLocalBounds() == sf::Rect2f({1, 5}, {33, 13}));
            CHECK(text.getGlobalBounds() == sf::Rect2f({101, 205}, {33, 13}));
        }

        SECTION("Change rotation")
        {
            text.rotation = sf::degrees(180);
            CHECK(text.getLocalBounds() == sf::Rect2f({1, 5}, {33, 13}));
            CHECK(text.getGlobalBounds() == Approx(sf::Rect2f({66, 182}, {33, 13})));
        }
    }

#if defined(SFML_ENABLE_LIFETIME_TRACKING)
    SECTION("Lifetime tracking")
    {
        SECTION("Return local from function")
        {
            const auto badFunction = []
            {
                const auto localFont = sf::Font::openFromFile("tuffy.ttf").value();
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
                BadStruct() : memberFont{sf::Font::openFromFile("tuffy.ttf").value()}, memberText{memberFont, {}}
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

            auto sb0 = sf::Font::openFromFile("tuffy.ttf").value();
            CHECK(!guard.fatalErrorTriggered("Font"));

            sf::Text s0(sb0, {});
            CHECK(!guard.fatalErrorTriggered("Font"));

            sb0 = sf::Font::openFromFile("tuffy.ttf").value();
            CHECK(!guard.fatalErrorTriggered("Font"));
        }

        SECTION("Copy assignment same dependee")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"Font"};
            CHECK(!guard.fatalErrorTriggered("Font"));

            auto     f = sf::Font::openFromFile("tuffy.ttf").value();
            sf::Text a(f, {});
            sf::Text b(f, {});
            CHECK(!guard.fatalErrorTriggered("Font"));

            a = b;
            CHECK(!guard.fatalErrorTriggered("Font"));
        }

        SECTION("Copy assignment different dependee")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"Font"};
            CHECK(!guard.fatalErrorTriggered("Font"));

            auto     f1 = sf::Font::openFromFile("tuffy.ttf").value();
            auto     f2 = sf::Font::openFromFile("tuffy.ttf").value();
            sf::Text a(f1, {});
            sf::Text b(f2, {});
            CHECK(!guard.fatalErrorTriggered("Font"));

            a = b;
            CHECK(!guard.fatalErrorTriggered("Font"));
        }

        SECTION("Repeated copy assignment does not inflate count")
        {
            const sf::priv::LifetimeDependee::TestingModeGuard guard{"Font"};
            CHECK(!guard.fatalErrorTriggered("Font"));

            auto     f = sf::Font::openFromFile("tuffy.ttf").value();
            sf::Text a(f, {});
            sf::Text b(f, {});
            CHECK(!guard.fatalErrorTriggered("Font"));

            a = b;
            a = b;
            a = b;
            CHECK(!guard.fatalErrorTriggered("Font"));
        }
    }
#endif
}
