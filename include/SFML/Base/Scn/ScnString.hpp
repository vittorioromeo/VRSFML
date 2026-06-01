#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Scn/ScnCore.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `scnArg` for `base::String`: skips leading whitespace, then
/// reads the next whitespace-delimited token into `out` (overwriting
/// any prior contents).
///
/// Returns `false` only if the source has nothing but whitespace left
/// (i.e. would produce an empty token). Otherwise `out` holds the token
/// and the source is positioned at the first trailing whitespace byte
/// (or EOF).
///
/// On a `ContiguousScnSource` the implementation finds the run end via
/// a tight loop over `remaining()` and appends in one `append(data, n)`
/// call. On a non-contiguous source it falls back to per-byte
/// `peek`/`consume`.
///
////////////////////////////////////////////////////////////
template <ScnSource S>
[[nodiscard]] bool scnArg(S& src, String& out)
{
    scnSkipWhitespace(src);

    out.clear();

    if constexpr (ContiguousScnSource<S>)
    {
        const StringView rem = src.remaining();
        SizeT            k   = 0u;

        while (k < rem.size() && !scnIsWhitespace(rem.data()[k]))
            ++k;

        if (k == 0u)
            return false;

        out.append(rem.data(), k);
        src.advance(k);

        return true;
    }
    else
    {
        bool gotAny = false;

        while (auto c = src.peek())
        {
            if (scnIsWhitespace(*c))
                break;

            out.append(*c);
            src.consume();

            gotAny = true;
        }

        return gotAny;
    }
}


////////////////////////////////////////////////////////////
/// \brief Read up to (and consume) the next `'\n'` into `out`,
/// **without** including the `'\n'` itself. A trailing `'\r'` from
/// CRLF endings stays in `out`; callers strip it if they need to.
///
/// Returns `false` only when the source is at EOF on entry. An empty
/// line (just `'\n'`) returns `true` with `out` empty.
///
////////////////////////////////////////////////////////////
template <ScnSource S>
[[nodiscard]] bool scnReadLine(S& src, String& out)
{
    if (scnAtEnd(src))
        return false;

    out.clear();

    if constexpr (ContiguousScnSource<S>)
    {
        const StringView rem = src.remaining();
        SizeT            k   = 0u;

        while (k < rem.size() && rem.data()[k] != '\n')
            ++k;

        if (k != 0u)
            out.append(rem.data(), k);

        // Skip the bytes we copied, plus the '\n' if we hit one.
        src.advance(k < rem.size() ? k + 1u : k);
    }
    else
    {
        while (auto c = src.peek())
        {
            src.consume();

            if (*c == '\n')
                return true;

            out.append(*c);
        }
    }

    return true;
}

} // namespace sf::base


////////////////////////////////////////////////////////////
/// \file
/// Built-in `scnArg` for `base::String` (skip-whitespace + read-token),
/// plus the `scnReadLine` helper.
///
////////////////////////////////////////////////////////////
