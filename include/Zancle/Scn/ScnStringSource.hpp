#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/String/StringView.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/SizeT.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \brief `ScnSource` (and `ContiguousScnSource`) over a `StringView`.
///
/// Owns no characters -- the underlying buffer must outlive the
/// source. Cheap to construct and copy. Suitable as a drop-in
/// replacement for `std::istringstream` in parsing-only contexts.
///
/// Modeled directly on `StringView`: an internal cursor `m_pos` slices
/// the original view into a consumed prefix and an unconsumed suffix.
/// The contiguous fast path simply hands the suffix out via `remaining()`.
///
////////////////////////////////////////////////////////////
class ScnStringSource
{
public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr explicit ScnStringSource(const StringView view) noexcept : m_view{view}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Optional<char> peek() const noexcept
    {
        if (m_pos >= m_view.size())
            return nullOpt;

        return makeOptional(m_view.data()[m_pos]);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void consume() noexcept
    {
        ZA_ASSERT(m_pos < m_view.size());
        ++m_pos;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr StringView remaining() const noexcept
    {
        return m_view.substrByPosLen(m_pos);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void advance(const SizeT n) noexcept
    {
        // Non-overflowing form: `m_pos + n <= m_view.size()` can wrap
        // when `n` is huge. The invariant `m_view.size() >= m_pos`
        // makes `m_view.size() - m_pos` safe.
        ZA_ASSERT(n <= m_view.size() - m_pos);
        m_pos += n;
    }


    ////////////////////////////////////////////////////////////
    /// \brief How many bytes have been consumed since construction.
    /// Useful for "resume from cursor" patterns.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr SizeT bytesConsumed() const noexcept
    {
        return m_pos;
    }

private:
    StringView m_view;
    SizeT      m_pos = 0u;
};

} // namespace za


////////////////////////////////////////////////////////////
/// \file
/// In-memory `ScnSource` backed by a `StringView`. Satisfies the opt-in
/// `ContiguousScnSource` concept, so built-in parsers take their bulk
/// fast paths against it.
///
////////////////////////////////////////////////////////////
