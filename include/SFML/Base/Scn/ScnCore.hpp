#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Default scratch buffer size for integer parsers. Comfortably
/// covers any signed/unsigned 64-bit value in any radix.
///
////////////////////////////////////////////////////////////
inline constexpr SizeT scnNumericScratchSize = 64u;


////////////////////////////////////////////////////////////
/// \brief Scratch buffer size for the float parser.
///
/// Deliberately smaller than `scnNumericScratchSize`: 40 bytes is more
/// than IEEE 754 double's 17 significant decimal digits plus sign,
/// decimal point, and slack. Any token longer than this is policy-
/// rejected (consumed, then the parse fails) -- prevents pathological
/// inputs from forcing a huge stack scratch.
///
////////////////////////////////////////////////////////////
inline constexpr SizeT scnFloatScratchSize = 40u;


////////////////////////////////////////////////////////////
/// \brief Minimal byte-source concept consumed by `scnArg` overloads.
///
/// A source exposes two primitives:
///
///   - `peek()`: returns the next byte without consuming it, or an empty
///     value at EOF. Must be both contextually convertible to `bool` and
///     dereferenceable to a `char`-assignable value (`Optional<char>`
///     fits, and is what every built-in source returns).
///
///   - `consume()`: advances past the byte most recently returned by
///     `peek()`. Calling `consume()` without a preceding successful
///     `peek()` is undefined.
///
/// This is the slow-but-universal path. Sources that can additionally
/// hand out a contiguous view of remaining bytes should also model
/// `ContiguousScnSource` -- that lets built-in parsers take a byte-bulk
/// fast path. `ScnStringSource` is the canonical example; `InFile`
/// satisfies `ScnSource` but not the contiguous extension (no in-memory
/// buffer is exposed).
///
////////////////////////////////////////////////////////////
template <typename T>
concept ScnSource = requires(T& src) {
    { static_cast<bool>(src.peek()) };
    { char{*src.peek()} };
    src.consume();
};


////////////////////////////////////////////////////////////
/// \brief Opt-in extension of `ScnSource`: source exposes a contiguous
/// view of the remaining bytes plus a bulk `advance` primitive.
///
/// Built-in parsers branch on this via `if constexpr` -- on a contiguous
/// source they scan + bulk-append in a single shot; on a non-contiguous
/// source they fall back to per-byte `peek`/`consume`.
///
////////////////////////////////////////////////////////////
template <typename T>
concept ContiguousScnSource = ScnSource<T> && requires(T& src, SizeT n) {
    { src.remaining().data() };
    { src.remaining().size() };
    src.advance(n);
};


////////////////////////////////////////////////////////////
/// \brief Whitespace test matching `std::isspace` / `iostream::operator>>`.
///
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool scnIsWhitespace(const char c) noexcept
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}


////////////////////////////////////////////////////////////
/// \brief Drop leading whitespace from `src`. Equivalent in effect to
/// `iostream::operator>>` skipping rules.
////////////////////////////////////////////////////////////
template <ScnSource S>
constexpr void scnSkipWhitespace(S& src)
{
    if constexpr (ContiguousScnSource<S>)
    {
        const StringView rem = src.remaining();
        SizeT            k   = 0u;

        while (k < rem.size() && scnIsWhitespace(rem.data()[k]))
            ++k;

        if (k != 0u)
            src.advance(k);
    }
    else
    {
        while (auto c = src.peek())
        {
            if (!scnIsWhitespace(*c))
                break;

            src.consume();
        }
    }
}


////////////////////////////////////////////////////////////
/// \brief Consume bytes until `delim` is seen (and consumed), or EOF.
/// Mirrors `std::istream::ignore`'s default-count semantics.
////////////////////////////////////////////////////////////
template <ScnSource S>
constexpr void scnSkipPast(S& src, const char delim)
{
    if constexpr (ContiguousScnSource<S>)
    {
        const StringView rem = src.remaining();
        SizeT            k   = 0u;

        while (k < rem.size() && rem.data()[k] != delim)
            ++k;

        // Either we found the delimiter (skip past it) or hit EOF (skip the rest).
        src.advance(k < rem.size() ? k + 1u : k);
    }
    else
    {
        while (auto c = src.peek())
        {
            src.consume();

            if (*c == delim)
                return;
        }
    }
}


////////////////////////////////////////////////////////////
/// \brief `true` once `src.peek()` reports end-of-input.
///
////////////////////////////////////////////////////////////
[[nodiscard]] constexpr bool scnAtEnd(ScnSource auto& src)
{
    return !static_cast<bool>(src.peek());
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Core of the `Scn` parsing component:
///
///   - `ScnSource`           -- minimal byte-source concept (peek + consume)
///   - `ContiguousScnSource` -- opt-in fast-path concept (remaining + advance)
///   - `scnIsWhitespace` / `scnSkipWhitespace` / `scnSkipPast` / `scnAtEnd`
///
/// The high-level entry points `scn<T>` / `scnInto` live in
/// `<SFML/Base/Scn/Scn.hpp>`, which also pulls in the primitive body
/// headers needed for built-in targets.
///
/// User types extend the system via an ADL `scnArg(src, out) -> bool`
/// overload, parallel to `fmtArg`. Built-in `scnArg` overloads live in
/// dedicated headers:
///   - `<SFML/Base/Scn/ScnChar.hpp>`       -- `char`
///   - `<SFML/Base/Scn/ScnString.hpp>`     -- `base::String` + `scnReadLine`
///   - `<SFML/Base/Scn/ScnNumeric.hpp>`    -- int / float / bool + `scnRadix`
///
/// Whitespace policy follows scnlib:
///   - `char` does NOT skip leading whitespace.
///   - everything else (int / float / bool / string-likes) skips first.
///
////////////////////////////////////////////////////////////
