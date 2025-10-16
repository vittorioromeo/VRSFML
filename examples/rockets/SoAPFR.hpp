#pragma once

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/MiniPFR.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsMemberPointer.hpp"
#include "SFML/Base/TypePackElement.hpp"
#include "SFML/Base/Vector.hpp"


namespace priv
{
////////////////////////////////////////////////////////////
template <sf::base::SizeT I, typename T>
struct SoABase
{
    sf::base::Vector<T> data;
};

} // namespace priv

////////////////////////////////////////////////////////////
template <typename, typename, typename...>
class SoA;

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOA_AS_BASE(I)       static_cast<priv::SoABase<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_AS_CONST_BASE(I) static_cast<const priv::SoABase<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_ALL_BASES()      static_cast<priv::SoABase<Is, Ts>&>(*this)

////////////////////////////////////////////////////////////
template <typename T, sf::base::SizeT... Is, typename... Ts>
class SoA<T, sf::base::IndexSequence<Is...>, Ts...> : private priv::SoABase<Is, Ts>...
{
private:
    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[nodiscard]] static consteval sf::base::SizeT fieldIndexFromPM()
    {
        // const auto& obj = minipfr::priv::getFakeObject<T>();
        constexpr T obj{};

        auto result = static_cast<sf::base::SizeT>(-1);

        (...,
         ((static_cast<const void*>(&(sf::base::minipfr::getField<Is>(obj))) == static_cast<const void*>(&(obj.*PM)))
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
    [[gnu::always_inline]] void reserve(const sf::base::SizeT capacity)
    {
        (..., SOA_ALL_BASES().data.reserve(capacity));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const sf::base::SizeT size)
    {
        (..., SOA_ALL_BASES().data.resize(size));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void pushBack(const T& aos)
    {
        emplaceBack(sf::base::minipfr::getField<Is>(aos)...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void emplaceBack(auto&&... values)
    {
        (..., SOA_ALL_BASES().data.emplaceBack(SFML_BASE_FORWARD(values)));
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] sf::base::SizeT getSize() const
    {
        return SOA_AS_CONST_BASE(0).data.size();
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT I>
    [[nodiscard, gnu::always_inline]] auto& get() noexcept
    {
        return SOA_AS_BASE(I).data;
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT I>
    [[nodiscard, gnu::always_inline]] const auto& get() const noexcept
    {
        return SOA_AS_CONST_BASE(I).data;
    }

    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[nodiscard, gnu::always_inline]] auto& get() noexcept
        requires sf::base::isMemberPointer<decltype(PM)>
    {
        return get<fieldIndexFromPM<PM>()>();
    }

    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[nodiscard, gnu::always_inline]] const auto& get() const noexcept
        requires sf::base::isMemberPointer<decltype(PM)>
    {
        return get<fieldIndexFromPM<PM>()>();
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    [[gnu::always_inline]] void withNth(const sf::base::SizeT i, auto&& f)
    {
        f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllNth(const sf::base::SizeT i, auto&& f)
    {
        f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    [[gnu::always_inline]] void withSubRange(const sf::base::SizeT start, const sf::base::SizeT end, auto&& f)
    {
        SFML_BASE_ASSERT(end <= getSize());
        SFML_BASE_ASSERT(start <= end);

        for (sf::base::SizeT i = start; i < end; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllSubRange(const sf::base::SizeT start, const sf::base::SizeT end, auto&& f)
    {
        SFML_BASE_ASSERT(end <= getSize());
        SFML_BASE_ASSERT(start <= end);

        for (sf::base::SizeT i = start; i < end; ++i)
            f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    [[gnu::always_inline]] void with(auto&& f)
    {
        const sf::base::SizeT size = getSize();

        for (sf::base::SizeT i = 0u; i < size; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <auto... PMs>
    [[gnu::always_inline]] void with(auto&& f)
        requires(... && sf::base::isMemberPointer<decltype(PMs)>)
    {
        with<fieldIndexFromPM<PMs>()...>(SFML_BASE_FORWARD(f));
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAll(auto&& f)
    {
        const sf::base::SizeT size = getSize();

        for (sf::base::SizeT i = 0u; i < size; ++i)
            f(SOA_ALL_BASES().data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    void eraseIfByShifting(auto&& f)
    {
        const sf::base::SizeT n = getSize();

        // Find the first element to remove.
        sf::base::SizeT i = 0u;
        while (i < n && !f(SOA_AS_BASE(Js).data[i]...))
            ++i;

        // For the remaining elements, shift over those that must be kept.
        sf::base::SizeT newSize = i;

        for (; i < n; ++i)
        {
            if (f(SOA_AS_BASE(Js).data[i]...))
                continue;

            if (newSize != i)
                (..., (SOA_ALL_BASES().data[newSize] = SFML_BASE_MOVE(SOA_ALL_BASES().data[i])));

            ++newSize;
        }

        // Resize all columns to the new size.
        (..., SOA_ALL_BASES().data.resize(newSize));
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    void eraseIfBySwapping(auto&& f)
    {
        sf::base::SizeT currentSize = getSize();

        for (sf::base::SizeT i = currentSize; i-- > 0u;)
        {
            if (!f(SOA_AS_BASE(Js).data[i]...))
                continue;

            --currentSize;
            (..., (SOA_ALL_BASES().data[i] = SFML_BASE_MOVE(SOA_ALL_BASES().data[currentSize])));
        }

        (..., SOA_ALL_BASES().data.resize(currentSize));
    }

    ////////////////////////////////////////////////////////////
    template <auto... PMs>
    [[gnu::always_inline]] void eraseIfBySwapping(auto&& f)
        requires(... && sf::base::isMemberPointer<decltype(PMs)>)
    {
        eraseIfBySwapping<fieldIndexFromPM<PMs>()...>(SFML_BASE_FORWARD(f));
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
template <sf::base::SizeT... Is, typename T>
class SoAHelper<sf::base::IndexSequence<Is...>, T>
{
public:
    using Type = SoA<T, sf::base::IndexSequence<Is...>, sf::base::minipfr::FieldType<Is, T>...>;
};

////////////////////////////////////////////////////////////
template <typename T>
using SoAFor = typename SoAHelper<SFML_BASE_MAKE_INDEX_SEQUENCE(sf::base::minipfr::numFields<T>), T>::Type;
