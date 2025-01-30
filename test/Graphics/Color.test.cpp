#include "SFML/Graphics/Color.hpp"

#include "SFML/Base/TrivialVector.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>


TEST_CASE("[Graphics] sf::Color")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Color));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::Color)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::Color));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::Color, sf::Color));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            constexpr sf::Color color{};
            STATIC_CHECK(color.r == 0);
            STATIC_CHECK(color.g == 0);
            STATIC_CHECK(color.b == 0);
            STATIC_CHECK(color.a == 255);
        }

        SECTION("(r, g, b) constructor")
        {
            constexpr sf::Color color(1, 2, 3);
            STATIC_CHECK(color.r == 1);
            STATIC_CHECK(color.g == 2);
            STATIC_CHECK(color.b == 3);
            STATIC_CHECK(color.a == 255);
        }

        SECTION("(r, g, b, a) constructor")
        {
            constexpr sf::Color color(1, 2, 3, 4);
            STATIC_CHECK(color.r == 1);
            STATIC_CHECK(color.g == 2);
            STATIC_CHECK(color.b == 3);
            STATIC_CHECK(color.a == 4);
        }

        SECTION("sf::base::I32 constructor")
        {
            STATIC_CHECK(sf::Color::fromRGBA(0x00000000) == sf::Color(0, 0, 0, 0));
            STATIC_CHECK(sf::Color::fromRGBA(0x01020304) == sf::Color(1, 2, 3, 4));
            STATIC_CHECK(sf::Color::fromRGBA(0xFFFFFFFF) == sf::Color(255, 255, 255, 255));
        }
    }

    SECTION("toInteger()")
    {
        STATIC_CHECK(sf::Color(0, 0, 0, 0).toInteger() == 0x00000000);
        STATIC_CHECK(sf::Color(1, 2, 3, 4).toInteger() == 0x01020304);
        STATIC_CHECK(sf::Color(255, 255, 255, 255).toInteger() == 0xFFFFFFFF);
    }

    SECTION("Operations")
    {
        SECTION("operator==")
        {
            STATIC_CHECK(sf::Color() == sf::Color());
            STATIC_CHECK(sf::Color(1, 2, 3, 4) == sf::Color(1, 2, 3, 4));

            STATIC_CHECK_FALSE(sf::Color(1, 0, 0, 0) == sf::Color(0, 0, 0, 0));
            STATIC_CHECK_FALSE(sf::Color(0, 1, 0, 0) == sf::Color(0, 0, 0, 0));
            STATIC_CHECK_FALSE(sf::Color(0, 0, 1, 0) == sf::Color(0, 0, 0, 0));
            STATIC_CHECK_FALSE(sf::Color(0, 0, 0, 1) == sf::Color(0, 0, 0, 0));
        }

        SECTION("operator!=")
        {
            STATIC_CHECK(sf::Color(1, 0, 0, 0) != sf::Color(0, 0, 0, 0));
            STATIC_CHECK(sf::Color(0, 1, 0, 0) != sf::Color(0, 0, 0, 0));
            STATIC_CHECK(sf::Color(0, 0, 1, 0) != sf::Color(0, 0, 0, 0));
            STATIC_CHECK(sf::Color(0, 0, 0, 1) != sf::Color(0, 0, 0, 0));

            STATIC_CHECK_FALSE(sf::Color() != sf::Color());
            STATIC_CHECK_FALSE(sf::Color(1, 2, 3, 4) != sf::Color(1, 2, 3, 4));
        }

        SECTION("operator+")
        {
            STATIC_CHECK(sf::Color(0, 0, 0, 0) + sf::Color(0, 0, 0, 0) == sf::Color(0, 0, 0, 0));
            STATIC_CHECK(sf::Color(50, 50, 50, 50) + sf::Color(50, 50, 50, 50) == sf::Color(100, 100, 100, 100));
            STATIC_CHECK(sf::Color(100, 100, 100, 100) + sf::Color(100, 100, 100, 100) == sf::Color(200, 200, 200, 200));
            STATIC_CHECK(sf::Color(150, 150, 150, 150) + sf::Color(150, 150, 150, 150) == sf::Color(255, 255, 255, 255));
            STATIC_CHECK(sf::Color(255, 255, 255, 255) + sf::Color(255, 255, 255, 255) == sf::Color(255, 255, 255, 255));
        }

        SECTION("operator-")
        {
            constexpr sf::Color c(50, 50, 50, 50);
            constexpr sf::Color c2(150, 150, 150, 150);
            STATIC_CHECK(c2 - c == sf::Color(100, 100, 100, 100));
            STATIC_CHECK(c - c2 == sf::Color(0, 0, 0, 0));
        }

        SECTION("operator*")
        {
            constexpr sf::Color c(255, 255, 255, 255);
            constexpr sf::Color c2(2, 2, 2, 2);
            STATIC_CHECK(c * c2 == sf::Color(2, 2, 2, 2));
            STATIC_CHECK(c2 * c == sf::Color(2, 2, 2, 2));
        }

        SECTION("operator+=")
        {
            sf::Color color(42, 42, 42, 42);
            color += sf::Color(1, 1, 1, 1);
            CHECK(color == sf::Color(43, 43, 43, 43));
            color += sf::Color(250, 250, 250, 250);
            CHECK(color == sf::Color(255, 255, 255, 255));
        }

        SECTION("operator-=")
        {
            sf::Color color(248, 248, 248, 248);
            color -= sf::Color(1, 1, 1, 1);
            CHECK(color == sf::Color(247, 247, 247, 247));
            color -= sf::Color(250, 250, 250, 250);
            CHECK(color == sf::Color(0, 0, 0, 0));
        }

        SECTION("operator*=")
        {
            sf::Color color(50, 50, 50, 50);
            color *= sf::Color(20, 20, 20, 20);
            CHECK(color == sf::Color(3, 3, 3, 3));
            color *= sf::Color(120, 120, 120, 120);
            CHECK(color == sf::Color(1, 1, 1, 1));
        }
    }

    SECTION("Constants")
    {
        STATIC_CHECK(sf::Color::Black == sf::Color(0, 0, 0));
        STATIC_CHECK(sf::Color::White == sf::Color(255, 255, 255));
        STATIC_CHECK(sf::Color::Red == sf::Color(255, 0, 0));
        STATIC_CHECK(sf::Color::Green == sf::Color(0, 255, 0));
        STATIC_CHECK(sf::Color::Blue == sf::Color(0, 0, 255));
        STATIC_CHECK(sf::Color::Yellow == sf::Color(255, 255, 0));
        STATIC_CHECK(sf::Color::Magenta == sf::Color(255, 0, 255));
        STATIC_CHECK(sf::Color::Cyan == sf::Color(0, 255, 255));
        STATIC_CHECK(sf::Color::Transparent == sf::Color(0, 0, 0, 0));
    }

    SECTION("Reinterpret as sf::base::U8*")
    {
        STATIC_CHECK(sizeof(sf::Color) == 4);
        STATIC_CHECK(alignof(sf::Color) == 1);

        sf::base::TrivialVector<sf::Color> pixels(3);
        pixels[0] = {10, 11, 12, 13};
        pixels[1] = {14, 15, 16, 17};
        pixels[2] = {18, 19, 20, 21};

        const auto* begin = reinterpret_cast<const sf::base::U8*>(pixels.data());
        CHECK(begin[0] == pixels[0].r);
        CHECK(begin[1] == pixels[0].g);
        CHECK(begin[2] == pixels[0].b);
        CHECK(begin[3] == pixels[0].a);

        CHECK(begin[4] == pixels[1].r);
        CHECK(begin[5] == pixels[1].g);
        CHECK(begin[6] == pixels[1].b);
        CHECK(begin[7] == pixels[1].a);

        CHECK(begin[8] == pixels[2].r);
        CHECK(begin[9] == pixels[2].g);
        CHECK(begin[10] == pixels[2].b);
        CHECK(begin[11] == pixels[2].a);
    }


    SECTION("HSLtoColor converts basic colors correctly")
    {
        SECTION("Primary colors")
        {
            // Red (Hue = 0)
            CHECK(sf::Color::fromHSLA({0.0f, 1.0f, 0.5f}) == sf::Color(255, 0, 0));

            // Green (Hue = 120)
            CHECK(sf::Color::fromHSLA({120.0f, 1.0f, 0.5f}) == sf::Color(0, 255, 0));

            // Blue (Hue = 240)
            CHECK(sf::Color::fromHSLA({240.0f, 1.0f, 0.5f}) == sf::Color(0, 0, 255));
        }

        SECTION("Secondary colors")
        {
            // Yellow (Hue = 60)
            CHECK(sf::Color::fromHSLA({60.0f, 1.0f, 0.5f}) == sf::Color(255, 255, 0));

            // Cyan (Hue = 180)
            CHECK(sf::Color::fromHSLA({180.0f, 1.0f, 0.5f}) == sf::Color(0, 255, 255));

            // Magenta (Hue = 300)
            CHECK(sf::Color::fromHSLA({300.0f, 1.0f, 0.5f}) == sf::Color(255, 0, 255));
        }
    }

    SECTION("HSLtoColor handles edge cases correctly")
    {
        SECTION("Grayscale (Saturation = 0)")
        {
            // Black
            CHECK(sf::Color::fromHSLA({0.0f, 0.0f, 0.0f}) == sf::Color(0, 0, 0));

            // White
            CHECK(sf::Color::fromHSLA({0.0f, 0.0f, 1.0f}) == sf::Color(255, 255, 255));

            // 50% Gray (any hue)
            CHECK(sf::Color::fromHSLA({0.0f, 0.0f, 0.5f}) == sf::Color(128, 128, 128));
            CHECK(sf::Color::fromHSLA({180.0f, 0.0f, 0.5f}) == sf::Color(128, 128, 128));
        }
    }

    SECTION("HSLtoColor produces expected intermediate values")
    {
        SECTION("Pastels (High lightness, medium saturation)")
        {
            sf::Color pastelPink = sf::Color::fromHSLA({350.0f, 0.5f, 0.8f});
            CHECK(pastelPink.r > 200);
            CHECK(pastelPink.g > 150);
            CHECK(pastelPink.b > 150);
        }

        SECTION("Deep colors (Low lightness, high saturation)")
        {
            sf::Color deepBlue = sf::Color::fromHSLA({240.0f, 1.0f, 0.2f});
            CHECK(deepBlue.r < 50);
            CHECK(deepBlue.g < 50);
            CHECK(deepBlue.b > 100);
        }
    }

    SECTION("Hue wrapping")
    {
        // Negative hue should wrap to positive
        CHECK(sf::Color::fromHSLA({-120.0f, 1.0f, 0.5f}) == sf::Color::fromHSLA({240.0f, 1.0f, 0.5f}));

        // Hue > 360 should wrap
        CHECK(sf::Color::fromHSLA({480.0f, 1.0f, 0.5f}) == sf::Color::fromHSLA({120.0f, 1.0f, 0.5f}));
    }

    SECTION("Saturation and lightness clamping")
    {
        // Oversaturated should clamp to 1
        CHECK(sf::Color::fromHSLA({0.0f, 1.5f, 0.5f}) == sf::Color::fromHSLA({0.0f, 1.0f, 0.5f}));

        // Negative saturation should clamp to 0
        CHECK(sf::Color::fromHSLA({0.0f, -0.5f, 0.5f}) == sf::Color::fromHSLA({0.0f, 0.0f, 0.5f}));

        // Overlight should clamp to 1
        CHECK(sf::Color::fromHSLA({0.0f, 1.0f, 1.5f}) == sf::Color::fromHSLA({0.0f, 1.0f, 1.0f}));

        // Negative lightness should clamp to 0
        CHECK(sf::Color::fromHSLA({0.0f, 1.0f, -0.5f}) == sf::Color::fromHSLA({0.0f, 1.0f, 0.0f}));
    }

    SECTION("withHueMod modifier")
    {
        SECTION("Basic hue shifts")
        {
            // Red (0°) + 120° → Green (120°)
            sf::Color red   = sf::Color::Red;
            sf::Color green = red.withHueMod(120.0f);
            CHECK(green == sf::Color::Green);

            // Green (120°) - 120° → Red (0°)
            sf::Color greenColor = sf::Color::Green;
            sf::Color redColor   = greenColor.withHueMod(-120.0f);
            CHECK(redColor == sf::Color::Red);

            // Blue (240°) + 60° → 300° (Magenta)
            sf::Color blue    = sf::Color::Blue;
            sf::Color magenta = blue.withHueMod(60.0f);
            CHECK(magenta == sf::Color::Magenta);
        }

        SECTION("Hue wrapping")
        {
            // 350° + 20° → 10° (wrapped within [0, 360))
            sf::Color color        = sf::Color::fromHSLA({350.0f, 1.0f, 0.5f});
            sf::Color shiftedColor = color.withHueMod(20.0f);
            CHECK(shiftedColor == Approx(sf::Color::fromHSLA({10.0f, 1.0f, 0.5f})));

            // 30° - 50° → 340°
            color        = sf::Color::fromHSLA({30.0f, 1.0f, 0.5f});
            shiftedColor = color.withHueMod(-50.0f);
            CHECK(shiftedColor == sf::Color::fromHSLA({340.0f, 1.0f, 0.5f}));

            // 300° + 120° → 60° (Yellow)
            color        = sf::Color::fromHSLA({300.0f, 1.0f, 0.5f});
            shiftedColor = color.withHueMod(120.0f);
            CHECK(shiftedColor == sf::Color::fromHSLA({60.0f, 1.0f, 0.5f}));

            // 300° + 480° → (300 + 480) % 360 = 60°
            shiftedColor = color.withHueMod(480.0f);
            CHECK(shiftedColor == sf::Color::fromHSLA({60.0f, 1.0f, 0.5f}));
        }

        SECTION("Alpha preservation")
        {
            // Original alpha should remain unchanged
            sf::Color color{255, 0, 0, 128};
            sf::Color shiftedColor = color.withHueMod(120.0f);
            CHECK(shiftedColor.a == 128);
        }

        SECTION("Saturation and lightness unchanged")
        {
            // After hue shift, saturation and lightness should match original
            sf::Color      color        = sf::Color::fromHSLA({180.0f, 0.8f, 0.6f}, 255);
            sf::Color      shiftedColor = color.withHueMod(90.0f);
            sf::Color::HSL hsla         = shiftedColor.toHSL();
            CHECK(hsla.saturation == Approx(0.8039f));
            CHECK(hsla.lightness == Approx(0.6f));
            CHECK(hsla.hue == Approx(270.0f)); // 180° + 90°
        }

        SECTION("Edge cases")
        {
            // Shift by 0° (no change)
            sf::Color color     = sf::Color::Red;
            sf::Color sameColor = color.withHueMod(0.0f);
            CHECK(sameColor == color);

            // Shift by 360° (no change)
            sameColor = color.withHueMod(360.0f);
            CHECK(sameColor == color);

            // Shift by 720° (equivalent to 0°)
            sameColor = color.withHueMod(720.0f);
            CHECK(sameColor == color);

            // Negative shift wrapping (50° - 400° = -350° ≡ 10°)
            color                  = sf::Color::fromHSLA({50.0f, 1.0f, 0.5f});
            sf::Color shiftedColor = color.withHueMod(-400.0f);
            CHECK(shiftedColor == sf::Color::fromHSLA({10.0f, 1.0f, 0.5f}));
        }
    }
}
