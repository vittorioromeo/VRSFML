#pragma once

#include "ZancleBase/IndexSequence.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/MakeIndexSequence.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/TypePackElement.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
template <zb::SizeT I, typename T>
struct SoABase
{
    ////////////////////////////////////////////////////////////
    enum : zb::SizeT
    {
        index = I
    };

    ////////////////////////////////////////////////////////////
    [[no_unique_address]] zb::Vector<T> data;
};

////////////////////////////////////////////////////////////
template <typename, typename...>
class SoA;

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOA_AS_BASE(I)       static_cast<SoABase<I, ZB_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_AS_CONST_BASE(I) static_cast<const SoABase<I, ZB_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_ALL_BASES()      static_cast<SoABase<Is, Ts>&>(*this)

////////////////////////////////////////////////////////////
template <zb::SizeT... Is, typename... Ts>
class SoA<zb::IndexSequence<Is...>, Ts...> : private SoABase<Is, Ts>...
{
public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        (..., SOA_ALL_BASES().data.clear());
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserve(const zb::SizeT capacity)
    {
        (..., SOA_ALL_BASES().data.reserve(capacity));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const zb::SizeT size)
    {
        (..., SOA_ALL_BASES().data.resize(size));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void pushBack(auto&&... values)
    {
        (..., SOA_ALL_BASES().data.pushBack(ZB_FORWARD(values)));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] zb::SizeT getSize() const
    {
        return SOA_AS_CONST_BASE(0).data.size();
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT I>
    [[nodiscard, gnu::always_inline]] auto& get() noexcept
    {
        return SOA_AS_BASE(I).data;
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT I>
    [[nodiscard, gnu::always_inline]] const auto& get() const noexcept
    {
        return SOA_AS_CONST_BASE(I).data;
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT... Js>
    [[gnu::always_inline]] void withNth(const zb::SizeT i, auto&& f)
    {
        f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllNth(const zb::SizeT i, auto&& f)
    {
        f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT... Js>
    [[gnu::always_inline]] void withSubRange(const zb::SizeT start, const zb::SizeT end, auto&& f)
    {
        for (zb::SizeT i = start; i < end; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllSubRange(const zb::SizeT start, const zb::SizeT end, auto&& f)
    {
        for (zb::SizeT i = start; i < end; ++i)
            f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT... Js>
    [[gnu::always_inline]] void with(auto&& f)
    {
        const zb::SizeT size = getSize();

        for (zb::SizeT i = 0u; i < size; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAll(auto&& f)
    {
        const zb::SizeT size = getSize();

        for (zb::SizeT i = 0u; i < size; ++i)
            f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT... Js>
    void eraseIfByShifting(auto&& f)
    {
        const zb::SizeT n = getSize();

        // Find the first element to remove.
        zb::SizeT i = 0u;
        while (i < n && !f(SOA_AS_BASE(Js).data[i]...))
            ++i;

        // For the remaining elements, shift over those that must be kept.
        zb::SizeT newSize = i;

        for (; i < n; ++i)
        {
            if (f(SOA_AS_BASE(Js).data[i]...))
                continue;

            if (newSize != i)
                (..., (SOA_ALL_BASES().data[newSize] = ZB_MOVE(SOA_ALL_BASES().data[i])));

            ++newSize;
        }

        // Resize all columns to the new size.
        (..., SOA_ALL_BASES().data.resize(newSize));
    }

    ////////////////////////////////////////////////////////////
    template <zb::SizeT... Js>
    void eraseIfBySwapping(auto&& f)
    {
        zb::SizeT currentSize = getSize();

        for (zb::SizeT i = currentSize; i-- > 0u;)
        {
            if (!f(SOA_AS_BASE(Js).data[i]...))
                continue;

            --currentSize;
            (..., (SOA_ALL_BASES().data[i] = ZB_MOVE(SOA_ALL_BASES().data[currentSize])));
        }

        (..., SOA_ALL_BASES().data.resize(currentSize));
    }
};

////////////////////////////////////////////////////////////
#undef SOA_ALL_BASES
#undef SOA_AS_CONST_BASE
#undef SOA_AS_BASE

////////////////////////////////////////////////////////////
template <typename... Ts>
using SoAFor = SoA<ZB_INDEX_SEQUENCE_FOR(Ts), Ts...>;
