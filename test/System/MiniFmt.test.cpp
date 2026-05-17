#include "StringifySfBaseStringUtil.hpp" // IWYU pragma: keep
#include "StringifyStringViewUtil.hpp"   // IWYU pragma: keep

#include "SFML/System/Utf8String.hpp"

#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/MiniFmtFormat.hpp"
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
[[nodiscard]] sf::base::StringView formatToView(
    char (&buffer)[512],
    typename sf::base::NonDeduced<const sf::base::FormatString<Args...>>::type fmt,
    const Args&... args)
{
    char* const end = sf::base::formatIntoBuffer(buffer, fmt, args...);
    if (end == nullptr)
        return {};
    return sf::base::StringView{buffer, static_cast<sf::base::SizeT>(end - buffer)};
}

} // namespace MiniFmtTest
} // namespace


using namespace MiniFmtTest;


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - format() basics")
{
    SECTION("No args, plain text")
    {
        CHECK(sf::base::format("hello") == sf::base::String{"hello"});
        CHECK(sf::base::format("") == sf::base::String{""});
    }

    SECTION("Single integer arg")
    {
        CHECK(sf::base::format("{}", 0) == sf::base::String{"0"});
        CHECK(sf::base::format("{}", 42) == sf::base::String{"42"});
        CHECK(sf::base::format("{}", -7) == sf::base::String{"-7"});
        CHECK(sf::base::format("[{}]", 100) == sf::base::String{"[100]"});
    }

    SECTION("Single floating-point arg (default precision = 6)")
    {
        CHECK(sf::base::format("{}", 3.5) == sf::base::String{"3.500000"});
        CHECK(sf::base::format("{}", -0.25) == sf::base::String{"-0.250000"});
    }

    SECTION("Single string-like arg")
    {
        CHECK(sf::base::format("{}", "world") == sf::base::String{"world"});
        CHECK(sf::base::format("{}", sf::base::StringView{"hello"}) == sf::base::String{"hello"});
        CHECK(sf::base::format("{}", sf::base::String{"abc"}) == sf::base::String{"abc"});
    }

    SECTION("Multiple mixed args")
    {
        CHECK(sf::base::format("{} = {}", "x", 42) == sf::base::String{"x = 42"});
        CHECK(sf::base::format("{}, {}, {}", 1, 2, 3) == sf::base::String{"1, 2, 3"});
        CHECK(sf::base::format("{} + {} = {}", 2, 3, 5) == sf::base::String{"2 + 3 = 5"});
    }

    SECTION("Brace escapes")
    {
        CHECK(sf::base::format("{{}}") == sf::base::String{"{}"});
        CHECK(sf::base::format("{{{}}}", 7) == sf::base::String{"{7}"});
        CHECK(sf::base::format("a{{b}}c") == sf::base::String{"a{b}c"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - precision spec {:.Nf}")
{
    SECTION("Float precision")
    {
        CHECK(sf::base::format("{:.0f}", 3.7) == sf::base::String{"4"});
        CHECK(sf::base::format("{:.1f}", 3.14) == sf::base::String{"3.1"});
        CHECK(sf::base::format("{:.2f}", 3.14) == sf::base::String{"3.14"});
        CHECK(sf::base::format("{:.3f}", 3.14159) == sf::base::String{"3.142"});
        CHECK(sf::base::format("{:.6f}", 1.0) == sf::base::String{"1.000000"});
    }

    SECTION("Negative floats")
    {
        CHECK(sf::base::format("{:.1f}", -2.5) == sf::base::String{"-2.5"});
        CHECK(sf::base::format("{:.3f}", -0.001) == sf::base::String{"-0.001"});
    }

    SECTION("Float precision without `f` tag works the same")
    {
        CHECK(sf::base::format("{:.2}", 3.14159) == sf::base::String{"3.14"});
    }

    SECTION("Integer args ignore precision (no-op)")
    {
        CHECK(sf::base::format("{:.3}", 42) == sf::base::String{"42"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - width spec {:N}")
{
    SECTION("Numbers default to right-align")
    {
        CHECK(sf::base::format("[{:5}]", 42) == sf::base::String{"[   42]"});
        CHECK(sf::base::format("[{:3}]", 1) == sf::base::String{"[  1]"});
    }

    SECTION("Strings default to left-align")
    {
        CHECK(sf::base::format("[{:5}]", "hi") == sf::base::String{"[hi   ]"});
        CHECK(sf::base::format("[{:6}]", sf::base::StringView{"abc"}) == sf::base::String{"[abc   ]"});
    }

    SECTION("Content equal to width -- no padding")
    {
        CHECK(sf::base::format("[{:3}]", 123) == sf::base::String{"[123]"});
        CHECK(sf::base::format("[{:3}]", "abc") == sf::base::String{"[abc]"});
    }

    SECTION("Content wider than width -- no truncation")
    {
        CHECK(sf::base::format("[{:3}]", 12'345) == sf::base::String{"[12345]"});
        CHECK(sf::base::format("[{:2}]", "long string") == sf::base::String{"[long string]"});
    }

    SECTION("Width of zero (degenerate but valid)")
    {
        CHECK(sf::base::format("[{:0}]", 42) == sf::base::String{"[42]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - explicit alignment {:<N}, {:>N}, {:^N}")
{
    SECTION("Right-align '>'")
    {
        CHECK(sf::base::format("[{:>5}]", "hi") == sf::base::String{"[   hi]"});
        CHECK(sf::base::format("[{:>5}]", 42) == sf::base::String{"[   42]"});
    }

    SECTION("Left-align '<'")
    {
        CHECK(sf::base::format("[{:<5}]", "hi") == sf::base::String{"[hi   ]"});
        CHECK(sf::base::format("[{:<5}]", 42) == sf::base::String{"[42   ]"});
    }

    SECTION("Center align '^'")
    {
        // Even-width slack splits evenly (extra goes right).
        CHECK(sf::base::format("[{:^6}]", "hi") == sf::base::String{"[  hi  ]"});
        // Odd-width slack: extra goes to right side.
        CHECK(sf::base::format("[{:^5}]", "hi") == sf::base::String{"[ hi  ]"});
        CHECK(sf::base::format("[{:^5}]", 1) == sf::base::String{"[  1  ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - custom fill character")
{
    SECTION("Zero-pad on the left")
    {
        CHECK(sf::base::format("[{:0>5}]", 42) == sf::base::String{"[00042]"});
        CHECK(sf::base::format("[{:0>5}]", 7) == sf::base::String{"[00007]"});
    }

    SECTION("Dot-fill on the left")
    {
        CHECK(sf::base::format("[{:.>5}]", "ab") == sf::base::String{"[...ab]"});
    }

    SECTION("Dash-center-fill")
    {
        CHECK(sf::base::format("[{:-^7}]", "hi") == sf::base::String{"[--hi---]"});
    }

    SECTION("Custom fill on left-align")
    {
        CHECK(sf::base::format("[{:*<5}]", "ab") == sf::base::String{"[ab***]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - combined specs (width + precision)")
{
    SECTION("Width + precision, right-aligned")
    {
        CHECK(sf::base::format("[{:>10.3f}]", 3.14159) == sf::base::String{"[     3.142]"});
    }

    SECTION("Width + precision, zero-padded")
    {
        CHECK(sf::base::format("[{:0>10.2f}]", 3.14) == sf::base::String{"[0000003.14]"});
    }

    SECTION("Width + precision, left-aligned")
    {
        CHECK(sf::base::format("[{:<10.2f}]", 3.14) == sf::base::String{"[3.14      ]"});
    }

    SECTION("Width + precision, center-aligned")
    {
        CHECK(sf::base::format("[{:^10.2f}]", 3.14) == sf::base::String{"[   3.14   ]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - mixed-spec multi-arg")
{
    const auto result = sf::base::format("[{:>5}|{:<5}|{:^5}]", 1, 2, 3);
    CHECK(result == sf::base::String{"[    1|2    |  3  ]"});

    const auto hud = sf::base::format("FPS: {:>5.1f} | Frame: {:>4}ms | Cells: {:0>4}", 59.95, 16, 42);
    CHECK(hud == sf::base::String{"FPS:  60.0 | Frame:   16ms | Cells: 0042"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - integer type variety")
{
    SECTION("signed/unsigned")
    {
        CHECK(sf::base::format("{}", -32'000) == sf::base::String{"-32000"});
        CHECK(sf::base::format("{}", static_cast<unsigned>(4'000'000'000u)) == sf::base::String{"4000000000"});

        // Narrow signed types must survive `toChars` integer-promotion path
        CHECK(sf::base::format("{}", static_cast<short>(-32'000)) == sf::base::String{"-32000"});
        CHECK(sf::base::format("{}", static_cast<signed char>(-100)) == sf::base::String{"-100"});
    }

    SECTION("long long")
    {
        CHECK(sf::base::format("{}", static_cast<long long>(-1'234'567'890'123LL)) ==
              sf::base::String{"-123456789012"
                               "3"});
        CHECK(sf::base::format("{}", static_cast<unsigned long long>(18'000'000'000'000ULL)) ==
              sf::base::String{"180000"
                               "000000"
                               "00"});
    }

    SECTION("char dispatches as integer")
    {
        // `char` is integral; default formatting prints decimal code, not glyph.
        // We just check it succeeds; exact behavior is implementation-defined-signedness.
        char        buf[16];
        const auto* end = sf::base::formatIntoBuffer(buf, "{}", 'A');
        CHECK(end != nullptr);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - formatIntoBuffer raw API")
{
    char buf[512];

    SECTION("Returns end pointer on success (no implicit null terminator)")
    {
        const auto* end = sf::base::formatIntoBuffer(buf, "x={}", 42);
        REQUIRE(end != nullptr);
        CHECK(sf::base::StringView{buf, static_cast<sf::base::SizeT>(end - buf)} == sf::base::StringView{"x=42"});
    }

    SECTION("Returns nullptr on buffer overflow")
    {
        char small[4];
        // "hello" is 5 bytes, doesn't fit in 4.
        CHECK(sf::base::formatIntoBuffer(small, "hello") == nullptr);
    }

    SECTION("Caller may write their own null terminator")
    {
        // Reserve an extra byte for '\0' and write it manually.
        char        cstr[16];
        auto* const end = sf::base::formatIntoBuffer(cstr, sizeof(cstr) - 1u, "n={}", 7);
        REQUIRE(end != nullptr);
        *end = '\0';
        CHECK(sf::base::StringView{cstr} == sf::base::StringView{"n=7"});
    }

    SECTION("Returns nullptr when arg formatting overflows")
    {
        char small[4];
        CHECK(sf::base::formatIntoBuffer(small, "{}", 1'234'567) == nullptr);
    }

    SECTION("Helper produces same result as format()")
    {
        const auto view = formatToView(buf, "Hello, {}!", "world");
        CHECK(view == sf::base::StringView{"Hello, world!"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - formatTo() into base::String")
{
    SECTION("Appends to non-empty sink")
    {
        sf::base::String s = "prefix:";
        sf::base::formatTo(s, " {} = {:.2f}", "pi", 3.14159);
        CHECK(s == sf::base::String{"prefix: pi = 3.14"});
    }

    SECTION("Format into fresh sink")
    {
        sf::base::String s;
        sf::base::formatTo(s, "{:>8}", "abc");
        CHECK(s == sf::base::String{"     abc"});
    }

    SECTION("Repeated formatTo accumulates")
    {
        sf::base::String s;
        sf::base::formatTo(s, "{}-", 1);
        sf::base::formatTo(s, "{}-", 2);
        sf::base::formatTo(s, "{}", 3);
        CHECK(s == sf::base::String{"1-2-3"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - formatTo() into Utf8String")
{
    SECTION("Build a HUD-style string")
    {
        sf::Utf8String text;
        sf::base::formatTo(text, "Frame: {:>5}ms / Delta: {:.2f}", 16, 0.016667);

        const auto bytes = text.asBytes();
        CHECK(sf::base::StringView{bytes.data(), bytes.size()} == sf::base::StringView{"Frame:    16ms / Delta: 0.02"});
    }

    SECTION("Appends to a Utf8String already containing UTF-8 content")
    {
        sf::Utf8String text{u8"café "};
        sf::base::formatTo(text, "= {}", 42);

        const auto bytes = text.asBytes();
        CHECK(sf::base::StringView{bytes.data(), bytes.size()} == sf::base::StringView{"café = 42"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - edge cases")
{
    SECTION("Empty format string")
    {
        CHECK(sf::base::format("") == sf::base::String{""});
    }

    SECTION("Only escapes")
    {
        CHECK(sf::base::format("{{") == sf::base::String{"{"});
        CHECK(sf::base::format("}}") == sf::base::String{"}"});
        CHECK(sf::base::format("{{{{}}}}") == sf::base::String{"{{}}"});
    }

    SECTION("Empty string arg with width")
    {
        CHECK(sf::base::format("[{:4}]", "") == sf::base::String{"[    ]"});
        CHECK(sf::base::format("[{:>4}]", "") == sf::base::String{"[    ]"});
    }

    SECTION("Adjacent placeholders")
    {
        CHECK(sf::base::format("{}{}{}", 1, 2, 3) == sf::base::String{"123"});
    }
}


////////////////////////////////////////////////////////////
// Custom types: ADL `formatArg` overloads exercising every supported pattern.
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

inline void formatArg(sf::base::FormatSink& sink, const FixedTag& t, const sf::base::FormatSpec&) noexcept
{
    sink.append(t.text, SFML_BASE_STRLEN(t.text));
}


////////////////////////////////////////////////////////////
// Pattern B: small simple composite -- recurse via `sink.format`.
////////////////////////////////////////////////////////////
struct Vec2f
{
    float x;
    float y;
};

inline void formatArg(sf::base::FormatSink& sink, const Vec2f& v, const sf::base::FormatSpec&)
{
    sink.format("({}, {})", v.x, v.y);
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
inline void formatArg(sf::base::FormatSink& sink, const MiniVec<T>& v, const sf::base::FormatSpec&)
{
    sink.appendChar('[');
    for (sf::base::SizeT i = 0u; i < v.size; ++i)
    {
        if (i != 0u)
            sink.append(", ", 2u);
        sink.format("{}", v.data[i]);
    }
    sink.appendChar(']');
}


////////////////////////////////////////////////////////////
// Pattern D: defaults to right-align (specialized variable template).
////////////////////////////////////////////////////////////
struct RightHex
{
    unsigned int value;
};

inline void formatArg(sf::base::FormatSink& sink, const RightHex& h, const sf::base::FormatSpec&)
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

    sink.append(buf, n);
}
} // namespace customtypes
} // anonymous namespace

// Default to right-align for `RightHex` (it's a "numeric-looking" type).
// Specialization lives in `namespace sf::base` to match the primary template;
// the type itself has internal linkage via the anonymous namespace above.
namespace sf::base
{
template <>
inline constexpr char formatArgDefaultAlign<customtypes::RightHex> = '>';
} // namespace sf::base


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - custom formatArg (Pattern A: zero-scratch byte copy)")
{
    using customtypes::FixedTag;

    SECTION("Default formatting")
    {
        CHECK(sf::base::format("{}", FixedTag{"hello"}) == sf::base::String{"hello"});
        CHECK(sf::base::format("[{}]", FixedTag{"world"}) == sf::base::String{"[world]"});
    }

    SECTION("Padding (default left-align)")
    {
        CHECK(sf::base::format("[{:7}]", FixedTag{"hi"}) == sf::base::String{"[hi     ]"});
        CHECK(sf::base::format("[{:>7}]", FixedTag{"hi"}) == sf::base::String{"[     hi]"});
        CHECK(sf::base::format("[{:^7}]", FixedTag{"hi"}) == sf::base::String{"[  hi   ]"});
    }

    SECTION("Custom fill")
    {
        CHECK(sf::base::format("[{:*>6}]", FixedTag{"xy"}) == sf::base::String{"[****xy]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - custom formatArg (Pattern B: recursive composition)")
{
    using customtypes::Vec2f;

    SECTION("Single Vec2f")
    {
        CHECK(sf::base::format("pos = {}", Vec2f{1.5f, 2.0f}) == sf::base::String{"pos = (1.500000, 2.000000)"});
    }

    SECTION("Multiple Vec2f in one format")
    {
        CHECK(sf::base::format("a={} b={}", Vec2f{1.f, 2.f}, Vec2f{3.f, 4.f}) ==
              sf::base::String{"a=(1.000000, "
                               "2.000000) "
                               "b=(3.000000, "
                               "4.000000)"});
    }

    SECTION("Outer width pads the whole composed result")
    {
        // Rendered inner: "(0.000000, 0.000000)" -- exactly 20 chars; width 24 right-pads by 4.
        CHECK(sf::base::format("[{:>24}]", Vec2f{0.0f, 0.0f}) == sf::base::String{"[    (0.000000, 0.000000)]"});
    }

    SECTION("Outer precision is NOT propagated to nested calls (each inner '{}' uses its own spec)")
    {
        // The outer spec applies to the outer placeholder's content (the whole rendered Vec2f),
        // not to the inner float placeholders. So inner floats use default precision 6.
        const auto result = sf::base::format("{:.2}", Vec2f{1.5f, 2.5f});
        CHECK(result == sf::base::String{"(1.500000, 2.500000)"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - custom formatArg (Pattern C: nested containers)")
{
    using customtypes::MiniVec;
    using customtypes::Vec2f;

    SECTION("Container of integers")
    {
        const int          elems[] = {1, 2, 3, 4};
        const MiniVec<int> v{elems, 4u};
        CHECK(sf::base::format("{}", v) == sf::base::String{"[1, 2, 3, 4]"});
    }

    SECTION("Empty container")
    {
        const MiniVec<int> v{nullptr, 0u};
        CHECK(sf::base::format("{}", v) == sf::base::String{"[]"});
    }

    SECTION("Container of Vec2f -- two levels of recursion")
    {
        const Vec2f          points[] = {{1.f, 2.f}, {3.f, 4.f}};
        const MiniVec<Vec2f> v{points, 2u};
        CHECK(sf::base::format("{}", v) == sf::base::String{"[(1.000000, 2.000000), (3.000000, 4.000000)]"});
    }

    SECTION("Container of container -- three levels of recursion")
    {
        const int                   row0[] = {1, 2, 3};
        const int                   row1[] = {4, 5};
        const MiniVec<int>          rows[] = {{row0, 3u}, {row1, 2u}};
        const MiniVec<MiniVec<int>> matrix{rows, 2u};

        CHECK(sf::base::format("{}", matrix) == sf::base::String{"[[1, 2, 3], [4, 5]]"});
    }

    SECTION("Container of containers of Vec2f -- mixed recursion")
    {
        const customtypes::Vec2f                   a[]    = {{1.f, 1.f}};
        const customtypes::Vec2f                   b[]    = {{2.f, 2.f}, {3.f, 3.f}};
        const MiniVec<customtypes::Vec2f>          rows[] = {{a, 1u}, {b, 2u}};
        const MiniVec<MiniVec<customtypes::Vec2f>> grid{rows, 2u};

        CHECK(sf::base::format("{}", grid) == sf::base::String{"[[(1.000000, 1.000000)], [(2.000000, 2.000000), "
                                                               "(3.000000, 3.000000)]]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - custom formatArgDefaultAlign specialization")
{
    using customtypes::RightHex;

    SECTION("Default alignment is now '>' (specialized)")
    {
        CHECK(sf::base::format("[{:6}]", RightHex{0xabu}) == sf::base::String{"[    ab]"});
    }

    SECTION("Explicit '<' overrides the specialized default")
    {
        CHECK(sf::base::format("[{:<6}]", RightHex{0xabu}) == sf::base::String{"[ab    ]"});
    }

    SECTION("Zero value")
    {
        CHECK(sf::base::format("[{:>5}]", RightHex{0u}) == sf::base::String{"[    0]"});
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - FormatSink direct use (low-level API)")
{
    SECTION("Plain bytes + format mix")
    {
        char                 buf[64];
        sf::base::FormatSink sink{buf, sizeof(buf)};

        sink.append("[", 1u);
        sink.format("{}", 42);
        sink.append("] ", 2u);
        sink.appendChar('=');
        sink.format(" {:.1f}", 3.14);

        REQUIRE_FALSE(sink.overflowed());
        CHECK(sf::base::StringView{buf, sink.size()} == sf::base::StringView{"[42] = 3.1"});
    }

    SECTION("Overflow sets sticky bit, further writes no-op")
    {
        char                 buf[5];
        sf::base::FormatSink sink{buf, sizeof(buf)};

        sink.append("hello", 5u); // exactly fills buffer
        CHECK_FALSE(sink.overflowed());

        sink.append("!", 1u); // overflow
        CHECK(sink.overflowed());

        sink.append(" world", 6u); // no-op (still overflowed)
        CHECK(sink.overflowed());
        CHECK(sink.size() == 5u); // size frozen at last good write
    }

    SECTION("Mark + atMark survives interleaved writes")
    {
        char                 buf[64];
        sf::base::FormatSink sink{buf, sizeof(buf)};

        sink.append("aa", 2u);
        const auto m = sink.mark();
        sink.append("bb", 2u);

        CHECK(m == 2u);
        CHECK(sink.atMark(m) == buf + 2);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - formatTo() large output (heap fallback path)")
{
    SECTION("Output larger than 512-byte staging buffer succeeds")
    {
        // Build a format-args list that produces > 512 bytes of output.
        sf::base::String big;
        for (int i = 0; i < 100; ++i)
            big += sf::base::String{"0123456789"}; // 1000 chars

        sf::base::String out;
        sf::base::formatTo(out, "[{}]", sf::base::StringView{big.data(), big.size()});

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
        sf::base::formatTo(out, "{}", sf::base::StringView{mid.data(), mid.size()});

        CHECK(out.size() == mid.size());
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - boundary conditions")
{
    SECTION("Exact fit -- buffer size equals output (no terminator reserved)")
    {
        char        buf[5];
        auto* const end = sf::base::formatIntoBuffer(buf, "hello");
        REQUIRE(end != nullptr);
        CHECK(sf::base::StringView{buf, static_cast<sf::base::SizeT>(end - buf)} == sf::base::StringView{"hello"});
    }

    SECTION("Off-by-one -- one byte short")
    {
        char        buf[4];
        auto* const end = sf::base::formatIntoBuffer(buf, "hello");
        CHECK(end == nullptr);
    }

    SECTION("Very wide width is honored")
    {
        sf::base::String out;
        sf::base::formatTo(out, "[{:>1000}]", "x");
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
        CHECK(sf::base::format("[{:0}]", "abc") == sf::base::String{"[abc]"});
    }

    SECTION("Float at boundary of supported precision (10)")
    {
        CHECK(sf::base::format("{:.10f}", 0.5) == sf::base::String{"0.5000000000"});
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
        sf::base::formatTo(out, "{}", v);

        CHECK(out.size() == 1320u);
        CHECK(out.data()[0] == '[');
        CHECK(out.data()[out.size() - 1u] == ']');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[System] MiniFmt.hpp - composition + width interaction")
{
    SECTION("Outer width pads composed inner content uniformly")
    {
        using customtypes::Vec2f;

        // Rendered inner: "(1.000000, 2.000000)" -- 20 chars.
        CHECK(sf::base::format("[{:<25}]", Vec2f{1.f, 2.f}) == sf::base::String{"[(1.000000, 2.000000)     ]"});
    }

    SECTION("Multiple levels of recursion + outer padding")
    {
        const int                       elems[] = {7, 8};
        const customtypes::MiniVec<int> v{elems, 2u};

        // Rendered inner: "[7, 8]" (6 chars). Outer width 10, right-align:
        CHECK(sf::base::format("[{:>10}]", v) == sf::base::String{"[    [7, 8]]"});
    }
}
