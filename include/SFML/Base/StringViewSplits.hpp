#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename Splitter, typename F>
[[gnu::always_inline]] constexpr void StringView::forSplitsImpl(Splitter splitter, F&& f) const
{
    SizeT segStart = 0u;

    while (segStart < theSize)
    {
        const SizeT splitPos = find(splitter, segStart);

        if (splitPos == nPos)
        {
            f(StringView{theData + segStart, theSize - segStart});
            return;
        }

        f(StringView{theData + segStart, splitPos - segStart});

        if constexpr (SFML_BASE_IS_SAME(Splitter, char))
            segStart = splitPos + 1u;
        else
            segStart = splitPos + splitter.theSize;
    }
}


////////////////////////////////////////////////////////////
template <typename F>
[[gnu::always_inline, gnu::flatten]] constexpr void StringView::forSplits(const StringView splitter, F&& f) const
{
    SFML_BASE_ASSERT(!splitter.empty() && "Splitter must be non-empty");
    forSplitsImpl(splitter, static_cast<F&&>(f));
}


////////////////////////////////////////////////////////////
template <typename F>
[[gnu::always_inline, gnu::flatten]] constexpr void StringView::forSplits(const char splitter, F&& f) const
{
    forSplitsImpl(splitter, static_cast<F&&>(f));
}


////////////////////////////////////////////////////////////
template <typename F>
[[gnu::always_inline, gnu::flatten]] constexpr void StringView::forLines(F&& f) const
{
    forSplits('\n', static_cast<F&&>(f));
}

} // namespace sf::base
