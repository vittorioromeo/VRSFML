#pragma once

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/IndexSequence.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/MakeIndexSequence.hpp"
#include "ZancleBase/MiniPFR.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Trait/IsMemberPointer.hpp"
#include "ZancleBase/TypePackElement.hpp"
#include "ZancleBase/Vector.hpp"


namespace priv
{
////////////////////////////////////////////////////////////
template <zb::SizeT I, typename T>
struct SoABase
{
    zb::Vector<T> data;
};

} // namespace priv

////////////////////////////////////////////////////////////
template <typename, typename, typename...>
class SoA;

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOA_AS_BASE(I)       static_cast<priv::SoABase<I, ZB_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_AS_CONST_BASE(I) static_cast<const priv::SoABase<I, ZB_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_ALL_BASES()      static_cast<priv::SoABase<Is, Ts>&>(*this)

////////////////////////////////////////////////////////////
template <typename T, zb::SizeT... Is, typename... Ts>
class SoA<T, zb::IndexSequence<Is...>, Ts...> : private priv::SoABase<Is, Ts>...
{
private:
    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[nodiscard]] static consteval zb::SizeT fieldIndexFromPM()
    {
        // const auto& obj = minipfr::priv::getFakeObject<T>();
        constexpr T obj{};

        auto result = static_cast<zb::SizeT>(-1);

        (...,
         ((static_cast<const void*>(&(zb::minipfr::getField<Is>(obj))) == static_cast<const void*>(&(obj.*PM)))
              ? (result = Is)
              : 0));

        return result;
    }

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
    [[gnu::always_inline]] void pushBack(const T& aos)
    {
        emplaceBack(zb::minipfr::getField<Is>(aos)...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void emplaceBack(auto&&... values)
    {
        (..., SOA_ALL_BASES().data.emplaceBack(ZB_FORWARD(values)));
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
    template <auto PM>
    [[nodiscard, gnu::always_inline]] auto& get() noexcept
        requires zb::isMemberPointer<decltype(PM)>
    {
        return get<fieldIndexFromPM<PM>()>();
    }

    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[nodiscard, gnu::always_inline]] const auto& get() const noexcept
        requires zb::isMemberPointer<decltype(PM)>
    {
        return get<fieldIndexFromPM<PM>()>();
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
        ZB_ASSERT(end <= getSize());
        ZB_ASSERT(start <= end);

        for (zb::SizeT i = start; i < end; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllSubRange(const zb::SizeT start, const zb::SizeT end, auto&& f)
    {
        ZB_ASSERT(end <= getSize());
        ZB_ASSERT(start <= end);

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
    template <auto... PMs>
    [[gnu::always_inline]] void with(auto&& f)
        requires(... && zb::isMemberPointer<decltype(PMs)>)
    {
        with<fieldIndexFromPM<PMs>()...>(ZB_FORWARD(f));
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

    ////////////////////////////////////////////////////////////
    template <auto... PMs>
    [[gnu::always_inline]] void eraseIfBySwapping(auto&& f)
        requires(... && zb::isMemberPointer<decltype(PMs)>)
    {
        eraseIfBySwapping<fieldIndexFromPM<PMs>()...>(ZB_FORWARD(f));
    }
};

////////////////////////////////////////////////////////////
#undef SOA_ALL_BASES
#undef SOA_AS_CONST_BASE
#undef SOA_AS_BASE

////////////////////////////////////////////////////////////
template <typename, typename T>
class SoAHelper;

////////////////////////////////////////////////////////////
template <zb::SizeT... Is, typename T>
class SoAHelper<zb::IndexSequence<Is...>, T>
{
public:
    using Type = SoA<T, zb::IndexSequence<Is...>, zb::minipfr::FieldType<Is, T>...>;
};

////////////////////////////////////////////////////////////
template <typename T>
using SoAFor = typename SoAHelper<ZB_MAKE_INDEX_SEQUENCE(zb::minipfr::numFields<T>), T>::Type;
