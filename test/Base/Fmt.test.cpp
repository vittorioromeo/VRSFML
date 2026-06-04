#include "StringifySfBaseStringUtil.hpp" // IWYU pragma: keep
#include "StringifyStringViewUtil.hpp"   // IWYU pragma: keep
#include "Tst/Tst.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"

#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/Builtin/Strlen.hpp"
#include "ZancleBase/Fmt/FmtArgDefaultAlign.hpp"
#include "ZancleBase/Fmt/FmtResult.hpp"
#include "ZancleBase/Fmt/FmtSink.hpp"
#include "ZancleBase/Fmt/FmtSpec.hpp"
#include "ZancleBase/Fmt/FmtToString.hpp"
#include "ZancleBase/NonDeduced.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"

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
[[nodiscard]] zb::StringView formatToView(char (&buffer)[512],
                                                typename zb::NonDeduced<const zb::FmtString<Args...>>::type fmt,
                                                const Args&... args)
{
    char* const end = zb::fmtIntoBuffer(buffer, fmt, args...);
    if (end == nullptr)
        return {};
    return zb::StringView{buffer, static_cast<zb::SizeT>(end - buffer)};
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
        (void)zb::FmtString<Args...>{"{:.}"};
        return true;
    }()>;
};


////////////////////////////////////////////////////////////
template <typename... Args>
concept AcceptsEmptyPrecisionF = requires {
    typename FmtStringAccepted<[] consteval
    {
        (void)zb::FmtString<Args...>{"{:.f}"};
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
        CHECK(zb::fmtToString("hello") == zb::String{"hello"});
        CHECK(zb::fmtToString("") == zb::String{""});
    }

    SECTION("Single integer arg")
    {
        CHECK(zb::fmtToString("{}", 0) == zb::String{"0"});
        CHECK(zb::fmtToString("{}", 42) == zb::String{"42"});
        CHECK(zb::fmtToString("{}", -7) == zb::String{"-7"});
        CHECK(zb::fmtToString("[{}]", 100) == zb::String{"[100]"});
    }

    SECTION("Single floating-point arg (default precision = 6)")
    {
        CHECK(zb::fmtToString("{}", 3.5) == zb::String{"3.500000"});
        CHECK(zb::fmtToString("{}", -0.25) == zb::String{"-0.250000"});
    }

    SECTION("Every arithmetic type")
    {
        CHECK(zb::fmtToString("{}", false) == zb::String{"false"});
        CHECK(zb::fmtToString("{}", 'A') == zb::String{"A"});
        CHECK(zb::fmtToString("{}", static_cast<signed char>(-1)) == zb::String{"-1"});
        CHECK(zb::fmtToString("{}", static_cast<unsigned char>(1)) == zb::String{"1"});
        CHECK(zb::fmtToString("{}", static_cast<short>(-1)) == zb::String{"-1"});
        CHECK(zb::fmtToString("{}", static_cast<unsigned short>(1)) == zb::String{"1"});
        CHECK(zb::fmtToString("{}", -1) == zb::String{"-1"});
        CHECK(zb::fmtToString("{}", 1u) == zb::String{"1"});
        CHECK(zb::fmtToString("{}", -1l) == zb::String{"-1"});
        CHECK(zb::fmtToString("{}", 1ul) == zb::String{"1"});
        CHECK(zb::fmtToString("{}", -1ll) == zb::String{"-1"});
        CHECK(zb::fmtToString("{}", 1ull) == zb::String{"1"});
        CHECK(zb::fmtToString("{}", 1.f) == zb::String{"1.000000"});
        CHECK(zb::fmtToString("{}", 1.) == zb::String{"1.000000"});
        CHECK(zb::fmtToString("{}", 1.L) == zb::String{"1.000000"});
    }

    SECTION("Single string-like arg")
    {
        CHECK(zb::fmtToString("{}", "world") == zb::String{"world"});
        CHECK(zb::fmtToString("{}", zb::StringView{"hello"}) == zb::String{"hello"});
        CHECK(zb::fmtToString("{}", zb::String{"abc"}) == zb::String{"abc"});
    }

    SECTION("Multiple mixed args")
    {
        CHECK(zb::fmtToString("{} = {}", "x", 42) == zb::String{"x = 42"});
        CHECK(zb::fmtToString("{}, {}, {}", 1, 2, 3) == zb::String{"1, 2, 3"});
        CHECK(zb::fmtToString("{} + {} = {}", 2, 3, 5) == zb::String{"2 + 3 = 5"});
    }

    SECTION("Brace escapes")
    {
        CHECK(zb::fmtToString("{{}}") == zb::String{"{}"});
        CHECK(zb::fmtToString("{{{}}}", 7) == zb::String{"{7}"});
        CHECK(zb::fmtToString("a{{b}}c") == zb::String{"a{b}c"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - precision spec {:.Nf}")
{
    SECTION("Float precision")
    {
        CHECK(zb::fmtToString("{:.0f}", 3.7) == zb::String{"4"});
        CHECK(zb::fmtToString("{:.1f}", 3.14) == zb::String{"3.1"});
        CHECK(zb::fmtToString("{:.2f}", 3.14) == zb::String{"3.14"});
        CHECK(zb::fmtToString("{:.3f}", 3.14159) == zb::String{"3.142"});
        CHECK(zb::fmtToString("{:.6f}", 1.0) == zb::String{"1.000000"});
    }

    SECTION("Negative floats")
    {
        CHECK(zb::fmtToString("{:.1f}", -2.5) == zb::String{"-2.5"});
        CHECK(zb::fmtToString("{:.3f}", -0.001) == zb::String{"-0.001"});
    }

    SECTION("Float precision without `f` tag works the same")
    {
        CHECK(zb::fmtToString("{:.2}", 3.14159) == zb::String{"3.14"});
    }

    // Precision is only valid for floating-point arguments. Integer/string
    // formatters assert on precision in debug builds.
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - width spec {:N}")
{
    SECTION("Numbers default to right-align")
    {
        CHECK(zb::fmtToString("[{:5}]", 42) == zb::String{"[   42]"});
        CHECK(zb::fmtToString("[{:3}]", 1) == zb::String{"[  1]"});
    }

    SECTION("Strings default to left-align")
    {
        CHECK(zb::fmtToString("[{:5}]", "hi") == zb::String{"[hi   ]"});
        CHECK(zb::fmtToString("[{:6}]", zb::StringView{"abc"}) == zb::String{"[abc   ]"});
    }

    SECTION("Content equal to width -- no padding")
    {
        CHECK(zb::fmtToString("[{:3}]", 123) == zb::String{"[123]"});
        CHECK(zb::fmtToString("[{:3}]", "abc") == zb::String{"[abc]"});
    }

    SECTION("Content wider than width -- no truncation")
    {
        CHECK(zb::fmtToString("[{:3}]", 12'345) == zb::String{"[12345]"});
        CHECK(zb::fmtToString("[{:2}]", "long string") == zb::String{"[long string]"});
    }

    SECTION("Width of zero (degenerate but valid)")
    {
        CHECK(zb::fmtToString("[{:0}]", 42) == zb::String{"[42]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - explicit alignment {:<N}, {:>N}, {:^N}")
{
    SECTION("Right-align '>'")
    {
        CHECK(zb::fmtToString("[{:>5}]", "hi") == zb::String{"[   hi]"});
        CHECK(zb::fmtToString("[{:>5}]", 42) == zb::String{"[   42]"});
    }

    SECTION("Left-align '<'")
    {
        CHECK(zb::fmtToString("[{:<5}]", "hi") == zb::String{"[hi   ]"});
        CHECK(zb::fmtToString("[{:<5}]", 42) == zb::String{"[42   ]"});
    }

    SECTION("Center align '^'")
    {
        // Even-width slack splits evenly (extra goes right).
        CHECK(zb::fmtToString("[{:^6}]", "hi") == zb::String{"[  hi  ]"});
        // Odd-width slack: extra goes to right side.
        CHECK(zb::fmtToString("[{:^5}]", "hi") == zb::String{"[ hi  ]"});
        CHECK(zb::fmtToString("[{:^5}]", 1) == zb::String{"[  1  ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fill character")
{
    SECTION("Zero-pad on the left")
    {
        CHECK(zb::fmtToString("[{:0>5}]", 42) == zb::String{"[00042]"});
        CHECK(zb::fmtToString("[{:0>5}]", 7) == zb::String{"[00007]"});
    }

    SECTION("Dot-fill on the left")
    {
        CHECK(zb::fmtToString("[{:.>5}]", "ab") == zb::String{"[...ab]"});
    }

    SECTION("Dash-center-fill")
    {
        CHECK(zb::fmtToString("[{:-^7}]", "hi") == zb::String{"[--hi---]"});
    }

    SECTION("Custom fill on left-align")
    {
        CHECK(zb::fmtToString("[{:*<5}]", "ab") == zb::String{"[ab***]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - combined specs (width + precision)")
{
    SECTION("Width + precision, right-aligned")
    {
        CHECK(zb::fmtToString("[{:>10.3f}]", 3.14159) == zb::String{"[     3.142]"});
    }

    SECTION("Width + precision, zero-padded")
    {
        CHECK(zb::fmtToString("[{:0>10.2f}]", 3.14) == zb::String{"[0000003.14]"});
    }

    SECTION("Width + precision, left-aligned")
    {
        CHECK(zb::fmtToString("[{:<10.2f}]", 3.14) == zb::String{"[3.14      ]"});
    }

    SECTION("Width + precision, center-aligned")
    {
        CHECK(zb::fmtToString("[{:^10.2f}]", 3.14) == zb::String{"[   3.14   ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - integer radix specs {:x}, {:X}, {:o}, {:b}, {:d}")
{
    SECTION("Lowercase hex")
    {
        CHECK(zb::fmtToString("{:x}", 0xAB'CDu) == zb::String{"abcd"});
        CHECK(zb::fmtToString("{:x}", 0u) == zb::String{"0"});
        CHECK(zb::fmtToString("{:x}", 0xDE'AD'BE'EFu) == zb::String{"deadbeef"});
    }

    SECTION("Uppercase hex")
    {
        CHECK(zb::fmtToString("{:X}", 0xAB'CDu) == zb::String{"ABCD"});
        CHECK(zb::fmtToString("{:X}", 0xDE'AD'BE'EFu) == zb::String{"DEADBEEF"});
    }

    SECTION("Octal")
    {
        CHECK(zb::fmtToString("{:o}", 8u) == zb::String{"10"});
        CHECK(zb::fmtToString("{:o}", 0u) == zb::String{"0"});
        CHECK(zb::fmtToString("{:o}", 511u) == zb::String{"777"});
    }

    SECTION("Binary")
    {
        CHECK(zb::fmtToString("{:b}", 0u) == zb::String{"0"});
        CHECK(zb::fmtToString("{:b}", 5u) == zb::String{"101"});
        CHECK(zb::fmtToString("{:b}", 0xFFu) == zb::String{"11111111"});
    }

    SECTION("Explicit decimal tag matches default")
    {
        CHECK(zb::fmtToString("{:d}", 42) == zb::String{"42"});
        CHECK(zb::fmtToString("{:d}", -42) == zb::String{"-42"});
    }

    SECTION("Negative signed: hex/oct/bin emit the bit pattern, no sign")
    {
        // 32-bit `int`: two's-complement of -1 is 0xFFFFFFFF.
        CHECK(zb::fmtToString("{:x}", -1) == zb::String{"ffffffff"});
        CHECK(zb::fmtToString("{:X}", -1) == zb::String{"FFFFFFFF"});
        CHECK(zb::fmtToString("{:b}", static_cast<signed char>(-1)) == zb::String{"11111111"});
    }

    SECTION("Width + fill: zero-padded hex scancode (Keyboard.cpp use case)")
    {
        CHECK(zb::fmtToString("{:0>4x}", 0x1Bu) == zb::String{"001b"});
        CHECK(zb::fmtToString("{:0>4X}", 0x1Bu) == zb::String{"001B"});
        CHECK(zb::fmtToString("{:0>8b}", 0xAu) == zb::String{"00001010"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - mixed-spec multi-arg")
{
    const auto result = zb::fmtToString("[{:>5}|{:<5}|{:^5}]", 1, 2, 3);
    CHECK(result == zb::String{"[    1|2    |  3  ]"});

    const auto hud = zb::fmtToString("FPS: {:>5.1f} | Frame: {:>4}ms | Cells: {:0>4}", 59.95, 16, 42);
    CHECK(hud == zb::String{"FPS:  60.0 | Frame:   16ms | Cells: 0042"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - integer type variety")
{
    SECTION("signed/unsigned")
    {
        CHECK(zb::fmtToString("{}", -32'000) == zb::String{"-32000"});
        CHECK(zb::fmtToString("{}", static_cast<unsigned>(4'000'000'000u)) == zb::String{"4000000000"});

        // Narrow signed types must survive `toChars` integer-promotion path
        CHECK(zb::fmtToString("{}", static_cast<short>(-32'000)) == zb::String{"-32000"});
        CHECK(zb::fmtToString("{}", static_cast<signed char>(-100)) == zb::String{"-100"});
    }

    SECTION("long long")
    {
        CHECK(zb::fmtToString("{}", static_cast<long long>(-1'234'567'890'123LL)) ==
              zb::String{"-123456789012"
                               "3"});
        CHECK(zb::fmtToString("{}", static_cast<unsigned long long>(18'000'000'000'000ULL)) ==
              zb::String{"180000"
                               "000000"
                               "00"});
    }

    SECTION("char formats as a glyph by default (matches libfmt)")
    {
        CHECK(zb::fmtToString("{}", 'A') == zb::String{"A"});
        CHECK(zb::fmtToString("{}", '\n') == zb::String{"\n"});
        CHECK(zb::fmtToString("[{}]", '"') == zb::String{"[\"]"});
        CHECK(zb::fmtToString("{}{}{}", 'a', 'b', 'c') == zb::String{"abc"});
    }

    SECTION("`:c` on char also emits the glyph")
    {
        CHECK(zb::fmtToString("{:c}", 'A') == zb::String{"A"});
        CHECK(zb::fmtToString("{:c}", '\n') == zb::String{"\n"});
    }

    SECTION("`:d` on char emits the numeric code")
    {
        CHECK(zb::fmtToString("{:d}", 'A') == zb::String{"65"});
        CHECK(zb::fmtToString("{:d}", '0') == zb::String{"48"});
    }

    SECTION("`:x` / `:X` / `:o` / `:b` on char emit the numeric code in that radix")
    {
        CHECK(zb::fmtToString("{:x}", 'A') == zb::String{"41"});
        CHECK(zb::fmtToString("{:X}", 'A') == zb::String{"41"});
        CHECK(zb::fmtToString("{:o}", 'A') == zb::String{"101"});
        CHECK(zb::fmtToString("{:b}", 'A') == zb::String{"1000001"});
    }

    SECTION("`:c` on an integer emits the lowest byte as a glyph")
    {
        CHECK(zb::fmtToString("{:c}", 65) == zb::String{"A"});
        CHECK(zb::fmtToString("{:c}", 0x41u) == zb::String{"A"});
    }

    SECTION("char default alignment is left, like a string")
    {
        CHECK(zb::fmtToString("[{:3}]", 'A') == zb::String{"[A  ]"});
        CHECK(zb::fmtToString("[{:>3}]", 'A') == zb::String{"[  A]"});
        CHECK(zb::fmtToString("[{:^3}]", 'A') == zb::String{"[ A ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - bool formats as 'true'/'false'")
{
    SECTION("Default presentation matches std::format / fmt::format")
    {
        CHECK(zb::fmtToString("{}", true) == zb::String{"true"});
        CHECK(zb::fmtToString("{}", false) == zb::String{"false"});
    }

    SECTION("Multiple bools in one format string")
    {
        CHECK(zb::fmtToString("{}/{}", true, false) == zb::String{"true/false"});
        CHECK(zb::fmtToString("a={} b={}", false, true) == zb::String{"a=false b=true"});
    }

    SECTION("Default alignment is left (matches string-like types)")
    {
        CHECK(zb::fmtToString("[{:8}]", true) == zb::String{"[true    ]"});
        CHECK(zb::fmtToString("[{:8}]", false) == zb::String{"[false   ]"});
    }

    SECTION("Explicit alignment overrides the default")
    {
        CHECK(zb::fmtToString("[{:>8}]", true) == zb::String{"[    true]"});
        CHECK(zb::fmtToString("[{:>8}]", false) == zb::String{"[   false]"});
        CHECK(zb::fmtToString("[{:^8}]", true) == zb::String{"[  true  ]"});
    }

    SECTION("Custom fill character")
    {
        CHECK(zb::fmtToString("[{:.>8}]", true) == zb::String{"[....true]"});
        CHECK(zb::fmtToString("[{:*<8}]", false) == zb::String{"[false***]"});
    }

    SECTION("Width equal to or smaller than content -- no padding")
    {
        CHECK(zb::fmtToString("[{:4}]", true) == zb::String{"[true]"});
        CHECK(zb::fmtToString("[{:3}]", false) == zb::String{"[false]"}); // wider than width
    }

    SECTION("Numeric form available via explicit int cast")
    {
        CHECK(zb::fmtToString("{}/{}", static_cast<int>(true), static_cast<int>(false)) ==
              zb::String{"1/"
                               "0"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtIntoBuffer raw API")
{
    char buf[512];

    SECTION("Returns end pointer on success (no implicit null terminator)")
    {
        const auto* end = zb::fmtIntoBuffer(buf, "x={}", 42);
        REQUIRE(end != nullptr);
        CHECK(zb::StringView{buf, static_cast<zb::SizeT>(end - buf)} == zb::StringView{"x=42"});
    }

    SECTION("Returns nullptr on buffer overflow")
    {
        char small[4];
        // "hello" is 5 bytes, doesn't fit in 4.
        CHECK(zb::fmtIntoBuffer(small, "hello") == nullptr);
    }

    SECTION("Caller may write their own null terminator")
    {
        // Reserve an extra byte for '\0' and write it manually.
        char        cstr[16];
        auto* const end = zb::fmtIntoBuffer(cstr, sizeof(cstr) - 1u, "n={}", 7);
        REQUIRE(end != nullptr);
        *end = '\0';
        CHECK(zb::StringView{cstr} == zb::StringView{"n=7"});
    }

    SECTION("Returns nullptr when arg formatting overflows")
    {
        char small[4];
        CHECK(zb::fmtIntoBuffer(small, "{}", 1'234'567) == nullptr);
    }

    SECTION("Helper produces same result as format()")
    {
        const auto view = formatToView(buf, "Hello, {}!", "world");
        CHECK(view == zb::StringView{"Hello, world!"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtTo() into base::String")
{
    SECTION("Appends to non-empty sink")
    {
        zb::String s = "prefix:";
        (void)zb::fmtTo(s, " {} = {:.2f}", "pi", 3.14159);
        CHECK(s == zb::String{"prefix: pi = 3.14"});
    }

    SECTION("Format into fresh sink")
    {
        zb::String s;
        (void)zb::fmtTo(s, "{:>8}", "abc");
        CHECK(s == zb::String{"     abc"});
    }

    SECTION("Repeated fmtTo accumulates")
    {
        zb::String s;
        (void)zb::fmtTo(s, "{}-", 1);
        (void)zb::fmtTo(s, "{}-", 2);
        (void)zb::fmtTo(s, "{}", 3);
        CHECK(s == zb::String{"1-2-3"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtTo() into Utf8String")
{
    SECTION("Build a HUD-style string")
    {
        za::Utf8String text;
        CHECK(zb::fmtTo(text, "Frame: {:>5}ms / Delta: {:.2f}", 16, 0.016667) == zb::FmtResult::Ok);

        const auto bytes = text.asBytes();
        CHECK(zb::StringView{bytes.data(), bytes.size()} == zb::StringView{"Frame:    16ms / Delta: 0.02"});
    }

    SECTION("Appends to a Utf8String already containing UTF-8 content")
    {
        za::Utf8String text{u8"café "};
        CHECK(zb::fmtTo(text, "= {}", 42) == zb::FmtResult::Ok);

        const auto bytes = text.asBytes();
        CHECK(zb::StringView{bytes.data(), bytes.size()} == zb::StringView{"café = 42"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - edge cases")
{
    SECTION("Empty format string")
    {
        CHECK(zb::fmtToString("") == zb::String{""});
    }

    SECTION("Only escapes")
    {
        CHECK(zb::fmtToString("{{") == zb::String{"{"});
        CHECK(zb::fmtToString("}}") == zb::String{"}"});
        CHECK(zb::fmtToString("{{{{}}}}") == zb::String{"{{}}"});
    }

    SECTION("Empty string arg with width")
    {
        CHECK(zb::fmtToString("[{:4}]", "") == zb::String{"[    ]"});
        CHECK(zb::fmtToString("[{:>4}]", "") == zb::String{"[    ]"});
    }

    SECTION("Adjacent placeholders")
    {
        CHECK(zb::fmtToString("{}{}{}", 1, 2, 3) == zb::String{"123"});
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

[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink& sink, const FixedTag& t, const zb::FmtSpec&) noexcept
{
    return sink.append(t.text, ZB_STRLEN(t.text));
}


////////////////////////////////////////////////////////////
// Pattern B: small simple composite -- recurse via `sink.format`.
////////////////////////////////////////////////////////////
struct Vec2f
{
    float x;
    float y;
};

[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink& sink, const Vec2f& v, const zb::FmtSpec&)
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
    zb::SizeT size;
};

template <typename T>
[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink& sink, const MiniVec<T>& v, const zb::FmtSpec&)
{
    ZB_FMT_TRY(sink.appendChar('['));
    for (zb::SizeT i = 0u; i < v.size; ++i)
    {
        if (i != 0u)
            ZB_FMT_TRY(sink.append(", ", 2u));
        ZB_FMT_TRY(sink.fmt("{}", v.data[i]));
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

[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink& sink, const RightHex& h, const zb::FmtSpec&)
{
    char            buf[12]{};
    zb::SizeT n = 0u;
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
    for (zb::SizeT i = 0u; i < n; ++i)
        buf[i] = tmp[n - 1u - i];

    return sink.append(buf, n);
}


////////////////////////////////////////////////////////////
// Pattern E: formatter-level failure, distinct from destination overflow.
////////////////////////////////////////////////////////////
struct FailTag
{
};

[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink&, const FailTag&, const zb::FmtSpec&) noexcept
{
    return zb::FmtResult::Failed;
}


////////////////////////////////////////////////////////////
struct RetryTag
{
    int* attemptCount;
};

[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink& sink, const RetryTag& t, const zb::FmtSpec&) noexcept
{
    ++*t.attemptCount;

    for (int i = 0; i < 600; ++i)
        ZB_FMT_TRY(sink.appendChar('x'));

    return zb::FmtResult::Ok;
}
} // namespace customtypes
} // anonymous namespace

// Default to right-align for `RightHex` (it's a "numeric-looking" type).
// Specialization lives in `namespace zb` to match the primary template;
// the type itself has internal linkage via the anonymous namespace above.
namespace zb
{
template <>
inline constexpr char fmtArgDefaultAlign<customtypes::RightHex> = '>';
} // namespace zb


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArg (Pattern A: zero-scratch byte copy)")
{
    using customtypes::FixedTag;

    SECTION("Default formatting")
    {
        CHECK(zb::fmtToString("{}", FixedTag{"hello"}) == zb::String{"hello"});
        CHECK(zb::fmtToString("[{}]", FixedTag{"world"}) == zb::String{"[world]"});
    }

    SECTION("Padding (default left-align)")
    {
        CHECK(zb::fmtToString("[{:7}]", FixedTag{"hi"}) == zb::String{"[hi     ]"});
        CHECK(zb::fmtToString("[{:>7}]", FixedTag{"hi"}) == zb::String{"[     hi]"});
        CHECK(zb::fmtToString("[{:^7}]", FixedTag{"hi"}) == zb::String{"[  hi   ]"});
    }

    SECTION("Custom fill")
    {
        CHECK(zb::fmtToString("[{:*>6}]", FixedTag{"xy"}) == zb::String{"[****xy]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArg (Pattern B: recursive composition)")
{
    using customtypes::Vec2f;

    SECTION("Single Vec2f")
    {
        CHECK(zb::fmtToString("pos = {}", Vec2f{1.5f, 2.0f}) == zb::String{"pos = (1.500000, 2.000000)"});
    }

    SECTION("Multiple Vec2f in one format")
    {
        CHECK(zb::fmtToString("a={} b={}", Vec2f{1.f, 2.f}, Vec2f{3.f, 4.f}) ==
              zb::String{"a=(1.000000, "
                               "2.000000) "
                               "b=(3.000000, "
                               "4.000000)"});
    }

    SECTION("Outer width pads the whole composed result")
    {
        // Rendered inner: "(0.000000, 0.000000)" -- exactly 20 chars; width 24 right-pads by 4.
        CHECK(zb::fmtToString("[{:>24}]", Vec2f{0.0f, 0.0f}) == zb::String{"[    (0.000000, 0.000000)]"});
    }

    SECTION("Outer precision is NOT propagated to nested calls (each inner '{}' uses its own spec)")
    {
        // The outer spec applies to the outer placeholder's content (the whole rendered Vec2f),
        // not to the inner float placeholders. So inner floats use default precision 6.
        const auto result = zb::fmtToString("{:.2}", Vec2f{1.5f, 2.5f});
        CHECK(result == zb::String{"(1.500000, 2.500000)"});
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
        CHECK(zb::fmtToString("{}", v) == zb::String{"[1, 2, 3, 4]"});
    }

    SECTION("Empty container")
    {
        const MiniVec<int> v{nullptr, 0u};
        CHECK(zb::fmtToString("{}", v) == zb::String{"[]"});
    }

    SECTION("Container of Vec2f -- two levels of recursion")
    {
        const Vec2f          points[] = {{1.f, 2.f}, {3.f, 4.f}};
        const MiniVec<Vec2f> v{points, 2u};
        CHECK(zb::fmtToString("{}", v) == zb::String{"[(1.000000, 2.000000), (3.000000, 4.000000)]"});
    }

    SECTION("Container of container -- three levels of recursion")
    {
        const int                   row0[] = {1, 2, 3};
        const int                   row1[] = {4, 5};
        const MiniVec<int>          rows[] = {{row0, 3u}, {row1, 2u}};
        const MiniVec<MiniVec<int>> matrix{rows, 2u};

        CHECK(zb::fmtToString("{}", matrix) == zb::String{"[[1, 2, 3], [4, 5]]"});
    }

    SECTION("Container of containers of Vec2f -- mixed recursion")
    {
        const customtypes::Vec2f                   a[]    = {{1.f, 1.f}};
        const customtypes::Vec2f                   b[]    = {{2.f, 2.f}, {3.f, 3.f}};
        const MiniVec<customtypes::Vec2f>          rows[] = {{a, 1u}, {b, 2u}};
        const MiniVec<MiniVec<customtypes::Vec2f>> grid{rows, 2u};

        CHECK(zb::fmtToString("{}", grid) == zb::String{"[[(1.000000, 1.000000)], [(2.000000, 2.000000), "
                                                                    "(3.000000, 3.000000)]]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - custom fmtArgDefaultAlign specialization")
{
    using customtypes::RightHex;

    SECTION("Default alignment is now '>' (specialized)")
    {
        CHECK(zb::fmtToString("[{:6}]", RightHex{0xabu}) == zb::String{"[    ab]"});
    }

    SECTION("Explicit '<' overrides the specialized default")
    {
        CHECK(zb::fmtToString("[{:<6}]", RightHex{0xabu}) == zb::String{"[ab    ]"});
    }

    SECTION("Zero value")
    {
        CHECK(zb::fmtToString("[{:>5}]", RightHex{0u}) == zb::String{"[    0]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - FmtSink direct use (low-level API)")
{
    SECTION("Plain bytes + format mix")
    {
        char              buf[64];
        zb::FmtSink sink{buf, sizeof(buf)};

        CHECK(sink.append("[", 1u) == zb::FmtResult::Ok);
        CHECK(sink.fmt("{}", 42) == zb::FmtResult::Ok);
        CHECK(sink.append("] ", 2u) == zb::FmtResult::Ok);
        CHECK(sink.appendChar('=') == zb::FmtResult::Ok);
        CHECK(sink.fmt(" {:.1f}", 3.14) == zb::FmtResult::Ok);

        CHECK(zb::StringView{buf, sink.size()} == zb::StringView{"[42] = 3.1"});
    }

    SECTION("Overflow is returned eagerly and does not change the sink")
    {
        char              buf[5];
        zb::FmtSink sink{buf, sizeof(buf)};

        CHECK(sink.append("hello", 5u) == zb::FmtResult::Ok); // exactly fills buffer

        CHECK(sink.append("!", 1u) == zb::FmtResult::Overflow);
        CHECK(sink.size() == 5u);

        char              larger[16];
        zb::FmtSink fresh{larger, sizeof(larger)};
        CHECK(fresh.append(" world", 6u) == zb::FmtResult::Ok);
        CHECK(fresh.size() == 6u);
    }

    SECTION("size() as checkpoint + atOffset survives interleaved writes")
    {
        char              buf[64];
        zb::FmtSink sink{buf, sizeof(buf)};

        CHECK(sink.append("aa", 2u) == zb::FmtResult::Ok);
        const zb::SizeT m = sink.size();
        CHECK(sink.append("bb", 2u) == zb::FmtResult::Ok);

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
        CHECK(zb::fmtIntoBuffer(buf, "{}", FailTag{}) == nullptr);
    }

    SECTION("fmtTo returns failed without appending partial output")
    {
        zb::String out = "prefix";
        (void)zb::fmtTo(out, "{}{}", "ok", FailTag{});
        CHECK(out == zb::String{"prefix"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - invalid built-in specs fail in release")
{
#ifndef ZA_DEBUG
    zb::String  out = "prefix";
    char              buffer[16];
    zb::FmtSink sink{buffer, sizeof(buffer)};

    CHECK(zb::fmtTo(out, "{:f}", 42) == zb::FmtResult::Failed);
    CHECK(zb::fmtArg(sink, 42, zb::FmtSpec{.type = 'q'}) == zb::FmtResult::Failed);
    CHECK(zb::fmtTo(out, "{:x}", 1.5) == zb::FmtResult::Failed);
    CHECK(zb::fmtTo(out, "{:.2}", "abc") == zb::FmtResult::Failed);
    CHECK(zb::fmtTo(out, "{:.2}", zb::StringView{"abc"}) == zb::FmtResult::Failed);
    CHECK(zb::fmtTo(out, "{:.2}", 'a') == zb::FmtResult::Failed);
    CHECK(zb::fmtTo(out, "{:x}", true) == zb::FmtResult::Failed);
    CHECK(out == zb::String{"prefix"});
#endif
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - fmtTo() large output (heap fallback path)")
{
    SECTION("Output larger than 512-byte staging buffer succeeds")
    {
        // Build a format-args list that produces > 512 bytes of output.
        zb::String big;
        for (int i = 0; i < 100; ++i)
            big += zb::String{"0123456789"}; // 1000 chars

        zb::String out;
        (void)zb::fmtTo(out, "[{}]", zb::StringView{big.data(), big.size()});

        REQUIRE(out.size() == big.size() + 2u);
        CHECK(out.data()[0] == '[');
        CHECK(out.data()[out.size() - 1u] == ']');
        CHECK(zb::StringView{out.data() + 1, big.size()} == zb::StringView{big.data(), big.size()});
    }

    SECTION("Output near staging boundary")
    {
        // Exactly fits the 512-byte staging buffer.
        zb::String mid;
        for (int i = 0; i < 50; ++i)
            mid += zb::String{"0123456789"}; // 500 chars

        zb::String out;
        (void)zb::fmtTo(out, "{}", zb::StringView{mid.data(), mid.size()});

        CHECK(out.size() == mid.size());
    }

    SECTION("Custom formatter is retried after overflow")
    {
        int              attemptCount = 0;
        zb::String out;

        CHECK(zb::fmtTo(out, "{}", customtypes::RetryTag{&attemptCount}) == zb::FmtResult::Ok);
        CHECK(attemptCount == 2);
        CHECK(out.size() == 600u);
    }

    SECTION("Destination sink overflow is propagated")
    {
        char              buffer[1];
        zb::FmtSink sink{buffer, sizeof(buffer)};

        CHECK(zb::fmtTo(sink, "too large") == zb::FmtResult::Overflow);
        CHECK(zb::fmtTo(sink, "{:>600}", "") == zb::FmtResult::Overflow);
        CHECK(sink.size() == 0u);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Fmt.hpp - boundary conditions")
{
    SECTION("Exact fit -- buffer size equals output (no terminator reserved)")
    {
        char        buf[5];
        auto* const end = zb::fmtIntoBuffer(buf, "hello");
        REQUIRE(end != nullptr);
        CHECK(zb::StringView{buf, static_cast<zb::SizeT>(end - buf)} == zb::StringView{"hello"});
    }

    SECTION("Off-by-one -- one byte short")
    {
        char        buf[4];
        auto* const end = zb::fmtIntoBuffer(buf, "hello");
        CHECK(end == nullptr);
    }

    SECTION("Very wide width is honored")
    {
        zb::String out;
        (void)zb::fmtTo(out, "[{:>1000}]", "x");
        CHECK(out.size() == 1002u);
        CHECK(out.data()[0] == '[');
        CHECK(out.data()[out.size() - 2u] == 'x');
        CHECK(out.data()[out.size() - 1u] == ']');

        // First 1000 chars after '[' should be spaces, last is 'x'.
        for (zb::SizeT i = 1u; i < 1000u; ++i)
            CHECK(out.data()[i] == ' ');
    }

    SECTION("Padding to width 0 is a no-op")
    {
        CHECK(zb::fmtToString("[{:0}]", "abc") == zb::String{"[abc]"});
    }

    SECTION("Float at boundary of supported precision (10)")
    {
        CHECK(zb::fmtToString("{:.10f}", 0.5) == zb::String{"0.5000000000"});
    }

    SECTION("Large finite double inside the toChars range")
    {
        // The float backend (`toChars`) uses an `int64` scaled
        // representation: `|value| * 10^precision` must fit in
        // `int64` (~9.2e18). 9e12 * 10^6 (default precision) = 9e18,
        // safely below.
        zb::String out;

        CHECK(zb::fmtTo(out, "{}", 9'000'000'000'000.0) == zb::FmtResult::Ok);
        CHECK(out == zb::String{"9000000000000.000000"});
    }

    SECTION("Large double recovered by reducing precision")
    {
        // 10^13 * 10^3 = 10^16 < ~9.2e18, so a smaller precision
        // formats values that overflow at the default precision.
        zb::String out;

        CHECK(zb::fmtTo(out, "{:.3f}", 10'000'000'000'000.0) == zb::FmtResult::Ok);
        CHECK(out == zb::String{"10000000000000.000"});
    }

    SECTION("Double beyond toChars range reports Failed")
    {
        // 10^13 * 10^6 (default precision) = 10^19 > ~9.2e18, so the
        // backend reports `Failed`. The sink is left unchanged.
        zb::String out = "prefix:";

        CHECK(zb::fmtTo(out, "{}", 10'000'000'000'000.0) == zb::FmtResult::Failed);
        CHECK(out == zb::String{"prefix:"});
    }

    SECTION("Maximum finite double reports Failed (out of toChars range)")
    {
        // `std::numeric_limits<double>::max()` is ~1.8e308, far beyond
        // the int64-scaled representation even at precision 0.
        zb::String out = "prefix:";

        CHECK(zb::fmtTo(out, "{}", std::numeric_limits<double>::max()) == zb::FmtResult::Failed);
        CHECK(zb::fmtTo(out, "{:.0f}", std::numeric_limits<double>::max()) == zb::FmtResult::Failed);
        CHECK(out == zb::String{"prefix:"});
    }

    SECTION("Long double")
    {
        CHECK(zb::fmtToString("{}", 123.5L) == zb::String{"123.500000"});
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

        zb::String out;
        (void)zb::fmtTo(out, "{}", v);

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
        CHECK(zb::fmtToString("[{:<25}]", Vec2f{1.f, 2.f}) == zb::String{"[(1.000000, 2.000000)     ]"});
    }

    SECTION("Multiple levels of recursion + outer padding")
    {
        const int                       elems[] = {7, 8};
        const customtypes::MiniVec<int> v{elems, 2u};

        // Rendered inner: "[7, 8]" (6 chars). Outer width 10, right-align:
        CHECK(zb::fmtToString("[{:>10}]", v) == zb::String{"[    [7, 8]]"});
    }
}
