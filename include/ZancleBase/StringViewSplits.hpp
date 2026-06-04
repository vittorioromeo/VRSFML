#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Trait/IsSame.hpp"


namespace zb
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

        if constexpr (ZB_IS_SAME(Splitter, char))
            segStart = splitPos + 1u;
        else
            segStart = splitPos + splitter.theSize;
    }
}


////////////////////////////////////////////////////////////
template <typename F>
[[gnu::always_inline, gnu::flatten]] constexpr void StringView::forSplits(const StringView splitter, F&& f) const
{
    ZB_ASSERT(!splitter.empty() && "Splitter must be non-empty");
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

} // namespace zb
