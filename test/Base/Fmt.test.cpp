#include "StringifySfBaseStringUtil.hpp" // IWYU pragma: keep
#include "StringifyStringViewUtil.hpp"   // IWYU pragma: keep
#include "Tst/Tst.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"

#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Fmt/FmtArgDefaultAlign.hpp"
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/Fmt/FmtSpec.hpp"
#include "SFML/Base/Fmt/FmtToString.hpp"
#include "SFML/Base/NonDeduced.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"

#include <limits>


namespace
{
namespace FmtTest // for unity builds
{
////////////////////////////////////////////////////////////
/// Helper: capture the result of `fmtIntoBuffer` as a `base::StringView`
/// over `buffer`, returning a zero-length view on failure.
////////////////////////////////////////////////////////////
template <typename... Args>
[[nodiscard]] sf::base::StringView formatToView(char (&buffer)[512],
                                                typename sf::base::NonDeduced<const sf::base::FmtString<Args...>>::type fmt,
                                                const Args&... args)
{
    char* const end = sf::base::fmtIntoBuffer(buffer, fmt, args...);
    if (end == nullptr)
        return {};
    return sf::base::StringView{buffer, static_cast<sf::base::SizeT>(end - buffer)};
}


////////////////////////////////////////////////////////////
template <auto>
struct FmtStringAccepted
{
};


////////////////////////////////////////////////////////////
template <typename... Args>
concept AcceptsEmptyPrecision = requires {
    typename FmtStringAccepted<[] consteval
    {
        (void)sf::base::FmtString<Args...>{"{:.}"};
        return true;
    }()>;
};


////////////////////////////////////////////////////////////
template <typename... Args>
concept AcceptsEmptyPrecisionF = requires {
    typename FmtStringAccepted<[] consteval
    {
        (void)sf::base::FmtString<Args...>{"{:.f}"};
        return true;
    }()>;
};


static_assert(!AcceptsEmptyPrecision<double>);
static_assert(!AcceptsEmptyPrecisionF<double>);

} // namespace FmtTest
} // namespace


using namespace FmtTest;


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - format() basics")
{
    SECTION("No args, plain text")
    {
        CHECK(sf::base::fmtToString("hello") == sf::base::String{"hello"});
        CHECK(sf::base::fmtToString("") == sf::base::String{""});
    }

    SECTION("Single integer arg")
    {
        CHECK(sf::base::fmtToString("{}", 0) == sf::base::String{"0"});
        CHECK(sf::base::fmtToString("{}", 42) == sf::base::String{"42"});
        CHECK(sf::base::fmtToString("{}", -7) == sf::base::String{"-7"});
        CHECK(sf::base::fmtToString("[{}]", 100) == sf::base::String{"[100]"});
    }

    SECTION("Single floating-point arg (default precision = 6)")
    {
        CHECK(sf::base::fmtToString("{}", 3.5) == sf::base::String{"3.500000"});
        CHECK(sf::base::fmtToString("{}", -0.25) == sf::base::String{"-0.250000"});
    }

    SECTION("Every arithmetic type")
    {
        CHECK(sf::base::fmtToString("{}", false) == sf::base::String{"false"});
        CHECK(sf::base::fmtToString("{}", 'A') == sf::base::String{"A"});
        CHECK(sf::base::fmtToString("{}", static_cast<signed char>(-1)) == sf::base::String{"-1"});
        CHECK(sf::base::fmtToString("{}", static_cast<unsigned char>(1)) == sf::base::String{"1"});
        CHECK(sf::base::fmtToString("{}", static_cast<short>(-1)) == sf::base::String{"-1"});
        CHECK(sf::base::fmtToString("{}", static_cast<unsigned short>(1)) == sf::base::String{"1"});
        CHECK(sf::base::fmtToString("{}", -1) == sf::base::String{"-1"});
        CHECK(sf::base::fmtToString("{}", 1u) == sf::base::String{"1"});
        CHECK(sf::base::fmtToString("{}", -1l) == sf::base::String{"-1"});
        CHECK(sf::base::fmtToString("{}", 1ul) == sf::base::String{"1"});
        CHECK(sf::base::fmtToString("{}", -1ll) == sf::base::String{"-1"});
        CHECK(sf::base::fmtToString("{}", 1ull) == sf::base::String{"1"});
        CHECK(sf::base::fmtToString("{}", 1.f) == sf::base::String{"1.000000"});
        CHECK(sf::base::fmtToString("{}", 1.) == sf::base::String{"1.000000"});
        CHECK(sf::base::fmtToString("{}", 1.L) == sf::base::String{"1.000000"});
    }

    SECTION("Single string-like arg")
    {
        CHECK(sf::base::fmtToString("{}", "world") == sf::base::String{"world"});
        CHECK(sf::base::fmtToString("{}", sf::base::StringView{"hello"}) == sf::base::String{"hello"});
        CHECK(sf::base::fmtToString("{}", sf::base::String{"abc"}) == sf::base::String{"abc"});
    }

    SECTION("Multiple mixed args")
    {
        CHECK(sf::base::fmtToString("{} = {}", "x", 42) == sf::base::String{"x = 42"});
        CHECK(sf::base::fmtToString("{}, {}, {}", 1, 2, 3) == sf::base::String{"1, 2, 3"});
        CHECK(sf::base::fmtToString("{} + {} = {}", 2, 3, 5) == sf::base::String{"2 + 3 = 5"});
    }

    SECTION("Brace escapes")
    {
        CHECK(sf::base::fmtToString("{{}}") == sf::base::String{"{}"});
        CHECK(sf::base::fmtToString("{{{}}}", 7) == sf::base::String{"{7}"});
        CHECK(sf::base::fmtToString("a{{b}}c") == sf::base::String{"a{b}c"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - precision spec {:.Nf}")
{
    SECTION("Float precision")
    {
        CHECK(sf::base::fmtToString("{:.0f}", 3.7) == sf::base::String{"4"});
        CHECK(sf::base::fmtToString("{:.1f}", 3.14) == sf::base::String{"3.1"});
        CHECK(sf::base::fmtToString("{:.2f}", 3.14) == sf::base::String{"3.14"});
        CHECK(sf::base::fmtToString("{:.3f}", 3.14159) == sf::base::String{"3.142"});
        CHECK(sf::base::fmtToString("{:.6f}", 1.0) == sf::base::String{"1.000000"});
    }

    SECTION("Negative floats")
    {
        CHECK(sf::base::fmtToString("{:.1f}", -2.5) == sf::base::String{"-2.5"});
        CHECK(sf::base::fmtToString("{:.3f}", -0.001) == sf::base::String{"-0.001"});
    }

    SECTION("Float precision without `f` tag works the same")
    {
        CHECK(sf::base::fmtToString("{:.2}", 3.14159) == sf::base::String{"3.14"});
    }

    // Precision is only valid for floating-point arguments. Integer/string
    // formatters assert on precision in debug builds.
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - width spec {:N}")
{
    SECTION("Numbers default to right-align")
    {
        CHECK(sf::base::fmtToString("[{:5}]", 42) == sf::base::String{"[   42]"});
        CHECK(sf::base::fmtToString("[{:3}]", 1) == sf::base::String{"[  1]"});
    }

    SECTION("Strings default to left-align")
    {
        CHECK(sf::base::fmtToString("[{:5}]", "hi") == sf::base::String{"[hi   ]"});
        CHECK(sf::base::fmtToString("[{:6}]", sf::base::StringView{"abc"}) == sf::base::String{"[abc   ]"});
    }

    SECTION("Content equal to width -- no padding")
    {
        CHECK(sf::base::fmtToString("[{:3}]", 123) == sf::base::String{"[123]"});
        CHECK(sf::base::fmtToString("[{:3}]", "abc") == sf::base::String{"[abc]"});
    }

    SECTION("Content wider than width -- no truncation")
    {
        CHECK(sf::base::fmtToString("[{:3}]", 12'345) == sf::base::String{"[12345]"});
        CHECK(sf::base::fmtToString("[{:2}]", "long string") == sf::base::String{"[long string]"});
    }

    SECTION("Width of zero (degenerate but valid)")
    {
        CHECK(sf::base::fmtToString("[{:0}]", 42) == sf::base::String{"[42]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - explicit alignment {:<N}, {:>N}, {:^N}")
{
    SECTION("Right-align '>'")
    {
        CHECK(sf::base::fmtToString("[{:>5}]", "hi") == sf::base::String{"[   hi]"});
        CHECK(sf::base::fmtToString("[{:>5}]", 42) == sf::base::String{"[   42]"});
    }

    SECTION("Left-align '<'")
    {
        CHECK(sf::base::fmtToString("[{:<5}]", "hi") == sf::base::String{"[hi   ]"});
        CHECK(sf::base::fmtToString("[{:<5}]", 42) == sf::base::String{"[42   ]"});
    }

    SECTION("Center align '^'")
    {
        // Even-width slack splits evenly (extra goes right).
        CHECK(sf::base::fmtToString("[{:^6}]", "hi") == sf::base::String{"[  hi  ]"});
        // Odd-width slack: extra goes to right side.
        CHECK(sf::base::fmtToString("[{:^5}]", "hi") == sf::base::String{"[ hi  ]"});
        CHECK(sf::base::fmtToString("[{:^5}]", 1) == sf::base::String{"[  1  ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fill character")
{
    SECTION("Zero-pad on the left")
    {
        CHECK(sf::base::fmtToString("[{:0>5}]", 42) == sf::base::String{"[00042]"});
        CHECK(sf::base::fmtToString("[{:0>5}]", 7) == sf::base::String{"[00007]"});
    }

    SECTION("Dot-fill on the left")
    {
        CHECK(sf::base::fmtToString("[{:.>5}]", "ab") == sf::base::String{"[...ab]"});
    }

    SECTION("Dash-center-fill")
    {
        CHECK(sf::base::fmtToString("[{:-^7}]", "hi") == sf::base::String{"[--hi---]"});
    }

    SECTION("Custom fill on left-align")
    {
        CHECK(sf::base::fmtToString("[{:*<5}]", "ab") == sf::base::String{"[ab***]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - combined specs (width + precision)")
{
    SECTION("Width + precision, right-aligned")
    {
        CHECK(sf::base::fmtToString("[{:>10.3f}]", 3.14159) == sf::base::String{"[     3.142]"});
    }

    SECTION("Width + precision, zero-padded")
    {
        CHECK(sf::base::fmtToString("[{:0>10.2f}]", 3.14) == sf::base::String{"[0000003.14]"});
    }

    SECTION("Width + precision, left-aligned")
    {
        CHECK(sf::base::fmtToString("[{:<10.2f}]", 3.14) == sf::base::String{"[3.14      ]"});
    }

    SECTION("Width + precision, center-aligned")
    {
        CHECK(sf::base::fmtToString("[{:^10.2f}]", 3.14) == sf::base::String{"[   3.14   ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - integer radix specs {:x}, {:X}, {:o}, {:b}, {:d}")
{
    SECTION("Lowercase hex")
    {
        CHECK(sf::base::fmtToString("{:x}", 0xAB'CDu) == sf::base::String{"abcd"});
        CHECK(sf::base::fmtToString("{:x}", 0u) == sf::base::String{"0"});
        CHECK(sf::base::fmtToString("{:x}", 0xDE'AD'BE'EFu) == sf::base::String{"deadbeef"});
    }

    SECTION("Uppercase hex")
    {
        CHECK(sf::base::fmtToString("{:X}", 0xAB'CDu) == sf::base::String{"ABCD"});
        CHECK(sf::base::fmtToString("{:X}", 0xDE'AD'BE'EFu) == sf::base::String{"DEADBEEF"});
    }

    SECTION("Octal")
    {
        CHECK(sf::base::fmtToString("{:o}", 8u) == sf::base::String{"10"});
        CHECK(sf::base::fmtToString("{:o}", 0u) == sf::base::String{"0"});
        CHECK(sf::base::fmtToString("{:o}", 511u) == sf::base::String{"777"});
    }

    SECTION("Binary")
    {
        CHECK(sf::base::fmtToString("{:b}", 0u) == sf::base::String{"0"});
        CHECK(sf::base::fmtToString("{:b}", 5u) == sf::base::String{"101"});
        CHECK(sf::base::fmtToString("{:b}", 0xFFu) == sf::base::String{"11111111"});
    }

    SECTION("Explicit decimal tag matches default")
    {
        CHECK(sf::base::fmtToString("{:d}", 42) == sf::base::String{"42"});
        CHECK(sf::base::fmtToString("{:d}", -42) == sf::base::String{"-42"});
    }

    SECTION("Negative signed: hex/oct/bin emit the bit pattern, no sign")
    {
        // 32-bit `int`: two's-complement of -1 is 0xFFFFFFFF.
        CHECK(sf::base::fmtToString("{:x}", -1) == sf::base::String{"ffffffff"});
        CHECK(sf::base::fmtToString("{:X}", -1) == sf::base::String{"FFFFFFFF"});
        CHECK(sf::base::fmtToString("{:b}", static_cast<signed char>(-1)) == sf::base::String{"11111111"});
    }

    SECTION("Width + fill: zero-padded hex scancode (Keyboard.cpp use case)")
    {
        CHECK(sf::base::fmtToString("{:0>4x}", 0x1Bu) == sf::base::String{"001b"});
        CHECK(sf::base::fmtToString("{:0>4X}", 0x1Bu) == sf::base::String{"001B"});
        CHECK(sf::base::fmtToString("{:0>8b}", 0xAu) == sf::base::String{"00001010"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - mixed-spec multi-arg")
{
    const auto result = sf::base::fmtToString("[{:>5}|{:<5}|{:^5}]", 1, 2, 3);
    CHECK(result == sf::base::String{"[    1|2    |  3  ]"});

    const auto hud = sf::base::fmtToString("FPS: {:>5.1f} | Frame: {:>4}ms | Cells: {:0>4}", 59.95, 16, 42);
    CHECK(hud == sf::base::String{"FPS:  60.0 | Frame:   16ms | Cells: 0042"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - integer type variety")
{
    SECTION("signed/unsigned")
    {
        CHECK(sf::base::fmtToString("{}", -32'000) == sf::base::String{"-32000"});
        CHECK(sf::base::fmtToString("{}", static_cast<unsigned>(4'000'000'000u)) == sf::base::String{"4000000000"});

        // Narrow signed types must survive `toChars` integer-promotion path
        CHECK(sf::base::fmtToString("{}", static_cast<short>(-32'000)) == sf::base::String{"-32000"});
        CHECK(sf::base::fmtToString("{}", static_cast<signed char>(-100)) == sf::base::String{"-100"});
    }

    SECTION("long long")
    {
        CHECK(sf::base::fmtToString("{}", static_cast<long long>(-1'234'567'890'123LL)) ==
              sf::base::String{"-123456789012"
                               "3"});
        CHECK(sf::base::fmtToString("{}", static_cast<unsigned long long>(18'000'000'000'000ULL)) ==
              sf::base::String{"180000"
                               "000000"
                               "00"});
    }

    SECTION("char formats as a glyph by default (matches libfmt)")
    {
        CHECK(sf::base::fmtToString("{}", 'A') == sf::base::String{"A"});
        CHECK(sf::base::fmtToString("{}", '\n') == sf::base::String{"\n"});
        CHECK(sf::base::fmtToString("[{}]", '"') == sf::base::String{"[\"]"});
        CHECK(sf::base::fmtToString("{}{}{}", 'a', 'b', 'c') == sf::base::String{"abc"});
    }

    SECTION("`:c` on char also emits the glyph")
    {
        CHECK(sf::base::fmtToString("{:c}", 'A') == sf::base::String{"A"});
        CHECK(sf::base::fmtToString("{:c}", '\n') == sf::base::String{"\n"});
    }

    SECTION("`:d` on char emits the numeric code")
    {
        CHECK(sf::base::fmtToString("{:d}", 'A') == sf::base::String{"65"});
        CHECK(sf::base::fmtToString("{:d}", '0') == sf::base::String{"48"});
    }

    SECTION("`:x` / `:X` / `:o` / `:b` on char emit the numeric code in that radix")
    {
        CHECK(sf::base::fmtToString("{:x}", 'A') == sf::base::String{"41"});
        CHECK(sf::base::fmtToString("{:X}", 'A') == sf::base::String{"41"});
        CHECK(sf::base::fmtToString("{:o}", 'A') == sf::base::String{"101"});
        CHECK(sf::base::fmtToString("{:b}", 'A') == sf::base::String{"1000001"});
    }

    SECTION("`:c` on an integer emits the lowest byte as a glyph")
    {
        CHECK(sf::base::fmtToString("{:c}", 65) == sf::base::String{"A"});
        CHECK(sf::base::fmtToString("{:c}", 0x41u) == sf::base::String{"A"});
    }

    SECTION("char default alignment is left, like a string")
    {
        CHECK(sf::base::fmtToString("[{:3}]", 'A') == sf::base::String{"[A  ]"});
        CHECK(sf::base::fmtToString("[{:>3}]", 'A') == sf::base::String{"[  A]"});
        CHECK(sf::base::fmtToString("[{:^3}]", 'A') == sf::base::String{"[ A ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - bool formats as 'true'/'false'")
{
    SECTION("Default presentation matches std::format / fmt::format")
    {
        CHECK(sf::base::fmtToString("{}", true) == sf::base::String{"true"});
        CHECK(sf::base::fmtToString("{}", false) == sf::base::String{"false"});
    }

    SECTION("Multiple bools in one format string")
    {
        CHECK(sf::base::fmtToString("{}/{}", true, false) == sf::base::String{"true/false"});
        CHECK(sf::base::fmtToString("a={} b={}", false, true) == sf::base::String{"a=false b=true"});
    }

    SECTION("Default alignment is left (matches string-like types)")
    {
        CHECK(sf::base::fmtToString("[{:8}]", true) == sf::base::String{"[true    ]"});
        CHECK(sf::base::fmtToString("[{:8}]", false) == sf::base::String{"[false   ]"});
    }

    SECTION("Explicit alignment overrides the default")
    {
        CHECK(sf::base::fmtToString("[{:>8}]", true) == sf::base::String{"[    true]"});
        CHECK(sf::base::fmtToString("[{:>8}]", false) == sf::base::String{"[   false]"});
        CHECK(sf::base::fmtToString("[{:^8}]", true) == sf::base::String{"[  true  ]"});
    }

    SECTION("Custom fill character")
    {
        CHECK(sf::base::fmtToString("[{:.>8}]", true) == sf::base::String{"[....true]"});
        CHECK(sf::base::fmtToString("[{:*<8}]", false) == sf::base::String{"[false***]"});
    }

    SECTION("Width equal to or smaller than content -- no padding")
    {
        CHECK(sf::base::fmtToString("[{:4}]", true) == sf::base::String{"[true]"});
        CHECK(sf::base::fmtToString("[{:3}]", false) == sf::base::String{"[false]"}); // wider than width
    }

    SECTION("Numeric form available via explicit int cast")
    {
        CHECK(sf::base::fmtToString("{}/{}", static_cast<int>(true), static_cast<int>(false)) ==
              sf::base::String{"1/"
                               "0"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtIntoBuffer raw API")
{
    char buf[512];

    SECTION("Returns end pointer on success (no implicit null terminator)")
    {
        const auto* end = sf::base::fmtIntoBuffer(buf, "x={}", 42);
        REQUIRE(end != nullptr);
        CHECK(sf::base::StringView{buf, static_cast<sf::base::SizeT>(end - buf)} == sf::base::StringView{"x=42"});
    }

    SECTION("Returns nullptr on buffer overflow")
    {
        char small[4];
        // "hello" is 5 bytes, doesn't fit in 4.
        CHECK(sf::base::fmtIntoBuffer(small, "hello") == nullptr);
    }

    SECTION("Caller may write their own null terminator")
    {
        // Reserve an extra byte for '\0' and write it manually.
        char        cstr[16];
        auto* const end = sf::base::fmtIntoBuffer(cstr, sizeof(cstr) - 1u, "n={}", 7);
        REQUIRE(end != nullptr);
        *end = '\0';
        CHECK(sf::base::StringView{cstr} == sf::base::StringView{"n=7"});
    }

    SECTION("Returns nullptr when arg formatting overflows")
    {
        char small[4];
        CHECK(sf::base::fmtIntoBuffer(small, "{}", 1'234'567) == nullptr);
    }

    SECTION("Helper produces same result as format()")
    {
        const auto view = formatToView(buf, "Hello, {}!", "world");
        CHECK(view == sf::base::StringView{"Hello, world!"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtTo() into base::String")
{
    SECTION("Appends to non-empty sink")
    {
        sf::base::String s = "prefix:";
        (void)sf::base::fmtTo(s, " {} = {:.2f}", "pi", 3.14159);
        CHECK(s == sf::base::String{"prefix: pi = 3.14"});
    }

    SECTION("Format into fresh sink")
    {
        sf::base::String s;
        (void)sf::base::fmtTo(s, "{:>8}", "abc");
        CHECK(s == sf::base::String{"     abc"});
    }

    SECTION("Repeated fmtTo accumulates")
    {
        sf::base::String s;
        (void)sf::base::fmtTo(s, "{}-", 1);
        (void)sf::base::fmtTo(s, "{}-", 2);
        (void)sf::base::fmtTo(s, "{}", 3);
        CHECK(s == sf::base::String{"1-2-3"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtTo() into Utf8String")
{
    SECTION("Build a HUD-style string")
    {
        sf::Utf8String text;
        CHECK(sf::base::fmtTo(text, "Frame: {:>5}ms / Delta: {:.2f}", 16, 0.016667) == sf::base::FmtResult::Ok);

        const auto bytes = text.asBytes();
        CHECK(sf::base::StringView{bytes.data(), bytes.size()} == sf::base::StringView{"Frame:    16ms / Delta: 0.02"});
    }

    SECTION("Appends to a Utf8String already containing UTF-8 content")
    {
        sf::Utf8String text{u8"café "};
        CHECK(sf::base::fmtTo(text, "= {}", 42) == sf::base::FmtResult::Ok);

        const auto bytes = text.asBytes();
        CHECK(sf::base::StringView{bytes.data(), bytes.size()} == sf::base::StringView{"café = 42"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - edge cases")
{
    SECTION("Empty format string")
    {
        CHECK(sf::base::fmtToString("") == sf::base::String{""});
    }

    SECTION("Only escapes")
    {
        CHECK(sf::base::fmtToString("{{") == sf::base::String{"{"});
        CHECK(sf::base::fmtToString("}}") == sf::base::String{"}"});
        CHECK(sf::base::fmtToString("{{{{}}}}") == sf::base::String{"{{}}"});
    }

    SECTION("Empty string arg with width")
    {
        CHECK(sf::base::fmtToString("[{:4}]", "") == sf::base::String{"[    ]"});
        CHECK(sf::base::fmtToString("[{:>4}]", "") == sf::base::String{"[    ]"});
    }

    SECTION("Adjacent placeholders")
    {
        CHECK(sf::base::fmtToString("{}{}{}", 1, 2, 3) == sf::base::String{"123"});
    }
}


////////////////////////////////////////////////////////////
// Custom types: ADL `fmtArg` overloads exercising every supported pattern.
//
// Wrapped in an anonymous namespace for internal linkage; the named
// inner `customtypes` namespace is what ADL keys on for these types.
////////////////////////////////////////////////////////////
namespace
{
namespace customtypes
{
////////////////////////////////////////////////////////////
// Pattern A: zero-scratch, just a byte copy.
////////////////////////////////////////////////////////////
struct FixedTag
{
    const char* text;
};

[[nodiscard]] inline sf::base::FmtResult fmtArg(sf::base::FmtSink& sink, const FixedTag& t, const sf::base::FmtSpec&) noexcept
{
    return sink.append(t.text, SFML_BASE_STRLEN(t.text));
}


////////////////////////////////////////////////////////////
// Pattern B: small simple composite -- recurse via `sink.format`.
////////////////////////////////////////////////////////////
struct Vec2f
{
    float x;
    float y;
};

[[nodiscard]] inline sf::base::FmtResult fmtArg(sf::base::FmtSink& sink, const Vec2f& v, const sf::base::FmtSpec&)
{
    return sink.fmt("({}, {})", v.x, v.y);
}


////////////////////////////////////////////////////////////
// Pattern C: composite container -- recursion over elements.
//
// Note: this composes over any T, so `MiniVec<Vec2f>` and even
// `MiniVec<MiniVec<Vec2f>>` Just Work.
////////////////////////////////////////////////////////////
template <typename T>
struct MiniVec
{
    const T*        data;
    sf::base::SizeT size;
};

template <typename T>
[[nodiscard]] inline sf::base::FmtResult fmtArg(sf::base::FmtSink& sink, const MiniVec<T>& v, const sf::base::FmtSpec&)
{
    SFML_BASE_FMT_TRY(sink.appendChar('['));
    for (sf::base::SizeT i = 0u; i < v.size; ++i)
    {
        if (i != 0u)
            SFML_BASE_FMT_TRY(sink.append(", ", 2u));
        SFML_BASE_FMT_TRY(sink.fmt("{}", v.data[i]));
    }
    return sink.appendChar(']');
}


////////////////////////////////////////////////////////////
// Pattern D: defaults to right-align (specialized variable template).
////////////////////////////////////////////////////////////
struct RightHex
{
    unsigned int value;
};

[[nodiscard]] inline sf::base::FmtResult fmtArg(sf::base::FmtSink& sink, const RightHex& h, const sf::base::FmtSpec&)
{
    char            buf[12]{};
    sf::base::SizeT n = 0u;
    unsigned int    v = h.value;
    char            tmp[10];

    if (v == 0u)
    {
        tmp[n++] = '0';
    }
    else
    {
        while (v != 0u && n < sizeof(tmp))
        {
            const unsigned int digit = v & 0xFu;
            tmp[n++]                 = static_cast<char>(digit < 10u ? '0' + digit : 'a' + digit - 10u);
            v >>= 4;
        }
    }

    // Reverse into `buf`.
    for (sf::base::SizeT i = 0u; i < n; ++i)
        buf[i] = tmp[n - 1u - i];

    return sink.append(buf, n);
}


////////////////////////////////////////////////////////////
// Pattern E: formatter-level failure, distinct from destination overflow.
////////////////////////////////////////////////////////////
struct FailTag
{
};

[[nodiscard]] inline sf::base::FmtResult fmtArg(sf::base::FmtSink&, const FailTag&, const sf::base::FmtSpec&) noexcept
{
    return sf::base::FmtResult::Failed;
}


////////////////////////////////////////////////////////////
struct RetryTag
{
    int* attemptCount;
};

[[nodiscard]] inline sf::base::FmtResult fmtArg(sf::base::FmtSink& sink, const RetryTag& t, const sf::base::FmtSpec&) noexcept
{
    ++*t.attemptCount;

    for (int i = 0; i < 600; ++i)
        SFML_BASE_FMT_TRY(sink.appendChar('x'));

    return sf::base::FmtResult::Ok;
}
} // namespace customtypes
} // anonymous namespace

// Default to right-align for `RightHex` (it's a "numeric-looking" type).
// Specialization lives in `namespace sf::base` to match the primary template;
// the type itself has internal linkage via the anonymous namespace above.
namespace sf::base
{
template <>
inline constexpr char fmtArgDefaultAlign<customtypes::RightHex> = '>';
} // namespace sf::base


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArg (Pattern A: zero-scratch byte copy)")
{
    using customtypes::FixedTag;

    SECTION("Default formatting")
    {
        CHECK(sf::base::fmtToString("{}", FixedTag{"hello"}) == sf::base::String{"hello"});
        CHECK(sf::base::fmtToString("[{}]", FixedTag{"world"}) == sf::base::String{"[world]"});
    }

    SECTION("Padding (default left-align)")
    {
        CHECK(sf::base::fmtToString("[{:7}]", FixedTag{"hi"}) == sf::base::String{"[hi     ]"});
        CHECK(sf::base::fmtToString("[{:>7}]", FixedTag{"hi"}) == sf::base::String{"[     hi]"});
        CHECK(sf::base::fmtToString("[{:^7}]", FixedTag{"hi"}) == sf::base::String{"[  hi   ]"});
    }

    SECTION("Custom fill")
    {
        CHECK(sf::base::fmtToString("[{:*>6}]", FixedTag{"xy"}) == sf::base::String{"[****xy]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArg (Pattern B: recursive composition)")
{
    using customtypes::Vec2f;

    SECTION("Single Vec2f")
    {
        CHECK(sf::base::fmtToString("pos = {}", Vec2f{1.5f, 2.0f}) == sf::base::String{"pos = (1.500000, 2.000000)"});
    }

    SECTION("Multiple Vec2f in one format")
    {
        CHECK(sf::base::fmtToString("a={} b={}", Vec2f{1.f, 2.f}, Vec2f{3.f, 4.f}) ==
              sf::base::String{"a=(1.000000, "
                               "2.000000) "
                               "b=(3.000000, "
                               "4.000000)"});
    }

    SECTION("Outer width pads the whole composed result")
    {
        // Rendered inner: "(0.000000, 0.000000)" -- exactly 20 chars; width 24 right-pads by 4.
        CHECK(sf::base::fmtToString("[{:>24}]", Vec2f{0.0f, 0.0f}) == sf::base::String{"[    (0.000000, 0.000000)]"});
    }

    SECTION("Outer precision is NOT propagated to nested calls (each inner '{}' uses its own spec)")
    {
        // The outer spec applies to the outer placeholder's content (the whole rendered Vec2f),
        // not to the inner float placeholders. So inner floats use default precision 6.
        const auto result = sf::base::fmtToString("{:.2}", Vec2f{1.5f, 2.5f});
        CHECK(result == sf::base::String{"(1.500000, 2.500000)"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArg (Pattern C: nested containers)")
{
    using customtypes::MiniVec;
    using customtypes::Vec2f;

    SECTION("Container of integers")
    {
        const int          elems[] = {1, 2, 3, 4};
        const MiniVec<int> v{elems, 4u};
        CHECK(sf::base::fmtToString("{}", v) == sf::base::String{"[1, 2, 3, 4]"});
    }

    SECTION("Empty container")
    {
        const MiniVec<int> v{nullptr, 0u};
        CHECK(sf::base::fmtToString("{}", v) == sf::base::String{"[]"});
    }

    SECTION("Container of Vec2f -- two levels of recursion")
    {
        const Vec2f          points[] = {{1.f, 2.f}, {3.f, 4.f}};
        const MiniVec<Vec2f> v{points, 2u};
        CHECK(sf::base::fmtToString("{}", v) == sf::base::String{"[(1.000000, 2.000000), (3.000000, 4.000000)]"});
    }

    SECTION("Container of container -- three levels of recursion")
    {
        const int                   row0[] = {1, 2, 3};
        const int                   row1[] = {4, 5};
        const MiniVec<int>          rows[] = {{row0, 3u}, {row1, 2u}};
        const MiniVec<MiniVec<int>> matrix{rows, 2u};

        CHECK(sf::base::fmtToString("{}", matrix) == sf::base::String{"[[1, 2, 3], [4, 5]]"});
    }

    SECTION("Container of containers of Vec2f -- mixed recursion")
    {
        const customtypes::Vec2f                   a[]    = {{1.f, 1.f}};
        const customtypes::Vec2f                   b[]    = {{2.f, 2.f}, {3.f, 3.f}};
        const MiniVec<customtypes::Vec2f>          rows[] = {{a, 1u}, {b, 2u}};
        const MiniVec<MiniVec<customtypes::Vec2f>> grid{rows, 2u};

        CHECK(sf::base::fmtToString("{}", grid) == sf::base::String{"[[(1.000000, 1.000000)], [(2.000000, 2.000000), "
                                                                    "(3.000000, 3.000000)]]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArgDefaultAlign specialization")
{
    using customtypes::RightHex;

    SECTION("Default alignment is now '>' (specialized)")
    {
        CHECK(sf::base::fmtToString("[{:6}]", RightHex{0xabu}) == sf::base::String{"[    ab]"});
    }

    SECTION("Explicit '<' overrides the specialized default")
    {
        CHECK(sf::base::fmtToString("[{:<6}]", RightHex{0xabu}) == sf::base::String{"[ab    ]"});
    }

    SECTION("Zero value")
    {
        CHECK(sf::base::fmtToString("[{:>5}]", RightHex{0u}) == sf::base::String{"[    0]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - FmtSink direct use (low-level API)")
{
    SECTION("Plain bytes + format mix")
    {
        char              buf[64];
        sf::base::FmtSink sink{buf, sizeof(buf)};

        CHECK(sink.append("[", 1u) == sf::base::FmtResult::Ok);
        CHECK(sink.fmt("{}", 42) == sf::base::FmtResult::Ok);
        CHECK(sink.append("] ", 2u) == sf::base::FmtResult::Ok);
        CHECK(sink.appendChar('=') == sf::base::FmtResult::Ok);
        CHECK(sink.fmt(" {:.1f}", 3.14) == sf::base::FmtResult::Ok);

        CHECK(sf::base::StringView{buf, sink.size()} == sf::base::StringView{"[42] = 3.1"});
    }

    SECTION("Overflow is returned eagerly and does not change the sink")
    {
        char              buf[5];
        sf::base::FmtSink sink{buf, sizeof(buf)};

        CHECK(sink.append("hello", 5u) == sf::base::FmtResult::Ok); // exactly fills buffer

        CHECK(sink.append("!", 1u) == sf::base::FmtResult::Overflow);
        CHECK(sink.size() == 5u);

        char              larger[16];
        sf::base::FmtSink fresh{larger, sizeof(larger)};
        CHECK(fresh.append(" world", 6u) == sf::base::FmtResult::Ok);
        CHECK(fresh.size() == 6u);
    }

    SECTION("size() as checkpoint + atOffset survives interleaved writes")
    {
        char              buf[64];
        sf::base::FmtSink sink{buf, sizeof(buf)};

        CHECK(sink.append("aa", 2u) == sf::base::FmtResult::Ok);
        const sf::base::SizeT m = sink.size();
        CHECK(sink.append("bb", 2u) == sf::base::FmtResult::Ok);

        CHECK(m == 2u);
        CHECK(sink.atOffset(m) == buf + 2);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - formatter failure is not treated as overflow")
{
    using customtypes::FailTag;

    SECTION("fmtIntoBuffer returns nullptr")
    {
        char buf[64];
        CHECK(sf::base::fmtIntoBuffer(buf, "{}", FailTag{}) == nullptr);
    }

    SECTION("fmtTo returns failed without appending partial output")
    {
        sf::base::String out = "prefix";
        (void)sf::base::fmtTo(out, "{}{}", "ok", FailTag{});
        CHECK(out == sf::base::String{"prefix"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - invalid built-in specs fail in release")
{
#ifndef SFML_DEBUG
    sf::base::String  out = "prefix";
    char              buffer[16];
    sf::base::FmtSink sink{buffer, sizeof(buffer)};

    CHECK(sf::base::fmtTo(out, "{:f}", 42) == sf::base::FmtResult::Failed);
    CHECK(sf::base::fmtArg(sink, 42, sf::base::FmtSpec{.type = 'q'}) == sf::base::FmtResult::Failed);
    CHECK(sf::base::fmtTo(out, "{:x}", 1.5) == sf::base::FmtResult::Failed);
    CHECK(sf::base::fmtTo(out, "{:.2}", "abc") == sf::base::FmtResult::Failed);
    CHECK(sf::base::fmtTo(out, "{:.2}", sf::base::StringView{"abc"}) == sf::base::FmtResult::Failed);
    CHECK(sf::base::fmtTo(out, "{:.2}", 'a') == sf::base::FmtResult::Failed);
    CHECK(sf::base::fmtTo(out, "{:x}", true) == sf::base::FmtResult::Failed);
    CHECK(out == sf::base::String{"prefix"});
#endif
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtTo() large output (heap fallback path)")
{
    SECTION("Output larger than 512-byte staging buffer succeeds")
    {
        // Build a format-args list that produces > 512 bytes of output.
        sf::base::String big;
        for (int i = 0; i < 100; ++i)
            big += sf::base::String{"0123456789"}; // 1000 chars

        sf::base::String out;
        (void)sf::base::fmtTo(out, "[{}]", sf::base::StringView{big.data(), big.size()});

        REQUIRE(out.size() == big.size() + 2u);
        CHECK(out.data()[0] == '[');
        CHECK(out.data()[out.size() - 1u] == ']');
        CHECK(sf::base::StringView{out.data() + 1, big.size()} == sf::base::StringView{big.data(), big.size()});
    }

    SECTION("Output near staging boundary")
    {
        // Exactly fits the 512-byte staging buffer.
        sf::base::String mid;
        for (int i = 0; i < 50; ++i)
            mid += sf::base::String{"0123456789"}; // 500 chars

        sf::base::String out;
        (void)sf::base::fmtTo(out, "{}", sf::base::StringView{mid.data(), mid.size()});

        CHECK(out.size() == mid.size());
    }

    SECTION("Custom formatter is retried after overflow")
    {
        int              attemptCount = 0;
        sf::base::String out;

        CHECK(sf::base::fmtTo(out, "{}", customtypes::RetryTag{&attemptCount}) == sf::base::FmtResult::Ok);
        CHECK(attemptCount == 2);
        CHECK(out.size() == 600u);
    }

    SECTION("Destination sink overflow is propagated")
    {
        char              buffer[1];
        sf::base::FmtSink sink{buffer, sizeof(buffer)};

        CHECK(sf::base::fmtTo(sink, "too large") == sf::base::FmtResult::Overflow);
        CHECK(sf::base::fmtTo(sink, "{:>600}", "") == sf::base::FmtResult::Overflow);
        CHECK(sink.size() == 0u);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - boundary conditions")
{
    SECTION("Exact fit -- buffer size equals output (no terminator reserved)")
    {
        char        buf[5];
        auto* const end = sf::base::fmtIntoBuffer(buf, "hello");
        REQUIRE(end != nullptr);
        CHECK(sf::base::StringView{buf, static_cast<sf::base::SizeT>(end - buf)} == sf::base::StringView{"hello"});
    }

    SECTION("Off-by-one -- one byte short")
    {
        char        buf[4];
        auto* const end = sf::base::fmtIntoBuffer(buf, "hello");
        CHECK(end == nullptr);
    }

    SECTION("Very wide width is honored")
    {
        sf::base::String out;
        (void)sf::base::fmtTo(out, "[{:>1000}]", "x");
        CHECK(out.size() == 1002u);
        CHECK(out.data()[0] == '[');
        CHECK(out.data()[out.size() - 2u] == 'x');
        CHECK(out.data()[out.size() - 1u] == ']');

        // First 1000 chars after '[' should be spaces, last is 'x'.
        for (sf::base::SizeT i = 1u; i < 1000u; ++i)
            CHECK(out.data()[i] == ' ');
    }

    SECTION("Padding to width 0 is a no-op")
    {
        CHECK(sf::base::fmtToString("[{:0}]", "abc") == sf::base::String{"[abc]"});
    }

    SECTION("Float at boundary of supported precision (10)")
    {
        CHECK(sf::base::fmtToString("{:.10f}", 0.5) == sf::base::String{"0.5000000000"});
    }

    SECTION("Large finite double inside the toChars range")
    {
        // The float backend (`toChars`) uses an `int64` scaled
        // representation: `|value| * 10^precision` must fit in
        // `int64` (~9.2e18). 9e12 * 10^6 (default precision) = 9e18,
        // safely below.
        sf::base::String out;

        CHECK(sf::base::fmtTo(out, "{}", 9'000'000'000'000.0) == sf::base::FmtResult::Ok);
        CHECK(out == sf::base::String{"9000000000000.000000"});
    }

    SECTION("Large double recovered by reducing precision")
    {
        // 10^13 * 10^3 = 10^16 < ~9.2e18, so a smaller precision
        // formats values that overflow at the default precision.
        sf::base::String out;

        CHECK(sf::base::fmtTo(out, "{:.3f}", 10'000'000'000'000.0) == sf::base::FmtResult::Ok);
        CHECK(out == sf::base::String{"10000000000000.000"});
    }

    SECTION("Double beyond toChars range reports Failed")
    {
        // 10^13 * 10^6 (default precision) = 10^19 > ~9.2e18, so the
        // backend reports `Failed`. The sink is left unchanged.
        sf::base::String out = "prefix:";

        CHECK(sf::base::fmtTo(out, "{}", 10'000'000'000'000.0) == sf::base::FmtResult::Failed);
        CHECK(out == sf::base::String{"prefix:"});
    }

    SECTION("Maximum finite double reports Failed (out of toChars range)")
    {
        // `std::numeric_limits<double>::max()` is ~1.8e308, far beyond
        // the int64-scaled representation even at precision 0.
        sf::base::String out = "prefix:";

        CHECK(sf::base::fmtTo(out, "{}", std::numeric_limits<double>::max()) == sf::base::FmtResult::Failed);
        CHECK(sf::base::fmtTo(out, "{:.0f}", std::numeric_limits<double>::max()) == sf::base::FmtResult::Failed);
        CHECK(out == sf::base::String{"prefix:"});
    }

    SECTION("Long double")
    {
        CHECK(sf::base::fmtToString("{}", 123.5L) == sf::base::String{"123.500000"});
    }

    SECTION("Recursive format from a custom type writes more than 512 bytes")
    {
        // 60 Vec2f values, each emits "(1.000000, 1.000000)" (20 chars) + ", " separator
        // → 60*20 + 59*2 = 1318 chars + brackets = 1320 bytes. Forces the heap path.
        customtypes::Vec2f values[60];
        for (auto& v : values)
        {
            v.x = 1.f;
            v.y = 1.f;
        }
        const customtypes::MiniVec<customtypes::Vec2f> v{values, 60u};

        sf::base::String out;
        (void)sf::base::fmtTo(out, "{}", v);

        CHECK(out.size() == 1320u);
        CHECK(out.data()[0] == '[');
        CHECK(out.data()[out.size() - 1u] == ']');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - composition + width interaction")
{
    SECTION("Outer width pads composed inner content uniformly")
    {
        using customtypes::Vec2f;

        // Rendered inner: "(1.000000, 2.000000)" -- 20 chars.
        CHECK(sf::base::fmtToString("[{:<25}]", Vec2f{1.f, 2.f}) == sf::base::String{"[(1.000000, 2.000000)     ]"});
    }

    SECTION("Multiple levels of recursion + outer padding")
    {
        const int                       elems[] = {7, 8};
        const customtypes::MiniVec<int> v{elems, 2u};

        // Rendered inner: "[7, 8]" (6 chars). Outer width 10, right-align:
        CHECK(sf::base::fmtToString("[{:>10}]", v) == sf::base::String{"[    [7, 8]]"});
    }
}
