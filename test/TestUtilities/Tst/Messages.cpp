// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#include "Tst/Detail/State.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: keep
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToChars.hpp"


////////////////////////////////////////////////////////////
// Implements the message-related glue: scoped INFO push/pop,
// one-shot MESSAGE/FAIL emission, the `MessageBuilder` overloads, and
// the CHECK_NOTHROW helpers.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
void MessageBuilder::appendRaw(const char* data, sf::base::SizeT n) noexcept
{
    if (n == 0u)
        return;

    const sf::base::SizeT room     = capacity - len;
    const sf::base::SizeT toAppend = n < room ? n : room;

    if (toAppend == 0u)
        return;

    SFML_BASE_MEMCPY(buf + len, data, toAppend);
    len += toAppend;
}


namespace
{
////////////////////////////////////////////////////////////
void appendCString(MessageBuilder& mb, const char* v) noexcept
{
    if (v == nullptr)
        return;

    mb.appendRaw(v, SFML_BASE_STRLEN(v));
}


////////////////////////////////////////////////////////////
void appendStringView(MessageBuilder& mb, sf::base::StringView v) noexcept
{
    mb.appendRaw(v.data(), v.size());
}


////////////////////////////////////////////////////////////
template <typename T>
void appendNumeric(MessageBuilder& mb, T v) noexcept
{
    char        tmp[64];
    char* const end = sf::base::toChars(tmp, tmp + sizeof(tmp), v);
    if (end == nullptr)
        return;
    mb.appendRaw(tmp, static_cast<sf::base::SizeT>(end - tmp));
}


////////////////////////////////////////////////////////////
void appendBool(MessageBuilder& mb, bool v) noexcept
{
    if (v)
        mb.appendRaw("true", 4u);
    else
        mb.appendRaw("false", 5u);
}


////////////////////////////////////////////////////////////
void appendChar(MessageBuilder& mb, char c) noexcept
{
    mb.appendRaw(&c, 1u);
}

} // namespace


////////////////////////////////////////////////////////////
// `MessageBuilder` overload bodies. Macros stamp out the `&` / `&&`
// pairs for each supported type so the header can stay a tight list
// of declarations.
////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
#define SFML_TST_MSGB_DEF_PAIR(T, body)                          \
                                                                 \
    MessageBuilder& MessageBuilder::operator*(T v) & noexcept    \
    {                                                            \
        body;                                                    \
        return *this;                                            \
    }                                                            \
                                                                 \
    MessageBuilder& MessageBuilder::operator<<(T v) & noexcept   \
    {                                                            \
        body;                                                    \
        return *this;                                            \
    }                                                            \
                                                                 \
    MessageBuilder&& MessageBuilder::operator*(T v) && noexcept  \
    {                                                            \
        body;                                                    \
        return static_cast<MessageBuilder&&>(*this);             \
    }                                                            \
                                                                 \
    MessageBuilder&& MessageBuilder::operator<<(T v) && noexcept \
    {                                                            \
        body;                                                    \
        return static_cast<MessageBuilder&&>(*this);             \
    }

SFML_TST_MSGB_DEF_PAIR(const char*, appendCString(*this, v))
SFML_TST_MSGB_DEF_PAIR(sf::base::StringView, appendStringView(*this, v))
SFML_TST_MSGB_DEF_PAIR(bool, appendBool(*this, v))
SFML_TST_MSGB_DEF_PAIR(char, appendChar(*this, v))
SFML_TST_MSGB_DEF_PAIR(short, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(unsigned short, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(int, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(unsigned int, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(long, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(unsigned long, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(long long, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(unsigned long long, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(float, appendNumeric(*this, v))
SFML_TST_MSGB_DEF_PAIR(double, appendNumeric(*this, v))

#undef SFML_TST_MSGB_DEF_PAIR
// NOLINTEND(bugprone-macro-parentheses)


////////////////////////////////////////////////////////////
// Operator-comma overloads for the `MESSAGE("a", b)` form.
////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses, misc-unconventional-assign-operator)
#define SFML_TST_MSGB_COMMA_DEF(T, body)                          \
    MessageBuilder&& operator,(MessageBuilder&& mb, T v) noexcept \
    {                                                             \
        body;                                                     \
        return static_cast<MessageBuilder&&>(mb);                 \
    }

SFML_TST_MSGB_COMMA_DEF(const char*, appendCString(mb, v))
SFML_TST_MSGB_COMMA_DEF(sf::base::StringView, appendStringView(mb, v))
SFML_TST_MSGB_COMMA_DEF(bool, appendBool(mb, v))
SFML_TST_MSGB_COMMA_DEF(char, appendChar(mb, v))
SFML_TST_MSGB_COMMA_DEF(short, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(unsigned short, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(int, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(unsigned int, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(long, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(unsigned long, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(long long, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(unsigned long long, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(float, appendNumeric(mb, v))
SFML_TST_MSGB_COMMA_DEF(double, appendNumeric(mb, v))

#undef SFML_TST_MSGB_COMMA_DEF
// NOLINTEND(bugprone-macro-parentheses, misc-unconventional-assign-operator)


////////////////////////////////////////////////////////////
void emitInfoPushRaw(const char* /*file*/, int /*line*/, const char* data, sf::base::SizeT size) noexcept
{
    contextState().infoStack.emplaceBack(data, size);
}


////////////////////////////////////////////////////////////
void emitInfoPop() noexcept
{
    auto& stk = contextState().infoStack;
    if (!stk.empty())
        stk.popBack();
}


////////////////////////////////////////////////////////////
void emitMessageRaw(AssertKind kind, const char* file, int line, const char* data, sf::base::SizeT size)
{
    auto& ctx = contextState();

    const sf::base::StringView text{data, size};

    switch (kind)
    {
        case AssertKind::Warn:
            (void)sf::base::printErrLn("{}:{}: MESSAGE: {}", file, line, text);
            break;
        case AssertKind::Check:
            ++ctx.totalAssertions;
            ++ctx.failedAssertions;
            ctx.currentTestFailed = true;
            (void)sf::base::printErrLn("{}:{}: FAIL_CHECK: {}", file, line, text);
            break;
        case AssertKind::Require:
            ++ctx.totalAssertions;
            ++ctx.failedAssertions;
            ctx.currentTestFailed = true;
            (void)sf::base::printErrLn("{}:{}: FAIL: {}", file, line, text);
            throw ContextState::RequireFailedException{};
        default:
            (void)sf::base::printErrLn("{}:{}: {}", file, line, text);
            break;
    }
}


////////////////////////////////////////////////////////////
bool checkNothrowImpl(const char* file, int line, const char* exprStr) noexcept
{
    auto& ctx = contextState();
    ++ctx.totalAssertions;
    ++ctx.failedAssertions;
    ctx.currentTestFailed = true;

    (void)sf::base::printErrLn("{}:{}: FAILED: CHECK_NOTHROW({}) -- exception thrown", file, line, exprStr);
    return false;
}


////////////////////////////////////////////////////////////
void noteNothrowSuccess() noexcept
{
    ++contextState().totalAssertions;
}

} // namespace tst::detail
