#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/System/Fmt/FmtPath.hpp" // IWYU pragma: keep -- makes fmtArg(Path) visible at errMsg call sites

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: export -- errMsg call sites typically format numeric arguments
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::base
{
class String;
} // namespace sf::base


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Callback installed via `setErrSink` to redirect error output.
///
/// Receives the fully-assembled error text (already prefixed with
/// "[[SFML ERROR]]: " for the first chunk of an emission). May be
/// invoked once or twice per error -- once for the message body, once
/// for the trailing newline that follows an optional stack trace.
////////////////////////////////////////////////////////////
using ErrSinkFn = void (*)(void* ctx, const char* data, base::SizeT size);


////////////////////////////////////////////////////////////
/// \brief Install a custom error sink. `nullptr` restores the default
/// (writes to `stderr`). Used by Android (logcat) and tests.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void setErrSink(ErrSinkFn fn, void* ctx);


////////////////////////////////////////////////////////////
/// \brief Out-of-line emission helper. Called by `errMsg` / `~ErrMsgScope`.
///
/// Acquires the err mutex, prepends the "[[SFML ERROR]]: " prefix, and
/// writes through the installed sink. When `trailing == true` it also
/// emits a stack trace + trailing newline. Stderr is always flushed at
/// the end of the call so partial messages remain visible if the process
/// crashes before the next emission.
////////////////////////////////////////////////////////////
SFML_SYSTEM_API void emitErr(const char* data, base::SizeT size, bool trailing);


////////////////////////////////////////////////////////////
/// \brief Scoped error-message builder.
///
/// Composes a single error message across multiple `format` / `append`
/// calls (useful for loops, conditional content, multi-line diagnostics).
/// On destruction emits the accumulated content atomically: one prefix,
/// one optional trailing newline + stack trace, one flush.
///
/// Buffering avoids holding the err lock across user formatting work --
/// the lock is taken only once, during the final emission.
///
/// `base::String` is intentionally not exposed in this header. The
/// member buffer is stored opaquely via `InPlacePImpl`; format and
/// append operations route through `fmtImpl` / `append`, both
/// defined in `Err.cpp` where the full `String` type is available.
////////////////////////////////////////////////////////////
class SFML_SYSTEM_API ErrMsgScope
{
public:
    ErrMsgScope();
    ~ErrMsgScope();

    ErrMsgScope(const ErrMsgScope&)            = delete;
    ErrMsgScope& operator=(const ErrMsgScope&) = delete;

    ErrMsgScope(ErrMsgScope&&)            = delete;
    ErrMsgScope& operator=(ErrMsgScope&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Append a null-terminated string to the buffered message.
    /// For raw byte spans, use the two-argument overload.
    ////////////////////////////////////////////////////////////
    void append(const char* text);

    ////////////////////////////////////////////////////////////
    /// \brief Append `size` bytes from `data` to the buffered message.
    ////////////////////////////////////////////////////////////
    void append(const char* data, base::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief Append formatted content to the buffered message.
    ///
    /// Routes through `base::fmtTo`, which uses `*this` as an `AppendSink`
    /// (via the two-argument `append` overload above). Stack-scratched on
    /// the fast path, heap-fallback on overflow -- both end in a single
    /// `append(scratch, size)` call to this scope.
    ////////////////////////////////////////////////////////////
    template <typename... Args>
    void fmt(typename base::NonDeduced<const base::FmtString<Args...>>::type fmtStr, const Args&... args)
    {
        (void)base::fmtTo(*this, fmtStr, args...);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Suppress the trailing newline + stack trace on destruction.
    /// Use for multi-line messages that the caller has formatted to end
    /// in their own newline, or when no stack trace is wanted.
    ////////////////////////////////////////////////////////////
    void disableTrailing() noexcept;

private:
    // Buffer-sized for the layout of `base::String` (3 pointers/sizes on
    // x86_64 = 24 bytes; rounded up for headroom and 32-bit safety).
    base::InPlacePImpl<base::String, 32> m_buf;
    bool                                 m_trailing{true};
};


////////////////////////////////////////////////////////////
/// \brief Atomic single-emission error. Equivalent to a scoped
/// `ErrMsgScope` that lives just long enough to format `fmt` / `args`.
///
/// Output shape: `"[[SFML ERROR]]: " + content + "\n" + stack_trace + "\n"`.
////////////////////////////////////////////////////////////
template <typename... Args>
void errMsg(typename base::NonDeduced<const base::FmtString<Args...>>::type fmtStr, const Args&... args)
{
    ErrMsgScope scope;
    scope.fmt(fmtStr, args...);
}


////////////////////////////////////////////////////////////
/// \brief Atomic single-emission error with no trailing newline / stack
/// trace. Use when the caller's format string already supplies them or
/// when emitting a logical line of a larger composed message.
////////////////////////////////////////////////////////////
template <typename... Args>
void errMsgMulti(typename base::NonDeduced<const base::FmtString<Args...>>::type fmtStr, const Args&... args)
{
    ErrMsgScope scope;
    scope.disableTrailing();
    scope.fmt(fmtStr, args...);
}

} // namespace sf::priv
