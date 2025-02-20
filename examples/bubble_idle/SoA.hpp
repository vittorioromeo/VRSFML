#pragma once

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/TrivialVector.hpp"
#include "SFML/Base/TypePackElement.hpp"


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
    [[no_unique_address]] sf::base::TrivialVector<T> data;
};

////////////////////////////////////////////////////////////
template <typename, typename...>
class SoA;

////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOA_AS_BASE(I)  static_cast<SoABase<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this)
#define SOA_ALL_BASES() static_cast<SoABase<Is, Ts>&>(*this)

////////////////////////////////////////////////////////////
template <sf::base::SizeT... Is, typename... Ts>
class SoA<sf::base::IndexSequence<Is...>, Ts...> : private SoABase<Is, Ts>...
{
public:
    ////////////////////////////////////////////////////////////
    void pushBack(auto&&... values)
    {
        (SOA_ALL_BASES().data.pushBack(SFML_BASE_FORWARD(values)), ...);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] sf::base::SizeT getSize() const
    {
        return SOA_AS_BASE(0).data.size();
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    [[gnu::always_inline]] void with(auto&& f)
    {
        for (sf::base::SizeT i = 0u; i < getSize(); ++i)
            f(SOA_AS_BASE(Js).data[i]...);
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    void eraseIfByShifting(auto&& f)
    {
        const sf::base::SizeT n = getSize();

        // Find the first element to remove.
        sf::base::SizeT i = 0;
        while (i < n && !f(SOA_AS_BASE(Js).data[i]...))
            ++i;

        // For the remaining elements, shift over those that must be kept.
        sf::base::SizeT newSize = i;

        for (; i < n; ++i)
        {
            if (f(SOA_AS_BASE(Js).data[i]...))
                continue;

            if (newSize != i)
                ((SOA_ALL_BASES().data[newSize] = SFML_BASE_MOVE(SOA_ALL_BASES().data[i])), ...);

            ++newSize;
        }

        // Resize all columns to the new size.
        ((SOA_ALL_BASES().data.resize(newSize)), ...);
    }

    ////////////////////////////////////////////////////////////
    template <sf::base::SizeT... Js>
    void eraseIfBySwapping(auto&& f)
    {
        sf::base::SizeT n = getSize();
        sf::base::SizeT i = 0;

        // Process elements, swapping out removed ones.
        while (i < n)
        {
            if (!f(static_cast<SoABase<Js, SFML_BASE_TYPE_PACK_ELEMENT(Js, Ts...)>&>(*this).data[i]...))
            {
                ++i;
                continue;
            }

            // Swap the current element with the last one, then reduce the container size.
            --n;
            ((sf::base::swap(SOA_ALL_BASES().data[i], SOA_ALL_BASES().data[n])), ...);

            // Do not increment i; check the new element at i.
        }

        // Resize all columns to the new size.
        ((SOA_ALL_BASES().data.resize(n)), ...);
    }
};

////////////////////////////////////////////////////////////
#undef SOA_ALL_BASES
#undef SOA_AS_BASE

////////////////////////////////////////////////////////////
template <typename... Ts>
using SoAFor = SoA<SFML_BASE_MAKE_INDEX_SEQUENCE(sizeof...(Ts)), Ts...>;
