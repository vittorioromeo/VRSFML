// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/Err.hpp"

#include "Zancle/System/AtomicMutex.hpp"
#include "Zancle/System/LockGuard.hpp"

#include "ZancleBase/Builtin/Strlen.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StackTrace.hpp"
#include "ZancleBase/String.hpp"

#include <cstdio>


namespace za::priv
{
namespace
{
////////////////////////////////////////////////////////////
constexpr const char  errPrefix[]   = "[[ZANCLE ERROR]]: ";
constexpr zb::SizeT errPrefixSize = sizeof(errPrefix) - 1u; // exclude trailing '\0'


////////////////////////////////////////////////////////////
void defaultErrSink(void* /*ctx*/, const char* const data, const zb::SizeT size)
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
void emitErr(const char* const data, const zb::SizeT size, const bool trailing)
{
    // Assemble prefix + content + (optional first '\n') into a single buffer so
    // the sink sees one write covering the whole logical message.
    zb::String msg;
    msg.reserve(errPrefixSize + size + 2u);
    msg.append(errPrefix, errPrefixSize);
    msg.append(data, size);

    if (trailing)
        msg += '\n';

    const LockGuard lock(sinkMutex());

    currentSinkFn(currentSinkCtx, msg.data(), msg.size());

    if (trailing)
    {
#ifdef ZA_ENABLE_STACK_TRACES
        // Stack-trace output goes directly to `stderr` regardless of the
        // installed sink -- matches historical behavior and avoids re-plumbing
        // libbacktrace through a callback.
        zb::priv::printStackTrace();
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
    m_buf->append(text, ZB_STRLEN(text));
}


////////////////////////////////////////////////////////////
void ErrMsgScope::append(const char* const data, const zb::SizeT size)
{
    m_buf->append(data, size);
}


////////////////////////////////////////////////////////////
void ErrMsgScope::disableTrailing() noexcept
{
    m_trailing = false;
}

} // namespace za::priv
