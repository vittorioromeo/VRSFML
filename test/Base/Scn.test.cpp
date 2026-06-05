#include "StringifyStringViewUtil.hpp"
#include "StringifyZbStringUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Scn/Scn.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/Radix.hpp"
#include "Zancle/Scn/ScnNumeric.hpp"
#include "Zancle/Scn/ScnString.hpp"
#include "Zancle/Scn/ScnStringSource.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"


////////////////////////////////////////////////////////////
namespace
{
[[nodiscard]] za::ScnStringSource stringSource(za::StringView v)
{
    return za::ScnStringSource{v};
}


////////////////////////////////////////////////////////////
// Pure-`ScnSource` wrapper that hides `remaining()`/`advance()` -- forces
// parsers to take the slow per-byte fallback path. Used in tests that
// need to validate the non-contiguous codepath.
class NonContiguousSource
{
public:
    explicit NonContiguousSource(za::StringView v) noexcept : m_inner{v}
    {
    }

    [[nodiscard]] za::Optional<char> peek() const noexcept
    {
        return m_inner.peek();
    }

    void consume() noexcept
    {
        m_inner.consume();
    }

private:
    za::ScnStringSource m_inner;
};

static_assert(za::ScnSource<NonContiguousSource>);
static_assert(!za::ContiguousScnSource<NonContiguousSource>);

static_assert(za::ScnSource<za::ScnStringSource>);
static_assert(za::ContiguousScnSource<za::ScnStringSource>);

namespace customscn
{
struct MyVec2
{
    int x = 0;
    int y = 0;
};

inline bool scnArg(za::ScnStringSource& src, MyVec2& out)
{
    // Format: `x,y`
    if (!za::scnInto(src, out.x))
        return false;

    auto c = src.peek();
    if (!c || *c != ',')
        return false;

    src.consume();
    return za::scnInto(src, out.y);
}

struct BaseValue
{
    int value = 0;
};

struct DerivedValue : BaseValue
{
};

inline bool scnArg(za::ScnStringSource& src, BaseValue& out)
{
    return za::scnInto(src, out.value);
}
} // namespace customscn

} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - empty input")
{
    auto src = stringSource({});

    CHECK(za::scnAtEnd(src));
    CHECK(src.bytesConsumed() == 0u);

    char c = 0;
    CHECK_FALSE(za::scnInto(src, c));

    za::String s;
    CHECK_FALSE(za::scnInto(src, s));

    int n = 0;
    CHECK_FALSE(za::scnInto(src, n));

    bool b = true;
    CHECK_FALSE(za::scnInto(src, b));

    CHECK_FALSE(za::scn<int>(src).hasValue());
    CHECK_FALSE(za::scn<za::String>(src).hasValue());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnArg<char> does NOT skip whitespace")
{
    auto src = stringSource("  ab");

    char c = 0;
    CHECK(za::scnInto(src, c));
    CHECK(c == ' ');

    CHECK(za::scnInto(src, c));
    CHECK(c == ' ');

    CHECK(za::scnInto(src, c));
    CHECK(c == 'a');

    CHECK(za::scnInto(src, c));
    CHECK(c == 'b');

    CHECK_FALSE(za::scnInto(src, c));
    CHECK(za::scnAtEnd(src));
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnArg<String> reads whitespace-delimited token")
{
    SUBCASE("token sequence")
    {
        auto src = stringSource("foo bar baz");

        const auto a = za::scn<za::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == za::String{"foo"});

        const auto b = za::scn<za::String>(src);
        REQUIRE(b.hasValue());
        CHECK(*b == za::String{"bar"});

        const auto c = za::scn<za::String>(src);
        REQUIRE(c.hasValue());
        CHECK(*c == za::String{"baz"});

        CHECK_FALSE(za::scn<za::String>(src).hasValue());
    }

    SUBCASE("leading whitespace is skipped")
    {
        auto src = stringSource("   hello\tworld\n");

        const auto a = za::scn<za::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == za::String{"hello"});

        const auto b = za::scn<za::String>(src);
        REQUIRE(b.hasValue());
        CHECK(*b == za::String{"world"});

        CHECK_FALSE(za::scn<za::String>(src).hasValue());
    }

    SUBCASE("trailing whitespace produces a final token then EOF")
    {
        auto src = stringSource("only   ");

        const auto a = za::scn<za::String>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == za::String{"only"});

        CHECK_FALSE(za::scn<za::String>(src).hasValue());
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnReadLine")
{
    using za::scnReadLine;

    SUBCASE("multiple lines, LF endings")
    {
        auto src = stringSource("alpha\nbeta\ngamma\n");

        za::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"alpha"});

        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"beta"});

        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"gamma"});

        CHECK_FALSE(scnReadLine(src, line));
    }

    SUBCASE("CRLF endings leave the '\\r' in the result")
    {
        auto src = stringSource("alpha\r\nbeta\r\n");

        za::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"alpha\r"});

        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"beta\r"});
    }

    SUBCASE("missing trailing newline still returns the last line")
    {
        auto src = stringSource("first\nlast-no-newline");

        za::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"first"});

        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"last-no-newline"});

        CHECK_FALSE(scnReadLine(src, line));
    }

    SUBCASE("empty lines are reported as success with empty string")
    {
        auto src = stringSource("\n\nx\n");

        za::String line;
        CHECK(scnReadLine(src, line));
        CHECK(line.empty());

        CHECK(scnReadLine(src, line));
        CHECK(line.empty());

        CHECK(scnReadLine(src, line));
        CHECK(line == za::String{"x"});

        CHECK_FALSE(scnReadLine(src, line));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - integer parsing skips whitespace")
{
    SUBCASE("multiple signed values")
    {
        auto src = stringSource("42 -7 100");

        CHECK(*za::scn<int>(src) == 42);
        CHECK(*za::scn<int>(src) == -7);
        CHECK(*za::scn<int>(src) == 100);

        CHECK_FALSE(za::scn<int>(src).hasValue());
    }

    SUBCASE("stops at first non-digit")
    {
        auto src = stringSource("123.45");

        const auto a = za::scn<int>(src);
        REQUIRE(a.hasValue());
        CHECK(*a == 123);

        // The '.' is still on the source -- it is not a valid int byte.
        CHECK_FALSE(za::scn<int>(src).hasValue());
    }

    SUBCASE("non-numeric input fails")
    {
        auto src = stringSource("hello");

        CHECK_FALSE(za::scn<int>(src).hasValue());
    }

    SUBCASE("unsigned rejects negative")
    {
        auto src = stringSource("-1x");

        CHECK_FALSE(za::scn<unsigned int>(src).hasValue());
        CHECK(src.bytesConsumed() == 2u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("signed minimum is accepted")
    {
        auto src = stringSource("-2147483648");

        const auto v = za::scn<int>(src);
        REQUIRE(v.hasValue());
        CHECK(*v == -2'147'483'648);
    }

    SUBCASE("signed below minimum is consumed and rejected")
    {
        auto src = stringSource("-2147483649x");

        int v = 123;
        CHECK_FALSE(za::scnInto(src, v));
        CHECK(v == 123);
        CHECK(src.bytesConsumed() == 11u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("many leading zeroes are accepted")
    {
        za::String input;
        for (za::SizeT i = 0u; i < 32u; ++i)
            input.append('0');
        input.append("1x");

        auto src = stringSource(input.toStringView());

        unsigned v = 123u;
        CHECK(za::scnInto(src, v));
        CHECK(v == 1u);
        CHECK(src.bytesConsumed() == 33u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'x');
    }

    SUBCASE("overflowing digit run is consumed and rejected")
    {
        auto src = stringSource("99999999999999999999999999999999x");

        unsigned v = 123u;
        CHECK_FALSE(za::scnInto(src, v));
        CHECK(v == 123u);
        CHECK(src.bytesConsumed() == 32u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnRadix")
{
    using za::Radix;
    using za::scnRadix;

    SUBCASE("hex lowercase")
    {
        auto      src = stringSource("abcd");
        za::SizeT v   = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xab'cdu);
    }

    SUBCASE("hex stops at first non-hex byte")
    {
        auto src = stringSource("ff\n0\n");

        za::SizeT v = 0u;
        CHECK(scnRadix(src, v, Radix::Hex));
        CHECK(v == 0xffu);

        za::scnSkipWhitespace(src);

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
        za::String input;
        for (za::SizeT i = 0u; i < 70u; ++i)
            input.append('f');
        input.append('z');

        auto src = stringSource(input.toStringView());

        unsigned v = 123u;
        CHECK_FALSE(scnRadix(src, v, Radix::Hex));
        CHECK(v == 123u);
        CHECK(src.bytesConsumed() == 70u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'z');
    }

    SUBCASE("many leading zeroes followed by small value parse correctly")
    {
        // Incremental parser accepts arbitrarily many leading zeroes
        // since the accumulated value stays in range. The previous
        // scratch-buffer impl rejected this once the run exceeded
        // `scnNumericScratchSize`.
        za::String input;
        for (za::SizeT i = 0u; i < 200u; ++i)
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
        za::String input;
        for (za::SizeT i = 0u; i < 100u; ++i)
            input.append('0');
        input.append("17");

        auto src = stringSource(input.toStringView());

        unsigned v = 0u;
        CHECK(scnRadix(src, v, Radix::Oct));
        CHECK(v == 017u);
    }

    SUBCASE("binary with leading zeroes")
    {
        za::String input;
        for (za::SizeT i = 0u; i < 100u; ++i)
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

        CHECK(*za::scn<double>(src) == tst::Approx(3.14));
        CHECK(*za::scn<double>(src) == tst::Approx(-2.5));
        CHECK(*za::scn<double>(src) == tst::Approx(0.0));
    }

    SUBCASE("integer-only is accepted")
    {
        auto src = stringSource("42");

        CHECK(*za::scn<float>(src) == tst::Approx(42.0));
    }

    SUBCASE("overlong float token is consumed and rejected")
    {
        za::String input;
        for (za::SizeT i = 0u; i < 45u; ++i)
            input.append('1');
        input.append(".25x");

        auto src = stringSource(input.toStringView());

        double v = 12.0;
        CHECK_FALSE(za::scnInto(src, v));
        CHECK(v == tst::Approx(12.0));
        CHECK(src.bytesConsumed() == 48u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - bool parsing")
{
    SUBCASE("literal true/false")
    {
        auto src = stringSource("true false true");

        CHECK(*za::scn<bool>(src) == true);
        CHECK(*za::scn<bool>(src) == false);
        CHECK(*za::scn<bool>(src) == true);
    }

    SUBCASE("digit form")
    {
        auto src = stringSource("0 1 0");

        CHECK(*za::scn<bool>(src) == false);
        CHECK(*za::scn<bool>(src) == true);
        CHECK(*za::scn<bool>(src) == false);
    }

    SUBCASE("garbage rejected")
    {
        auto src = stringSource("hello");

        CHECK_FALSE(za::scn<bool>(src).hasValue());
    }

    SUBCASE("'truncated' literal fails")
    {
        auto src = stringSource("trux");

        CHECK_FALSE(za::scn<bool>(src).hasValue());
        CHECK(src.bytesConsumed() == 3u);

        char c = '\0';
        CHECK(za::scnInto(src, c));
        CHECK(c == 'x');
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - scnSkipPast")
{
    SUBCASE("delimiter found in the middle")
    {
        auto src = stringSource("garbage:value");

        za::scnSkipPast(src, ':');

        const auto tok = za::scn<za::String>(src);
        REQUIRE(tok.hasValue());
        CHECK(*tok == za::String{"value"});
    }

    SUBCASE("delimiter not present: drain the source")
    {
        auto src = stringSource("no-delim");

        za::scnSkipPast(src, ':');

        CHECK(za::scnAtEnd(src));
    }
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - mixed value types in one source")
{
    auto src = stringSource("42 hello 3.14 true x");

    CHECK(*za::scn<int>(src) == 42);

    const auto tok = za::scn<za::String>(src);
    REQUIRE(tok.hasValue());
    CHECK(*tok == za::String{"hello"});

    CHECK(*za::scn<double>(src) == tst::Approx(3.14));
    CHECK(*za::scn<bool>(src) == true);

    // `scnArg<char>` does not skip leading whitespace.
    za::scnSkipWhitespace(src);

    char c = 0;
    CHECK(za::scnInto(src, c));
    CHECK(c == 'x');
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - non-contiguous source takes the fallback path")
{
    NonContiguousSource src{"hello world\nline2"};

    za::String tok;
    CHECK(za::scnInto(src, tok));
    CHECK(tok == za::String{"hello"});

    CHECK(za::scnInto(src, tok));
    CHECK(tok == za::String{"world"});

    za::String line;
    CHECK(za::scnReadLine(src, line));
    // The next byte after "world" is '\n', so the line is empty -- but
    // `scnArg<String>` left the source positioned at the '\n' itself
    // since the trailing whitespace check stopped there. `scnReadLine`
    // therefore consumes the '\n' and returns an empty `line`.
    CHECK(line.empty());

    CHECK(za::scnReadLine(src, line));
    CHECK(line == za::String{"line2"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - long inputs (contiguous fast path)")
{
    // Push a 4 KiB token through to exercise the bulk-append branch.
    za::String payload;
    payload.reserve(4096u + 8u);
    for (za::SizeT i = 0u; i < 4096u; ++i)
        payload.append('a');
    payload.append(" tail");

    auto src = za::ScnStringSource{payload.toStringView()};

    const auto tok = za::scn<za::String>(src);
    REQUIRE(tok.hasValue());
    CHECK(tok->size() == 4096u);

    const auto tail = za::scn<za::String>(src);
    REQUIRE(tail.hasValue());
    CHECK(*tail == za::String{"tail"});
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ScnStringSource bytesConsumed tracks position")
{
    auto src = stringSource("42 hello");

    CHECK(src.bytesConsumed() == 0u);
    CHECK(*za::scn<int>(src) == 42);
    CHECK(src.bytesConsumed() == 2u);

    za::String tok;
    CHECK(za::scnInto(src, tok));
    CHECK(tok == za::String{"hello"});
    CHECK(src.bytesConsumed() == 8u);
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ScnStringSource advance covers full range without overflow")
{
    // Regression: the precondition used to be `m_pos + n <= size()`,
    // which could wrap for huge `n`. The non-overflowing form
    // `n <= size() - m_pos` is exercised below at the boundary
    // (advance exactly to end, advance zero, advance after partial).
    za::ScnStringSource src{"abcdef"};

    src.advance(0u);
    CHECK(src.bytesConsumed() == 0u);
    CHECK(src.remaining().size() == 6u);

    src.advance(3u);
    CHECK(src.bytesConsumed() == 3u);
    CHECK(src.remaining() == za::StringView{"def"});

    // Boundary: advance exactly to end.
    src.advance(3u);
    CHECK(src.bytesConsumed() == 6u);
    CHECK(src.remaining().empty());
    CHECK(za::scnAtEnd(src));

    // Zero-advance at end is still valid.
    src.advance(0u);
    CHECK(src.remaining().empty());
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - ADL on a user-defined type via custom scnArg")
{
    auto              src = stringSource("3,7");
    customscn::MyVec2 v{};
    CHECK(za::scnInto(src, v));
    CHECK(v.x == 3);
    CHECK(v.y == 7);
}


////////////////////////////////////////////////////////////
TEST_CASE("[Base] Scn - base-class ADL customization remains viable")
{
    auto src = stringSource("42");

    customscn::DerivedValue v{};
    CHECK(za::scnInto(src, v));
    CHECK(v.value == 42);
}
