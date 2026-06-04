#include "StringifyStringViewUtil.hpp"
#include "StringifyZbStringUtil.hpp"
#include "Tst/Tst.hpp"

#include "ZancleBase/Scn/Scn.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Radix.hpp"
#include "ZancleBase/Scn/ScnNumeric.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/Scn/ScnStringSource.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"


////////////////////////////////////////////////////////////
namespace
{
[[nodiscard]] zb::ScnStringSource stringSource(zb::StringView v)
{
    return zb::ScnStringSource{v};
}


////////////////////////////////////////////////////////////
// Pure-`ScnSource` wrapper that hides `remaining()`/`advance()` -- forces
// parsers to take the slow per-byte fallback path. Used in tests that
// need to validate the non-contiguous codepath.
class NonContiguousSource
{
public:
    explicit NonContiguousSource(zb::StringView v) noexcept : m_inner{v}
    {
    }

    [[nodiscard]] zb::Optional<char> peek() const noexcept
    {
        return m_inner.peek();
    }

    void consume() noexcept
    {
        m_inner.consume();
    }

private:
    zb::ScnStringSource m_inner;
};

static_assert(zb::ScnSource<NonContiguousSource>);
static_assert(!zb::ContiguousScnSource<NonContiguousSource>);

static_assert(zb::ScnSource<zb::ScnStringSource>);
static_assert(zb::ContiguousScnSource<zb::ScnStringSource>);

namespace customscn
{
struct MyVec2
{
    int x = 0;
    int y = 0;
};

inline bool scnArg(zb::ScnStringSource& src, MyVec2& out)
{
    // Format: `x,y`
    if (!zb::scnInto(src, out.x))
        return false;

    auto c = src.peek();
    if (!c || *c != ',')
        return false;

    src.consume();
    return zb::scnInto(src, out.y);
}

struct BaseValue
{
    int value = 0;
};

struct DerivedValue : BaseValue
{
};

inline bool scnArg(zb::ScnStringSource& src, BaseValue& out)
{
    return zb::scnInto(src, out.value);
}
} // namespace customscn

} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - empty input")
{
    auto src = stringSource({});

    CHECK(zb::scnAtEnd(src));
    CHECK(src.bytesConsumed() == 0u);

    char c = 0;
    CHECK_FALSE(zb::scnInto(src, c));

    zb::String s;
    CHECK_FALSE(zb::scnInto(src, s));

    int n = 0;
    CHECK_FALSE(zb::scnInto(src, n));

    bool b = true;
    CHECK_FALSE(zb::scnInto(src, b));

    CHECK_FALSE(zb::scn<int>(src).hasValue());
    CHECK_FALSE(zb::scn<zb::String>(src).hasValue());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnArg<char> does NOT skip whitespace")
{
    auto src = stringSource("  ab");

    char c = 0;
    CHECK(zb::scnInto(src, c));
    CHECK(c == ' ');

    CHECK(zb::scnInto(src, c));
    CHECK(c == ' ');

    CHECK(zb::scnInto(src, c));
    CHECK(c == 'a');

    CHECK(zb::scnInto(src, c));
    CHECK(c == 'b');

    CHECK_FALSE(zb::scnInto(src, c));
    CHECK(zb::scnAtEnd(src));
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnArg<String> reads whitespace-delimited token")
{
    SUBCASE("token sequence")
    {
        auto src = stringSource("foo bar baz");

        const auto a = zb::scn<zb::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == zb::String{"foo"});

        const auto b = zb::scn<zb::String>(src);
        REQUIRE(b.hasValue());
        CHECK(*b == zb::String{"bar"});

        const auto c = zb::scn<zb::String>(src);
        REQUIRE(c.hasValue());
        CHECK(*c == zb::String{"baz"});

        CHECK_FALSE(zb::scn<zb::String>(src).hasValue());
    }

    SUBCASE("leading whitespace is skipped")
    {
        auto src = stringSource("   hello\tworld\n");

        const auto a = zb::scn<zb::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == zb::String{"hello"});

        const auto b = zb::scn<zb::String>(src);
        REQUIRE(b.hasValue());
        CHECK(*b == zb::String{"world"});

        CHECK_FALSE(zb::scn<zb::String>(src).hasValue());
    }

    SUBCASE("trailing whitespace produces a final token then EOF")
    {
        auto src = stringSource("only   ");

        const auto a = zb::scn<zb::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == zb::String{"only"});

        CHECK_FALSE(zb::scn<zb::String>(src).hasValue());
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnReadLine")
{
    using zb::scnReadLine;

    SUBCASE("multiple lines, LF endings")
    {
        auto src = stringSource("alpha\nbeta\ngamma\n");

        zb::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"alpha"});

        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"beta"});

        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"gamma"});

        CHECK_FALSE(scnReadLine(src, line));
    }

    SUBCASE("CRLF endings leave the '\\r' in the result")
    {
        auto src = stringSource("alpha\r\nbeta\r\n");

        zb::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"alpha\r"});

        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"beta\r"});
    }

    SUBCASE("missing trailing newline still returns the last line")
    {
        auto src = stringSource("first\nlast-no-newline");

        zb::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"first"});

        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"last-no-newline"});

        CHECK_FALSE(scnReadLine(src, line));
    }

    SUBCASE("empty lines are reported as success with empty string")
    {
        auto src = stringSource("\n\nx\n");

        zb::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line.empty());

        CHECK(scnReadLine(src, line));
        CHECK(line.empty());

        CHECK(scnReadLine(src, line));
        CHECK(line == zb::String{"x"});

        CHECK_FALSE(scnReadLine(src, line));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - integer parsing skips whitespace")
{
    SUBCASE("multiple signed values")
    {
        auto src = stringSource("42 -7 100");

        CHECK(*zb::scn<int>(src) == 42);
        CHECK(*zb::scn<int>(src) == -7);
        CHECK(*zb::scn<int>(src) == 100);

        CHECK_FALSE(zb::scn<int>(src).hasValue());
    }

    SUBCASE("stops at first non-digit")
    {
        auto src = stringSource("123.45");

        const auto a = zb::scn<int>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == 123);

        // The '.' is still on the source -- it is not a valid int byte.
        CHECK_FALSE(zb::scn<int>(src).hasValue());
    }

    SUBCASE("non-numeric input fails")
    {
        auto src = stringSource("hello");

        CHECK_FALSE(zb::scn<int>(src).hasValue());
    }

    SUBCASE("unsigned rejects negative")
    {
        auto src = stringSource("-1x");

        CHECK_FALSE(zb::scn<unsigned int>(src).hasValue());
        CHECK(src.bytesConsumed() == 2u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("signed minimum is accepted")
    {
        auto src = stringSource("-2147483648");

        const auto v = zb::scn<int>(src);
        REQUIRE(v.hasValue());
        CHECK(*v == -2'147'483'648);
    }

    SUBCASE("signed below minimum is consumed and rejected")
    {
        auto src = stringSource("-2147483649x");

        int v = 123;
        CHECK_FALSE(zb::scnInto(src, v));
        CHECK(v == 123);
        CHECK(src.bytesConsumed() == 11u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("many leading zeroes are accepted")
    {
        zb::String input;
        for (zb::SizeT i = 0u; i < 32u; ++i)
            input.append('0');
        input.append("1x");

        auto src = stringSource(input.toStringView());

        unsigned v = 123u;
        CHECK(zb::scnInto(src, v));
        CHECK(v == 1u);
        CHECK(src.bytesConsumed() == 33u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("overflowing digit run is consumed and rejected")
    {
        auto src = stringSource("99999999999999999999999999999999x");

        unsigned v = 123u;
        CHECK_FALSE(zb::scnInto(src, v));
        CHECK(v == 123u);
        CHECK(src.bytesConsumed() == 32u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnRadix")
{
    using zb::Radix;
    using zb::scnRadix;

    SUBCASE("hex lowercase")
    {
        auto      src = stringSource("abcd");
        zb::SizeT v   = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xab'cdu);
    }

    SUBCASE("hex stops at first non-hex byte")
    {
        auto src = stringSource("ff\n0\n");

        zb::SizeT v = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xffu);

        zb::scnSkipWhitespace(src);

        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0u);
    }

    SUBCASE("octal rejects '8'")
    {
        auto     src = stringSource("78");
        unsigned v   = 0u;
        CHECK(scnRadix(src, v, Radix::Oct));
        CHECK(v == 7u);
    }

    SUBCASE("leading whitespace is skipped")
    {
        auto     src = stringSource("   1a");
        unsigned v   = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0x1au);
    }

    SUBCASE("overlong radix digit run is consumed and rejected")
    {
        zb::String input;
        for (zb::SizeT i = 0u; i < 70u; ++i)
            input.append('f');
        input.append('z');

        auto src = stringSource(input.toStringView());

        unsigned v = 123u;
        CHECK_FALSE(scnRadix(src, v, Radix::Hex));
        CHECK(v == 123u);
        CHECK(src.bytesConsumed() == 70u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'z');
    }

    SUBCASE("many leading zeroes followed by small value parse correctly")
    {
        // Incremental parser accepts arbitrarily many leading zeroes
        // since the accumulated value stays in range. The previous
        // scratch-buffer impl rejected this once the run exceeded
        // `scnNumericScratchSize`.
        zb::String input;
        for (zb::SizeT i = 0u; i < 200u; ++i)
            input.append('0');
        input.append("ff");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xffu);
        CHECK(src.bytesConsumed() == 202u);
    }

    SUBCASE("octal with leading zeroes")
    {
        zb::String input;
        for (zb::SizeT i = 0u; i < 100u; ++i)
            input.append('0');
        input.append("17");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Oct));
        CHECK(v == 017u);
    }

    SUBCASE("binary with leading zeroes")
    {
        zb::String input;
        for (zb::SizeT i = 0u; i < 100u; ++i)
            input.append('0');
        input.append("1011");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Bin));
        CHECK(v == 0b1011u);
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - float parsing")
{
    SUBCASE("positive and negative")
    {
        auto src = stringSource("3.14 -2.5 0.0");

        CHECK(*zb::scn<double>(src) == tst::Approx(3.14));
        CHECK(*zb::scn<double>(src) == tst::Approx(-2.5));
        CHECK(*zb::scn<double>(src) == tst::Approx(0.0));
    }

    SUBCASE("integer-only is accepted")
    {
        auto src = stringSource("42");

        CHECK(*zb::scn<float>(src) == tst::Approx(42.0));
    }

    SUBCASE("overlong float token is consumed and rejected")
    {
        zb::String input;
        for (zb::SizeT i = 0u; i < 45u; ++i)
            input.append('1');
        input.append(".25x");

        auto src = stringSource(input.toStringView());

        double v = 12.0;
        CHECK_FALSE(zb::scnInto(src, v));
        CHECK(v == tst::Approx(12.0));
        CHECK(src.bytesConsumed() == 48u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - bool parsing")
{
    SUBCASE("literal true/false")
    {
        auto src = stringSource("true false true");

        CHECK(*zb::scn<bool>(src) == true);
        CHECK(*zb::scn<bool>(src) == false);
        CHECK(*zb::scn<bool>(src) == true);
    }

    SUBCASE("digit form")
    {
        auto src = stringSource("0 1 0");

        CHECK(*zb::scn<bool>(src) == false);
        CHECK(*zb::scn<bool>(src) == true);
        CHECK(*zb::scn<bool>(src) == false);
    }

    SUBCASE("garbage rejected")
    {
        auto src = stringSource("hello");

        CHECK_FALSE(zb::scn<bool>(src).hasValue());
    }

    SUBCASE("'truncated' literal fails")
    {
        auto src = stringSource("trux");

        CHECK_FALSE(zb::scn<bool>(src).hasValue());
        CHECK(src.bytesConsumed() == 3u);

        char c = '\0';
        CHECK(zb::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnSkipPast")
{
    SUBCASE("delimiter found in the middle")
    {
        auto src = stringSource("garbage:value");

        zb::scnSkipPast(src, ':');

        const auto tok = zb::scn<zb::String>(src);
        REQUIRE(tok.hasValue());
        CHECK(*tok == zb::String{"value"});
    }

    SUBCASE("delimiter not present: drain the source")
    {
        auto src = stringSource("no-delim");

        zb::scnSkipPast(src, ':');

        CHECK(zb::scnAtEnd(src));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - mixed value types in one source")
{
    auto src = stringSource("42 hello 3.14 true x");

    CHECK(*zb::scn<int>(src) == 42);

    const auto tok = zb::scn<zb::String>(src);
    REQUIRE(tok.hasValue());
    CHECK(*tok == zb::String{"hello"});

    CHECK(*zb::scn<double>(src) == tst::Approx(3.14));
    CHECK(*zb::scn<bool>(src) == true);

    // `scnArg<char>` does not skip leading whitespace.
    zb::scnSkipWhitespace(src);

    char c = 0;
    CHECK(zb::scnInto(src, c));
    CHECK(c == 'x');
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - non-contiguous source takes the fallback path")
{
    NonContiguousSource src{"hello world\nline2"};

    zb::String tok;
    CHECK(zb::scnInto(src, tok));
    CHECK(tok == zb::String{"hello"});

    CHECK(zb::scnInto(src, tok));
    CHECK(tok == zb::String{"world"});

    zb::String line;
    CHECK(zb::scnReadLine(src, line));
    // The next byte after "world" is '\n', so the line is empty -- but
    // `scnArg<String>` left the source positioned at the '\n' itself
    // since the trailing whitespace check stopped there. `scnReadLine`
    // therefore consumes the '\n' and returns an empty `line`.
    CHECK(line.empty());

    CHECK(zb::scnReadLine(src, line));
    CHECK(line == zb::String{"line2"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - long inputs (contiguous fast path)")
{
    // Push a 4 KiB token through to exercise the bulk-append branch.
    zb::String payload;
    payload.reserve(4096u + 8u);
    for (zb::SizeT i = 0u; i < 4096u; ++i)
        payload.append('a');
    payload.append(" tail");

    auto src = zb::ScnStringSource{payload.toStringView()};

    const auto tok = zb::scn<zb::String>(src);
    REQUIRE(tok.hasValue());
    CHECK(tok->size() == 4096u);

    const auto tail = zb::scn<zb::String>(src);
    REQUIRE(tail.hasValue());
    CHECK(*tail == zb::String{"tail"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ScnStringSource bytesConsumed tracks position")
{
    auto src = stringSource("42 hello");

    CHECK(src.bytesConsumed() == 0u);
    CHECK(*zb::scn<int>(src) == 42);
    CHECK(src.bytesConsumed() == 2u);

    zb::String tok;
    CHECK(zb::scnInto(src, tok));
    CHECK(tok == zb::String{"hello"});
    CHECK(src.bytesConsumed() == 8u);
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ScnStringSource advance covers full range without overflow")
{
    // Regression: the precondition used to be `m_pos + n <= size()`,
    // which could wrap for huge `n`. The non-overflowing form
    // `n <= size() - m_pos` is exercised below at the boundary
    // (advance exactly to end, advance zero, advance after partial).
    zb::ScnStringSource src{"abcdef"};

    src.advance(0u);
    CHECK(src.bytesConsumed() == 0u);
    CHECK(src.remaining().size() == 6u);

    src.advance(3u);
    CHECK(src.bytesConsumed() == 3u);
    CHECK(src.remaining() == zb::StringView{"def"});

    // Boundary: advance exactly to end.
    src.advance(3u);
    CHECK(src.bytesConsumed() == 6u);
    CHECK(src.remaining().empty());
    CHECK(zb::scnAtEnd(src));

    // Zero-advance at end is still valid.
    src.advance(0u);
    CHECK(src.remaining().empty());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ADL on a user-defined type via custom scnArg")
{
    auto              src = stringSource("3,7");
    customscn::MyVec2 v{};
    CHECK(zb::scnInto(src, v));
    CHECK(v.x == 3);
    CHECK(v.y == 7);
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - base-class ADL customization remains viable")
{
    auto src = stringSource("42");

    customscn::DerivedValue v{};
    CHECK(zb::scnInto(src, v));
    CHECK(v.value == 42);
}
