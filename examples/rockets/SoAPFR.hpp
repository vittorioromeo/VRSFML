#pragma once

#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TypePackElement.hpp"
#include "SFML/Base/Vector.hpp"

#include <boost/pfr/core.hpp>
#include <boost/pfr/tuple_size.hpp>

namespace detail
{
////////////////////////////////////////////////////////////
template <typename>
struct PMTraits;

////////////////////////////////////////////////////////////
template <typename TMemberType, typename TClassType>
struct PMTraits<TMemberType TClassType::*>
{
    using ClassType = TClassType;
};

////////////////////////////////////////////////////////////
template <auto X>
concept PointerToMember = requires { typename PMTraits<decltype(X)>::ClassType; };

} // namespace detail


////////////////////////////////////////////////////////////
template <sf::base::SizeT I, typename T>
struct SoABase
{
    ////////////////////////////////////////////////////////////
    enum : sf::base::SizeT
    {
        index = I
    };

    ////////////////////////////////////////////////////////////
    [[no_unique_address]] sf::base::Vector<T> data;
};

////////////////////////////////////////////////////////////
template <auto>
struct SoAField
{
};

////////////////////////////////////////////////////////////
template <typename, typename, typename...>
class SoA;

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOA_AS_BASE(I)       static_cast<SoABase<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_AS_CONST_BASE(I) static_cast<const SoABase<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_ALL_BASES()      static_cast<SoABase<Is, Ts>&>(*this)

////////////////////////////////////////////////////////////
template <typename T, sf::base::SizeT... Is, typename... Ts>
class SoA<T, sf::base::IndexSequence<Is...>, Ts...> : private SoABase<Is, Ts>...
{
private:
    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[gnu::always_inline]] static consteval sf::base::SizeT fieldIndexFromPM()
    {
        constexpr T obj{};

        auto result = static_cast<sf::base::SizeT>(-1);

        (...,
         ((static_cast<const void*>(&(boost::pfr::get<Is>(obj))) == static_cast<const void*>(&(obj.*PM))) ? (result = Is) : 0));

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
        emplaceBack(boost::pfr::get<Is>(aos)...);
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
        requires detail::PointerToMember<PM>
    {
        return get<fieldIndexFromPM<PM>()>();
    }

    ////////////////////////////////////////////////////////////
    template <auto PM>
    [[nodiscard, gnu::always_inline]] const auto& get() const noexcept
        requires detail::PointerToMember<PM>
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
        for (sf::base::SizeT i = start; i < end; ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void withAllSubRange(const sf::base::SizeT start, const sf::base::SizeT end, auto&& f)
    {
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
        sf::base::SizeT n = getSize();
        sf::base::SizeT i = 0u;

        // Process elements, swapping out removed ones.
        while (i < n)
        {
            if (!f(SOA_AS_BASE(Js).data[i]...))
            {
                ++i;
                continue;
            }

            // Swap the current element with the last one, then reduce the container size.
            --n;
            // (..., sf::base::swap(SOA_ALL_BASES().data[i], SOA_ALL_BASES().data[n]));
            (..., (SOA_ALL_BASES().data[i] = SFML_BASE_MOVE(SOA_ALL_BASES().data[n])));

            // Do not increment `i`; check the new element at `i`.
        }

        // Resize all columns to the new size.
        (..., SOA_ALL_BASES().data.resize(n));
    }

    ////////////////////////////////////////////////////////////
    template <auto... PMs>
    void eraseIfBySwapping(auto&& f)
        requires(... && detail::PointerToMember<PMs>)
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
    using Type = SoA<T, sf::base::IndexSequence<Is...>, boost::pfr::tuple_element_t<Is, T>...>;
};

////////////////////////////////////////////////////////////
template <typename T>
using SoAFor = typename SoAHelper<SFML_BASE_MAKE_INDEX_SEQUENCE(boost::pfr::tuple_size_v<T>), T>::Type;
