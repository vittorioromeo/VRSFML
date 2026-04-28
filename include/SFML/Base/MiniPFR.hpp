#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief Compile-time reflection for aggregate types
///
/// `sf::base::minipfr` is a small fork of Boost.PFR that does not
/// depend on the C++ standard library. It supports aggregate types of
/// up to 64 fields and provides:
///
/// - `numFields<T>` -- number of public data members
///
/// - `tieAsTuple(obj)` -- bind every field of `obj` into a tuple of refs
///
/// - `getField<I>(obj)` -- fetch the `I`-th field by index
///
/// - `forEachField(obj, fn)` -- invoke `fn` on each field in declaration order
///
/// - `getFieldName<T, I>()` / `tieAsFieldNamesTuple<T>()` -- extract
///   field names by parsing the compiler's pretty-function string
///
/// Unions and C-style array types are explicitly rejected by the
/// public reflection entry points.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Small fork of Boost.PFR that does not depend on the Standard Libary.
// Boost.PFR license:
/*
   Copyright (c) 2016-2025 Antony Polukhin

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
////////////////////////////////////////////////////////////

#include "SFML/Base/Array.hpp"
#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsArray.hpp"
#include "SFML/Base/Trait/IsUnion.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"
#include "SFML/Base/Trait/RemoveReference.hpp"
#include "SFML/Base/TypePackElement.hpp"


namespace sf::base::minipfr::priv
{
////////////////////////////////////////////////////////////
template <SizeT N, typename T>
struct TupleMember
{
    ////////////////////////////////////////////////////////////
    [[no_unique_address]] T value;
};


////////////////////////////////////////////////////////////
template <typename IdxSeq, typename... Ts>
struct TupleBase;


////////////////////////////////////////////////////////////
template <SizeT... Is, typename... Ts>
struct TupleBase<IndexSequence<Is...>, Ts...> : TupleMember<Is, Ts>...
{
    ////////////////////////////////////////////////////////////
    enum : SizeT
    {
        memberCount = sizeof...(Ts)
    };


    ////////////////////////////////////////////////////////////
    constexpr TupleBase() = default;


    ////////////////////////////////////////////////////////////
    constexpr TupleBase(const TupleBase&) = default;
    constexpr TupleBase(TupleBase&&)      = default;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr TupleBase(Ts... v) : TupleMember<Is, Ts>{v}...
    {
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[gnu::always_inline]] constexpr auto& get()
    {
        return static_cast<TupleMember<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this).value;
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[gnu::always_inline]] constexpr const auto& get() const
    {
        return static_cast<const TupleMember<I, SFML_BASE_TYPE_PACK_ELEMENT(I, Ts...)>&>(*this).value;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void forEach(auto&& f)
    {
        (..., f(static_cast<TupleMember<Is, Ts>&>(*this).value));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void forEach(auto&& f) const
    {
        (..., f(static_cast<const TupleMember<Is, Ts>&>(*this).value));
    }
};


////////////////////////////////////////////////////////////
template <>
struct TupleBase<IndexSequence<>>
{
    ////////////////////////////////////////////////////////////
    enum : SizeT
    {
        memberCount = 0u
    };


    ////////////////////////////////////////////////////////////
    constexpr TupleBase() = default;
};


////////////////////////////////////////////////////////////
template <typename... Ts>
struct Tuple : TupleBase<SFML_BASE_INDEX_SEQUENCE_FOR(Ts), Ts...>
{
    ////////////////////////////////////////////////////////////
    using TupleBase<SFML_BASE_INDEX_SEQUENCE_FOR(Ts), Ts...>::TupleBase;
};


////////////////////////////////////////////////////////////
struct Anything
{
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard]] consteval operator T&() const;
};


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] consteval SizeT countFields()
{
    Anything x;

    // NOLINTBEGIN(readability-misleading-indentation)

    // clang-format off
         if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x}; }) { return 64u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};   }) { return 63u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};     }) { return 62u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};       }) { return 61u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};         }) { return 60u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};           }) { return 59u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};             }) { return 58u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};               }) { return 57u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                 }) { return 56u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                   }) { return 55u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                     }) { return 54u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                       }) { return 53u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                         }) { return 52u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                           }) { return 51u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                             }) { return 50u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                               }) { return 49u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                 }) { return 48u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                   }) { return 47u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                     }) { return 46u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                       }) { return 45u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                         }) { return 44u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                           }) { return 43u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                             }) { return 42u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                               }) { return 41u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                 }) { return 40u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                   }) { return 39u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                     }) { return 38u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                       }) { return 37u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                         }) { return 36u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                           }) { return 35u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                             }) { return 34u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                               }) { return 33u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                 }) { return 32u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                   }) { return 31u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                     }) { return 30u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                       }) { return 29u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                         }) { return 28u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                           }) { return 27u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                             }) { return 26u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                               }) { return 25u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                 }) { return 24u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                   }) { return 23u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                     }) { return 22u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                       }) { return 21u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                         }) { return 20u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                           }) { return 19u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                             }) { return 18u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                               }) { return 17u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                                 }) { return 16u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                                   }) { return 15u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                                     }) { return 14u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x};                                                                                                       }) { return 13u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x};                                                                                                         }) { return 12u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x};                                                                                                           }) { return 11u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x};                                                                                                             }) { return 10u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x};                                                                                                               }) { return 9u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x};                                                                                                                 }) { return 8u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x};                                                                                                                   }) { return 7u; }
    else if constexpr (requires { T{x,x,x,x,x,x};                                                                                                                     }) { return 6u; }
    else if constexpr (requires { T{x,x,x,x,x};                                                                                                                       }) { return 5u; }
    else if constexpr (requires { T{x,x,x,x};                                                                                                                         }) { return 4u; }
    else if constexpr (requires { T{x,x,x};                                                                                                                           }) { return 3u; }
    else if constexpr (requires { T{x,x};                                                                                                                             }) { return 2u; }
    else if constexpr (requires { T{x};                                                                                                                               }) { return 1u; }
    else if constexpr (requires { T{};                                                                                                                                }) { return 0u; }
    // clang-format on
    else
    {
        static_assert(sizeof(T) == 0, "Type is not aggregate initializable or has more than 64 fields.");
    }

    // NOLINTEND(readability-misleading-indentation)
}


////////////////////////////////////////////////////////////
template <SizeT I, typename T>
[[gnu::always_inline]] constexpr T& getImpl(TupleMember<I, T>& t)
{
    return t.value;
}


////////////////////////////////////////////////////////////
template <SizeT I, typename T>
[[gnu::always_inline]] constexpr T&& getImpl(TupleMember<I, T>&& t)
{
    return t.value;
}


////////////////////////////////////////////////////////////
template <SizeT I, typename T>
[[gnu::always_inline]] constexpr const T& getImpl(const TupleMember<I, T>& t)
{
    return t.value;
}


////////////////////////////////////////////////////////////
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wundefined-internal"
    #pragma clang diagnostic ignored "-Wundefined-var-template"
#endif


////////////////////////////////////////////////////////////
template <typename T>
struct FakeObjectWrapper
{
    const T value;
};


////////////////////////////////////////////////////////////
template <typename T>
extern const FakeObjectWrapper<T> fakeObjectImpl;


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard]] consteval const T& getFakeObject() noexcept
{
    return fakeObjectImpl<T>.value;
}


////////////////////////////////////////////////////////////
#ifdef __clang__
    #pragma clang diagnostic pop
#endif


////////////////////////////////////////////////////////////
template <typename... Ts>
[[gnu::always_inline]] constexpr auto makeRefTuple(Ts&... args)
{
    return Tuple<Ts&...>{args...};
}

} // namespace sf::base::minipfr::priv


namespace sf::base::minipfr
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr SizeT numFields = priv::countFields<T>();


////////////////////////////////////////////////////////////
template <typename T>
constexpr auto tieAsTuple(T&& obj)
{
    enum : SizeT
    {
        fieldCount = priv::countFields<SFML_BASE_REMOVE_CVREF(T)>()
    };

#define MINIPFR_RETURN_TIE_AS_TUPLE(N, ...) \
    auto& [__VA_ARGS__] = obj;              \
    return priv::makeRefTuple(__VA_ARGS__)

    // clang-format off
#define MINIPFR_BRANCH(N, ...) \
    constexpr (fieldCount == N) { MINIPFR_RETURN_TIE_AS_TUPLE(N, __VA_ARGS__); }
    // clang-format on

    // NOLINTBEGIN(readability-misleading-indentation)

    // clang-format off
         if constexpr (fieldCount == 0)  { return priv::Tuple<>{}; }
    else if MINIPFR_BRANCH(1,  a)
    else if MINIPFR_BRANCH(2,  a,b)
    else if MINIPFR_BRANCH(3,  a,b,c)
    else if MINIPFR_BRANCH(4,  a,b,c,d)
    else if MINIPFR_BRANCH(5,  a,b,c,d,e)
    else if MINIPFR_BRANCH(6,  a,b,c,d,e,f)
    else if MINIPFR_BRANCH(7,  a,b,c,d,e,f,g)
    else if MINIPFR_BRANCH(8,  a,b,c,d,e,f,g,h)
    else if MINIPFR_BRANCH(9,  a,b,c,d,e,f,g,h,i)
    else if MINIPFR_BRANCH(10, a,b,c,d,e,f,g,h,i,j)
    else if MINIPFR_BRANCH(11, a,b,c,d,e,f,g,h,i,j,k)
    else if MINIPFR_BRANCH(12, a,b,c,d,e,f,g,h,i,j,k,l)
    else if MINIPFR_BRANCH(13, a,b,c,d,e,f,g,h,i,j,k,l,m)
    else if MINIPFR_BRANCH(14, a,b,c,d,e,f,g,h,i,j,k,l,m,n)
    else if MINIPFR_BRANCH(15, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)
    else if MINIPFR_BRANCH(16, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)
    else if MINIPFR_BRANCH(17, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)
    else if MINIPFR_BRANCH(18, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)
    else if MINIPFR_BRANCH(19, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)
    else if MINIPFR_BRANCH(20, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)
    else if MINIPFR_BRANCH(21, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u)
    else if MINIPFR_BRANCH(22, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)
    else if MINIPFR_BRANCH(23, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w)
    else if MINIPFR_BRANCH(24, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x)
    else if MINIPFR_BRANCH(25, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y)
    else if MINIPFR_BRANCH(26, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z)
    else if MINIPFR_BRANCH(27, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A)
    else if MINIPFR_BRANCH(28, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B)
    else if MINIPFR_BRANCH(29, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C)
    else if MINIPFR_BRANCH(30, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D)
    else if MINIPFR_BRANCH(31, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E)
    else if MINIPFR_BRANCH(32, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F)
    else if MINIPFR_BRANCH(33, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G)
    else if MINIPFR_BRANCH(34, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H)
    else if MINIPFR_BRANCH(35, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J)
    else if MINIPFR_BRANCH(36, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K)
    else if MINIPFR_BRANCH(37, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L)
    else if MINIPFR_BRANCH(38, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M)
    else if MINIPFR_BRANCH(39, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N)
    else if MINIPFR_BRANCH(40, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O)
    else if MINIPFR_BRANCH(41, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P)
    else if MINIPFR_BRANCH(42, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q)
    else if MINIPFR_BRANCH(43, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R)
    else if MINIPFR_BRANCH(44, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S)
    else if MINIPFR_BRANCH(45, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U)
    else if MINIPFR_BRANCH(46, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V)
    else if MINIPFR_BRANCH(47, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W)
    else if MINIPFR_BRANCH(48, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X)
    else if MINIPFR_BRANCH(49, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y)
    else if MINIPFR_BRANCH(50, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z)
    else if MINIPFR_BRANCH(51, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa)
    else if MINIPFR_BRANCH(52, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb)
    else if MINIPFR_BRANCH(53, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc)
    else if MINIPFR_BRANCH(54, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd)
    else if MINIPFR_BRANCH(55, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee)
    else if MINIPFR_BRANCH(56, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff)
    else if MINIPFR_BRANCH(57, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg)
    else if MINIPFR_BRANCH(58, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh)
    else if MINIPFR_BRANCH(59, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh,ii)
    else if MINIPFR_BRANCH(60, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj)
    else if MINIPFR_BRANCH(61, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk)
    else if MINIPFR_BRANCH(62, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll)
    else if MINIPFR_BRANCH(63, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm)
    else if MINIPFR_BRANCH(64, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,J,K,L,M,N,O,P,Q,R,S,U,V,W,X,Y,Z,aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn)
            // clang-format on
            else
        {
            static_assert(sizeof(T) == 0, "`tieAsTuple` supports up to 64 fields.");
        }

    // NOLINTEND(readability-misleading-indentation)

#undef MINIPFR_BRANCH
#undef MINIPFR_RETURN_TIE_AS_TUPLE
}


////////////////////////////////////////////////////////////
template <SizeT I>
[[gnu::always_inline]] constexpr auto&& getField(auto&& obj)
{
    return priv::getImpl<I>(tieAsTuple(SFML_BASE_FORWARD(obj)));
}


////////////////////////////////////////////////////////////
template <SizeT I, typename T>
using FieldType = SFML_BASE_REMOVE_REFERENCE(decltype(getField<I>(declVal<T&>())));


////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr void forEachField(auto&& obj, auto&& f)
{
    tieAsTuple(SFML_BASE_FORWARD(obj)).forEach(SFML_BASE_FORWARD(f));
}

} // namespace sf::base::minipfr


namespace sf::base::minipfr::priv
{
////////////////////////////////////////////////////////////
// All field names of `T` are extracted from a single `__PRETTY_FUNCTION__`,
// shaped like:
//
//   Clang: `... Refs = <wImpl.value.f0, wImpl.value.f1, ...>]`
//   GCC:   `... Refs = {wImpl<T>.Wrap<T>::value.T::f0, wImpl<T>.Wrap<T>::value.T::f1, ...}]`
//
// On GCC the type T is rendered inline (e.g. `Members<0, int>`, or
// `{anonymous}::Foo` for types in an unnamed namespace) so segments may
// contain `,`, `<`, `>`, `{`, and `}` at depth > 0. The parser counts both
// `<>` and `{}` as balanced bracket pairs and only treats those characters
// as separators or close markers at depth 0.
//
// On Clang the type and the `<...>` template arguments are elided from the NTTP printout, so
// segments are simple identifier chains -- the bracket tracking is harmless there.
////////////////////////////////////////////////////////////
#if defined(__clang__)

inline constexpr char kOpenMarker[] = "Refs = <";

enum : char
{
    kCloseChar    = '>',
    kNameMarkerCh = '.'
};

enum : SizeT
{
    kNameMarkerLen = 1u
};

#elif defined(__GNUC__)

inline constexpr char kOpenMarker[] = "Refs = {";

enum : char
{
    kCloseChar    = '}',
    kNameMarkerCh = ':'
};

enum : SizeT
{
    kNameMarkerLen = 2u
};

#else
    #error "MiniPFR field-name extraction is only supported on Clang and GCC"
#endif


////////////////////////////////////////////////////////////
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wundefined-var-template"
#endif


////////////////////////////////////////////////////////////
// The single NTTP-heavy template. Carrying all field references into one
// `__PRETTY_FUNCTION__` is the whole point; downstream parsing happens in
// non-NTTP code so we do not pay mangling cost for the same pack three times.
////////////////////////////////////////////////////////////
template <auto&... Refs>
[[nodiscard]] consteval const char* getRawSignature() noexcept
{
    return __PRETTY_FUNCTION__;
}


////////////////////////////////////////////////////////////
// Loose scratch buffer used during parsing. Tightened to exact size in
// `computeStoredFieldNames` so end users only pay for the actual bytes.
////////////////////////////////////////////////////////////
template <SizeT N>
struct RawFieldNames
{
    Array<char, 4096u>            chars{};
    Array<unsigned short, N + 1u> offsets{};
    SizeT                         total{};
};


////////////////////////////////////////////////////////////
// Walks the signature once, char-by-char, tracking `<>` depth so that commas
// inside template arguments are not mistaken for segment separators. Within
// each segment, the rightmost depth-0 occurrence of `kNameMarkerCh` (".",
// "::") marks the start of the field name.
//
// `parseRawNames` is keyed on `N` only -- it is shared across every type that
// has the same field count, so the heavy compile-time work happens once per
// distinct field count rather than once per distinct type.
////////////////////////////////////////////////////////////
template <SizeT N>
[[nodiscard]] consteval RawFieldNames<N> parseRawNames(const char* sig) noexcept
{
    RawFieldNames<N> r{};

    if constexpr (N == 0u)
    {
        return r;
    }
    else
    {
        const char* p = sig;

        // Locate the open marker at the start of the pack body.
        while (true)
        {
            bool match = true;

            for (SizeT k = 0u; kOpenMarker[k] != '\0'; ++k)
            {
                if (p[k] != kOpenMarker[k])
                {
                    match = false;
                    break;
                }
            }

            if (match)
            {
                p += sizeof(kOpenMarker) - 1u;
                break;
            }

            ++p;
        }

        SizeT writeIdx = 0u;
        for (SizeT i = 0u; i < N; ++i)
        {
            r.offsets[i] = static_cast<unsigned short>(writeIdx);

            const char* nameStart = p;

            // Hot path at depth 0: find the next segment terminator while
            // tracking the rightmost name marker.
            while (true)
            {
                const char c = *p;

                if (c == ',' || c == kCloseChar)
                    break;

                if (c == kNameMarkerCh)
                {
                    nameStart = p + kNameMarkerLen;
                    p         = nameStart;
                    continue;
                }

                if (c == '<' || c == '{')
                {
                    // Depth > 0: skip to matching close. Both `<>` and `{}`
                    // are tracked as balanced pairs because GCC may emit
                    // either inside a segment (e.g. `<TmplArgs>` or
                    // `{anonymous}` for unnamed-namespace types).
                    int depth = 1;
                    ++p;

                    while (depth > 0)
                    {
                        const char d = *p;

                        if (d == '<' || d == '{')
                            ++depth;
                        else if (d == '>' || d == '}')
                            --depth;

                        ++p;
                    }

                    continue;
                }

                ++p;
            }

            for (const char* c = nameStart; c != p; ++c)
                r.chars[writeIdx++] = *c;

            if (*p == ',')
                p += 2u; // skip ", "
        }

        r.offsets[N] = static_cast<unsigned short>(writeIdx);
        r.total      = writeIdx;

        return r;
    }
}


////////////////////////////////////////////////////////////
template <SizeT N, SizeT TotalChars>
struct PackedFieldNames
{
    // The branch that returns `PackedFieldNames<0, 0>` for empty aggregates is
    // discarded by `if constexpr`, but the type is still instantiated. Clamp
    // the chars array size so `Array<char, 0>` (unsupported) never appears.
    Array<char, (TotalChars == 0u ? 1u : TotalChars)> chars{};
    Array<unsigned short, N + 1u>                     offsets{};
};


////////////////////////////////////////////////////////////
template <typename T, SizeT... Is>
[[nodiscard]] consteval auto computeStoredFieldNames(IndexSequence<Is...>) noexcept
{
    constexpr SizeT n = sizeof...(Is);

    if constexpr (n == 0u)
    {
        return PackedFieldNames<0u, 0u>{};
    }
    else
    {
        // A constexpr lvalue tuple is required so its `.get<I>()` lvalue refs
        // are valid as `auto&` non-type template arguments.
        constexpr auto        fakeTuple = tieAsTuple(getFakeObject<T>());
        constexpr const char* sig       = getRawSignature<fakeTuple.template get<Is>()...>();
        constexpr auto        raw       = parseRawNames<n>(sig);

        PackedFieldNames<n, raw.total> packed{};

        for (SizeT i = 0u; i < raw.total; ++i)
            packed.chars[i] = raw.chars[i];

        for (SizeT i = 0u; i <= n; ++i)
            packed.offsets[i] = raw.offsets[i];

        return packed;
    }
}


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr auto storedFieldNames = computeStoredFieldNames<T>(MakeIndexSequence<numFields<T>>{});


////////////////////////////////////////////////////////////
#ifdef __clang__
    #pragma clang diagnostic pop
#endif


////////////////////////////////////////////////////////////
template <SizeT>
using AlwaysStringView = StringView;

} // namespace sf::base::minipfr::priv


namespace sf::base::minipfr
{
////////////////////////////////////////////////////////////
template <typename T, SizeT I>
constexpr StringView getFieldName() noexcept
{
    static_assert(!SFML_BASE_IS_UNION(T), "union reflection is forbidden");
    static_assert(!SFML_BASE_IS_ARRAY(T), "impossible to extract name from C-style array");

    constexpr const auto& packed = priv::storedFieldNames<T>;

    return StringView{packed.chars.data() + packed.offsets[I],
                      static_cast<SizeT>(packed.offsets[I + 1u] - packed.offsets[I])};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr auto tieAsFieldNamesTuple() noexcept
{
    static_assert(!SFML_BASE_IS_UNION(T), "union reflection is forbidden");
    static_assert(!SFML_BASE_IS_ARRAY(T), "impossible to extract name from C-style array");

    return []<SizeT... Is>(IndexSequence<Is...>)
    { return priv::Tuple<priv::AlwaysStringView<Is>...>{getFieldName<T, Is>()...}; }(MakeIndexSequence<numFields<T>>{});
}

} // namespace sf::base::minipfr


////////////////////////////////////////////////////////////
namespace sf::base::minipfr::priv
{
////////////////////////////////////////////////////////////
// Sanity-check on include.
struct FieldNameSelfCheck
{
    int  alpha;
    char beta;
};

static_assert(getFieldName<FieldNameSelfCheck, 0u>() == StringView{"alpha"});
static_assert(getFieldName<FieldNameSelfCheck, 1u>() == StringView{"beta"});

} // namespace sf::base::minipfr::priv
