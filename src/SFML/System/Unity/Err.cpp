// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"

#include "SFML/System/AtomicMutex.hpp"
#include "SFML/System/LockGuard.hpp"

#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StackTrace.hpp"
#include "SFML/Base/String.hpp"

#include <cstdio>


namespace sf::priv
{
namespace
{
////////////////////////////////////////////////////////////
constexpr const char  errPrefix[]   = "[[SFML ERROR]]: ";
constexpr base::SizeT errPrefixSize = sizeof(errPrefix) - 1u; // exclude trailing '\0'


////////////////////////////////////////////////////////////
void defaultErrSink(void* /*ctx*/, const char* const data, const base::SizeT size)
{
    std::fwrite(data, 1u, size, stderr);
}


////////////////////////////////////////////////////////////
[[nodiscard]] AtomicMutex& sinkMutex()
{
    static AtomicMutex m;
    return m;
}


////////////////////////////////////////////////////////////
ErrSinkFn currentSinkFn  = &defaultErrSink;
void*     currentSinkCtx = nullptr;

} // namespace


////////////////////////////////////////////////////////////
void setErrSink(const ErrSinkFn fn, void* const ctx)
{
    const LockGuard lock(sinkMutex());
    currentSinkFn  = fn != nullptr ? fn : &defaultErrSink;
    currentSinkCtx = ctx;
}


////////////////////////////////////////////////////////////
void emitErr(const char* const data, const base::SizeT size, const bool trailing)
{
    // Assemble prefix + content + (optional first '\n') into a single buffer so
    // the sink sees one write covering the whole logical message.
    base::String msg;
    msg.reserve(errPrefixSize + size + 2u);
    msg.append(errPrefix, errPrefixSize);
    msg.append(data, size);

    if (trailing)
        msg += '\n';

    const LockGuard lock(sinkMutex());

    currentSinkFn(currentSinkCtx, msg.data(), msg.size());

    if (trailing)
    {
#ifdef SFML_ENABLE_STACK_TRACES
        // Stack-trace output goes directly to `stderr` regardless of the
        // installed sink -- matches historical behavior and avoids re-plumbing
        // libbacktrace through a callback.
        base::priv::printStackTrace();
#endif

        const char nl = '\n';
        currentSinkFn(currentSinkCtx, &nl, 1u);
    }

    // Always flush stderr so partial multi-line error messages (built with
    // `errMsgMulti`) remain visible even if a following emission never
    // arrives -- e.g., the process crashes after a multi-line preamble.
    // Cheap: when stderr is a TTY it's typically unbuffered already, so
    // `fflush` is a fast path.
    if (currentSinkFn == &defaultErrSink)
        std::fflush(stderr);
}


////////////////////////////////////////////////////////////
ErrMsgScope::ErrMsgScope() = default;


////////////////////////////////////////////////////////////
ErrMsgScope::~ErrMsgScope()
{
    emitErr(m_buf->data(), m_buf->size(), m_trailing);
}


////////////////////////////////////////////////////////////
void ErrMsgScope::append(const char* const text)
{
    m_buf->append(text, SFML_BASE_STRLEN(text));
}


////////////////////////////////////////////////////////////
void ErrMsgScope::append(const char* const data, const base::SizeT size)
{
    m_buf->append(data, size);
}


////////////////////////////////////////////////////////////
void ErrMsgScope::disableTrailing() noexcept
{
    m_trailing = false;
}

} // namespace sf::priv
