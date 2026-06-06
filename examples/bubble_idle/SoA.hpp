#pragma once

#include "Zancle/Container/Vector.hpp"

#include "Zancle/Base/IndexSequence.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Base/MakeIndexSequence.hpp"
#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Base/TypePackElement.hpp"


////////////////////////////////////////////////////////////
template <za::SizeT I, typename T>
struct SoABase
{
    ////////////////////////////////////////////////////////////
    enum : za::SizeT
    {
        index = I
    };

    ////////////////////////////////////////////////////////////
    [[no_unique_address]] za::Vector<T> data;
};

////////////////////////////////////////////////////////////
template <typename, typename...>
class SoA;

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOA_AS_BASE(I)       static_cast<SoABase<I, ZA_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_AS_CONST_BASE(I) static_cast<const SoABase<I, ZA_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_ALL_BASES()      static_cast<SoABase<Is, Ts>&>(*this)

////////////////////////////////////////////////////////////
template <za::SizeT... Is, typename... Ts>
class SoA<za::IndexSequence<Is...>, Ts...> : private SoABase<Is, Ts>...
{
public:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        (..., SOA_ALL_BASES().data.clear());
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserve(const za::SizeT capacity)
    {
        (..., SOA_ALL_BASES().data.reserve(capacity));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const za::SizeT size)
    {
        (..., SOA_ALL_BASES().data.resize(size));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void pushBack(auto&&... values)
    {
        (..., SOA_ALL_BASES().data.pushBack(ZA_FORWARD(values)));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] za::SizeT getSize() const
    {
        return SOA_AS_CONST_BASE(0).data.size();
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT I>
    [[nodiscard, gnu::always_inline]] auto& get() noexcept
    {
        return SOA_AS_BASE(I).data;
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT I>
    [[nodiscard, gnu::always_inline]] const auto& get() const noexcept
    {
        return SOA_AS_CONST_BASE(I).data;
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT... Js>
    [[gnu::always_inline]] void withNth(const za::SizeT i, auto&& f)
    {
        f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllNth(const za::SizeT i, auto&& f)
    {
        f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT... Js>
    [[gnu::always_inline]] void withSubRange(const za::SizeT start, const za::SizeT end, auto&& f)
    {
        for (za::SizeT i = start; i < end; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllSubRange(const za::SizeT start, const za::SizeT end, auto&& f)
    {
        for (za::SizeT i = start; i < end; ++i)
            f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT... Js>
    [[gnu::always_inline]] void with(auto&& f)
    {
        const za::SizeT size = getSize();

        for (za::SizeT i = 0u; i < size; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAll(auto&& f)
    {
        const za::SizeT size = getSize();

        for (za::SizeT i = 0u; i < size; ++i)
            f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT... Js>
    void eraseIfByShifting(auto&& f)
    {
        const za::SizeT n = getSize();

        // Find the first element to remove.
        za::SizeT i = 0u;
        while (i < n && !f(SOA_AS_BASE(Js).data[i]...))
            ++i;

        // For the remaining elements, shift over those that must be kept.
        za::SizeT newSize = i;

        for (; i < n; ++i)
        {
            if (f(SOA_AS_BASE(Js).data[i]...))
                continue;

            if (newSize != i)
                (..., (SOA_ALL_BASES().data[newSize] = ZA_MOVE(SOA_ALL_BASES().data[i])));

            ++newSize;
        }

        // Resize all columns to the new size.
        (..., SOA_ALL_BASES().data.resize(newSize));
    }

    ////////////////////////////////////////////////////////////
    template <za::SizeT... Js>
    void eraseIfBySwapping(auto&& f)
    {
        za::SizeT currentSize = getSize();

        for (za::SizeT i = currentSize; i-- > 0u;)
        {
            if (!f(SOA_AS_BASE(Js).data[i]...))
                continue;

            --currentSize;
            (..., (SOA_ALL_BASES().data[i] = ZA_MOVE(SOA_ALL_BASES().data[currentSize])));
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
using SoAFor = SoA<ZA_INDEX_SEQUENCE_FOR(Ts), Ts...>;
