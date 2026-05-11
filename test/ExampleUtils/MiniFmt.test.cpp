#include "StringifySfBaseStringUtil.hpp" // IWYU pragma: keep
#include "StringifyStringViewUtil.hpp"   // IWYU pragma: keep

#include "ExampleUtils/MiniFmt.hpp"

#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"

#include <Doctest.hpp>


namespace
{
namespace MiniFmtTest // for unity builds
{
////////////////////////////////////////////////////////////
/// Helper: capture the result of `formatIntoBuffer` as a `base::StringView`
/// over `buffer`, returning a zero-length view on failure.
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] sf::base::StringView formatToView(char (&buffer)[512],
                                                typename minifmt::NonDeduced<const minifmt::FormatString<Args...>>::type fmt,
                                                const Args&... args)
{
    char* const end = minifmt::formatIntoBuffer(buffer, fmt, args...);
    if (end == nullptr)
        return {};
    return sf::base::StringView{buffer, static_cast<sf::base::SizeT>(end - buffer)};
}

} // namespace MiniFmtTest
} // namespace


using namespace MiniFmtTest;


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - format() basics")
{
    SECTION("No args, plain text")
    {
        CHECK(minifmt::format("hello") == sf::base::String{"hello"});
        CHECK(minifmt::format("") == sf::base::String{""});
    }

    SECTION("Single integer arg")
    {
        CHECK(minifmt::format("{}", 0) == sf::base::String{"0"});
        CHECK(minifmt::format("{}", 42) == sf::base::String{"42"});
        CHECK(minifmt::format("{}", -7) == sf::base::String{"-7"});
        CHECK(minifmt::format("[{}]", 100) == sf::base::String{"[100]"});
    }

    SECTION("Single floating-point arg (default precision = 6)")
    {
        CHECK(minifmt::format("{}", 3.5) == sf::base::String{"3.500000"});
        CHECK(minifmt::format("{}", -0.25) == sf::base::String{"-0.250000"});
    }

    SECTION("Single string-like arg")
    {
        CHECK(minifmt::format("{}", "world") == sf::base::String{"world"});
        CHECK(minifmt::format("{}", sf::base::StringView{"hello"}) == sf::base::String{"hello"});
        CHECK(minifmt::format("{}", sf::base::String{"abc"}) == sf::base::String{"abc"});
    }

    SECTION("Multiple mixed args")
    {
        CHECK(minifmt::format("{} = {}", "x", 42) == sf::base::String{"x = 42"});
        CHECK(minifmt::format("{}, {}, {}", 1, 2, 3) == sf::base::String{"1, 2, 3"});
        CHECK(minifmt::format("{} + {} = {}", 2, 3, 5) == sf::base::String{"2 + 3 = 5"});
    }

    SECTION("Brace escapes")
    {
        CHECK(minifmt::format("{{}}") == sf::base::String{"{}"});
        CHECK(minifmt::format("{{{}}}", 7) == sf::base::String{"{7}"});
        CHECK(minifmt::format("a{{b}}c") == sf::base::String{"a{b}c"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - precision spec {:.Nf}")
{
    SECTION("Float precision")
    {
        CHECK(minifmt::format("{:.0f}", 3.7) == sf::base::String{"4"});
        CHECK(minifmt::format("{:.1f}", 3.14) == sf::base::String{"3.1"});
        CHECK(minifmt::format("{:.2f}", 3.14) == sf::base::String{"3.14"});
        CHECK(minifmt::format("{:.3f}", 3.14159) == sf::base::String{"3.142"});
        CHECK(minifmt::format("{:.6f}", 1.0) == sf::base::String{"1.000000"});
    }

    SECTION("Negative floats")
    {
        CHECK(minifmt::format("{:.1f}", -2.5) == sf::base::String{"-2.5"});
        CHECK(minifmt::format("{:.3f}", -0.001) == sf::base::String{"-0.001"});
    }

    SECTION("Float precision without `f` tag works the same")
    {
        CHECK(minifmt::format("{:.2}", 3.14159) == sf::base::String{"3.14"});
    }

    SECTION("Integer args ignore precision (no-op)")
    {
        CHECK(minifmt::format("{:.3}", 42) == sf::base::String{"42"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - width spec {:N}")
{
    SECTION("Numbers default to right-align")
    {
        CHECK(minifmt::format("[{:5}]", 42) == sf::base::String{"[   42]"});
        CHECK(minifmt::format("[{:3}]", 1) == sf::base::String{"[  1]"});
    }

    SECTION("Strings default to left-align")
    {
        CHECK(minifmt::format("[{:5}]", "hi") == sf::base::String{"[hi   ]"});
        CHECK(minifmt::format("[{:6}]", sf::base::StringView{"abc"}) == sf::base::String{"[abc   ]"});
    }

    SECTION("Content equal to width -- no padding")
    {
        CHECK(minifmt::format("[{:3}]", 123) == sf::base::String{"[123]"});
        CHECK(minifmt::format("[{:3}]", "abc") == sf::base::String{"[abc]"});
    }

    SECTION("Content wider than width -- no truncation")
    {
        CHECK(minifmt::format("[{:3}]", 12'345) == sf::base::String{"[12345]"});
        CHECK(minifmt::format("[{:2}]", "long string") == sf::base::String{"[long string]"});
    }

    SECTION("Width of zero (degenerate but valid)")
    {
        CHECK(minifmt::format("[{:0}]", 42) == sf::base::String{"[42]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - explicit alignment {:<N}, {:>N}, {:^N}")
{
    SECTION("Right-align '>'")
    {
        CHECK(minifmt::format("[{:>5}]", "hi") == sf::base::String{"[   hi]"});
        CHECK(minifmt::format("[{:>5}]", 42) == sf::base::String{"[   42]"});
    }

    SECTION("Left-align '<'")
    {
        CHECK(minifmt::format("[{:<5}]", "hi") == sf::base::String{"[hi   ]"});
        CHECK(minifmt::format("[{:<5}]", 42) == sf::base::String{"[42   ]"});
    }

    SECTION("Center align '^'")
    {
        // Even-width slack splits evenly (extra goes right).
        CHECK(minifmt::format("[{:^6}]", "hi") == sf::base::String{"[  hi  ]"});
        // Odd-width slack: extra goes to right side.
        CHECK(minifmt::format("[{:^5}]", "hi") == sf::base::String{"[ hi  ]"});
        CHECK(minifmt::format("[{:^5}]", 1) == sf::base::String{"[  1  ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - custom fill character")
{
    SECTION("Zero-pad on the left")
    {
        CHECK(minifmt::format("[{:0>5}]", 42) == sf::base::String{"[00042]"});
        CHECK(minifmt::format("[{:0>5}]", 7) == sf::base::String{"[00007]"});
    }

    SECTION("Dot-fill on the left")
    {
        CHECK(minifmt::format("[{:.>5}]", "ab") == sf::base::String{"[...ab]"});
    }

    SECTION("Dash-center-fill")
    {
        CHECK(minifmt::format("[{:-^7}]", "hi") == sf::base::String{"[--hi---]"});
    }

    SECTION("Custom fill on left-align")
    {
        CHECK(minifmt::format("[{:*<5}]", "ab") == sf::base::String{"[ab***]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - combined specs (width + precision)")
{
    SECTION("Width + precision, right-aligned")
    {
        CHECK(minifmt::format("[{:>10.3f}]", 3.14159) == sf::base::String{"[     3.142]"});
    }

    SECTION("Width + precision, zero-padded")
    {
        CHECK(minifmt::format("[{:0>10.2f}]", 3.14) == sf::base::String{"[0000003.14]"});
    }

    SECTION("Width + precision, left-aligned")
    {
        CHECK(minifmt::format("[{:<10.2f}]", 3.14) == sf::base::String{"[3.14      ]"});
    }

    SECTION("Width + precision, center-aligned")
    {
        CHECK(minifmt::format("[{:^10.2f}]", 3.14) == sf::base::String{"[   3.14   ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - mixed-spec multi-arg")
{
    const auto result = minifmt::format("[{:>5}|{:<5}|{:^5}]", 1, 2, 3);
    CHECK(result == sf::base::String{"[    1|2    |  3  ]"});

    const auto hud = minifmt::format("FPS: {:>5.1f} | Frame: {:>4}ms | Cells: {:0>4}", 59.95, 16, 42);
    CHECK(hud == sf::base::String{"FPS:  60.0 | Frame:   16ms | Cells: 0042"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - integer type variety")
{
    SECTION("signed/unsigned")
    {
        CHECK(minifmt::format("{}", -32'000) == sf::base::String{"-32000"});
        CHECK(minifmt::format("{}", static_cast<unsigned>(4'000'000'000u)) == sf::base::String{"4000000000"});

        // Narrow signed types must survive `toChars` integer-promotion path
        CHECK(minifmt::format("{}", static_cast<short>(-32'000)) == sf::base::String{"-32000"});
        CHECK(minifmt::format("{}", static_cast<signed char>(-100)) == sf::base::String{"-100"});
    }

    SECTION("long long")
    {
        CHECK(minifmt::format("{}", static_cast<long long>(-1'234'567'890'123LL)) ==
              sf::base::String{"-123456789012"
                               "3"});
        CHECK(minifmt::format("{}", static_cast<unsigned long long>(18'000'000'000'000ULL)) ==
              sf::base::String{"180000"
                               "000000"
                               "00"});
    }

    SECTION("char dispatches as integer")
    {
        // `char` is integral; default formatting prints decimal code, not glyph.
        // We just check it succeeds; exact behavior is implementation-defined-signedness.
        char        buf[16];
        const auto* end = minifmt::formatIntoBuffer(buf, "{}", 'A');
        CHECK(end != nullptr);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - formatIntoBuffer raw API")
{
    char buf[512];

    SECTION("Returns end pointer on success")
    {
        const auto* end = minifmt::formatIntoBuffer(buf, "x={}", 42);
        REQUIRE(end != nullptr);
        CHECK(sf::base::StringView{buf, static_cast<sf::base::SizeT>(end - buf)} == sf::base::StringView{"x=42"});
        CHECK(*end == '\0'); // null-terminated
    }

    SECTION("Returns nullptr on buffer overflow")
    {
        char small[4];
        // "hello" + null = 6 bytes, doesn't fit in 4.
        CHECK(minifmt::formatIntoBuffer(small, "hello") == nullptr);
    }

    SECTION("Returns nullptr when arg formatting overflows")
    {
        char small[4];
        CHECK(minifmt::formatIntoBuffer(small, "{}", 1'234'567) == nullptr);
    }

    SECTION("Helper produces same result as format()")
    {
        const auto view = formatToView(buf, "Hello, {}!", "world");
        CHECK(view == sf::base::StringView{"Hello, world!"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - formatTo() into base::String")
{
    SECTION("Appends to non-empty sink")
    {
        sf::base::String s = "prefix:";
        minifmt::formatTo(s, " {} = {:.2f}", "pi", 3.14159);
        CHECK(s == sf::base::String{"prefix: pi = 3.14"});
    }

    SECTION("Format into fresh sink")
    {
        sf::base::String s;
        minifmt::formatTo(s, "{:>8}", "abc");
        CHECK(s == sf::base::String{"     abc"});
    }

    SECTION("Repeated formatTo accumulates")
    {
        sf::base::String s;
        minifmt::formatTo(s, "{}-", 1);
        minifmt::formatTo(s, "{}-", 2);
        minifmt::formatTo(s, "{}", 3);
        CHECK(s == sf::base::String{"1-2-3"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - formatTo() into Utf8String")
{
    SECTION("Build a HUD-style string")
    {
        sf::Utf8String text;
        minifmt::formatTo(text, "Frame: {:>5}ms / Delta: {:.2f}", 16, 0.016667);

        const auto bytes = text.asBytes();
        CHECK(sf::base::StringView{bytes.data(), bytes.size()} == sf::base::StringView{"Frame:    16ms / Delta: 0.02"});
    }

    SECTION("Appends to a Utf8String already containing UTF-8 content")
    {
        sf::Utf8String text{u8"café "};
        minifmt::formatTo(text, "= {}", 42);

        const auto bytes = text.asBytes();
        CHECK(sf::base::StringView{bytes.data(), bytes.size()} == sf::base::StringView{"café = 42"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[ExampleUtils] MiniFmt.hpp - edge cases")
{
    SECTION("Empty format string")
    {
        CHECK(minifmt::format("") == sf::base::String{""});
    }

    SECTION("Only escapes")
    {
        CHECK(minifmt::format("{{") == sf::base::String{"{"});
        CHECK(minifmt::format("}}") == sf::base::String{"}"});
        CHECK(minifmt::format("{{{{}}}}") == sf::base::String{"{{}}"});
    }

    SECTION("Empty string arg with width")
    {
        CHECK(minifmt::format("[{:4}]", "") == sf::base::String{"[    ]"});
        CHECK(minifmt::format("[{:>4}]", "") == sf::base::String{"[    ]"});
    }

    SECTION("Adjacent placeholders")
    {
        CHECK(minifmt::format("{}{}{}", 1, 2, 3) == sf::base::String{"123"});
    }
}
