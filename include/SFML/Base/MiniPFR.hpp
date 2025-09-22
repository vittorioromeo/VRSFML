#pragma once

#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"
#include "SFML/Base/Traits/RemoveReference.hpp"
#include "SFML/Base/TypePackElement.hpp"


namespace sf::base::minipfr
{
namespace priv
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
    consteval operator T&() const;
};


////////////////////////////////////////////////////////////
template <typename T>
consteval SizeT countFields()
{
    Anything x;

    // NOLINTBEGIN(readability-misleading-indentation)

    // clang-format off
         if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x}; }) { return 24u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};   }) { return 23u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};     }) { return 22u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};       }) { return 21u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};         }) { return 20u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};           }) { return 19u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};             }) { return 18u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};               }) { return 17u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                 }) { return 16u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                   }) { return 15u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x};                     }) { return 14u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x};                       }) { return 13u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x};                         }) { return 12u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x};                           }) { return 11u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x};                             }) { return 10u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x};                               }) { return 9u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x};                                 }) { return 8u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x};                                   }) { return 7u; }
    else if constexpr (requires { T{x,x,x,x,x,x};                                     }) { return 6u; }
    else if constexpr (requires { T{x,x,x,x,x};                                       }) { return 5u; }
    else if constexpr (requires { T{x,x,x,x};                                         }) { return 4u; }
    else if constexpr (requires { T{x,x,x};                                           }) { return 3u; }
    else if constexpr (requires { T{x,x};                                             }) { return 2u; }
    else if constexpr (requires { T{x};                                               }) { return 1u; }
    else if constexpr (requires { T{};                                                }) { return 0u; }
    // clang-format on

    else
    {
        static_assert(sizeof(T) == 0, "Type is not aggregate initializable or has more than 24 fields.");
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
consteval const T& getFakeObject() noexcept
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

} // namespace priv


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

    // NOLINTBEGIN(readability-misleading-indentation)

    // clang-format off
         if constexpr (fieldCount == 0)  { return priv::Tuple<>{}; }
    else if constexpr (fieldCount == 1)  { MINIPFR_RETURN_TIE_AS_TUPLE(1,  a); }
    else if constexpr (fieldCount == 2)  { MINIPFR_RETURN_TIE_AS_TUPLE(2,  a,b); }
    else if constexpr (fieldCount == 3)  { MINIPFR_RETURN_TIE_AS_TUPLE(3,  a,b,c); }
    else if constexpr (fieldCount == 4)  { MINIPFR_RETURN_TIE_AS_TUPLE(4,  a,b,c,d); }
    else if constexpr (fieldCount == 5)  { MINIPFR_RETURN_TIE_AS_TUPLE(5,  a,b,c,d,e); }
    else if constexpr (fieldCount == 6)  { MINIPFR_RETURN_TIE_AS_TUPLE(6,  a,b,c,d,e,f); }
    else if constexpr (fieldCount == 7)  { MINIPFR_RETURN_TIE_AS_TUPLE(7,  a,b,c,d,e,f,g); }
    else if constexpr (fieldCount == 8)  { MINIPFR_RETURN_TIE_AS_TUPLE(8,  a,b,c,d,e,f,g,h); }
    else if constexpr (fieldCount == 9)  { MINIPFR_RETURN_TIE_AS_TUPLE(9,  a,b,c,d,e,f,g,h,i); }
    else if constexpr (fieldCount == 10) { MINIPFR_RETURN_TIE_AS_TUPLE(10, a,b,c,d,e,f,g,h,i,j); }
    else if constexpr (fieldCount == 11) { MINIPFR_RETURN_TIE_AS_TUPLE(11, a,b,c,d,e,f,g,h,i,j,k); }
    else if constexpr (fieldCount == 12) { MINIPFR_RETURN_TIE_AS_TUPLE(12, a,b,c,d,e,f,g,h,i,j,k,l); }
    else if constexpr (fieldCount == 13) { MINIPFR_RETURN_TIE_AS_TUPLE(13, a,b,c,d,e,f,g,h,i,j,k,l,m); }
    else if constexpr (fieldCount == 14) { MINIPFR_RETURN_TIE_AS_TUPLE(14, a,b,c,d,e,f,g,h,i,j,k,l,m,n); }
    else if constexpr (fieldCount == 15) { MINIPFR_RETURN_TIE_AS_TUPLE(15, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o); }
    else if constexpr (fieldCount == 16) { MINIPFR_RETURN_TIE_AS_TUPLE(16, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p); }
    else if constexpr (fieldCount == 17) { MINIPFR_RETURN_TIE_AS_TUPLE(17, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q); }
    else if constexpr (fieldCount == 18) { MINIPFR_RETURN_TIE_AS_TUPLE(18, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r); }
    else if constexpr (fieldCount == 19) { MINIPFR_RETURN_TIE_AS_TUPLE(19, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s); }
    else if constexpr (fieldCount == 20) { MINIPFR_RETURN_TIE_AS_TUPLE(20, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t); }
    else if constexpr (fieldCount == 21) { MINIPFR_RETURN_TIE_AS_TUPLE(21, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u); }
    else if constexpr (fieldCount == 22) { MINIPFR_RETURN_TIE_AS_TUPLE(22, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v); }
    else if constexpr (fieldCount == 23) { MINIPFR_RETURN_TIE_AS_TUPLE(23, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w); }
    else if constexpr (fieldCount == 24) { MINIPFR_RETURN_TIE_AS_TUPLE(24, a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x); }
    // clang-format on

    else
    {
        static_assert(sizeof(T) == 0, "`tieAsTuple` supports up to 24 fields.");
    }

    // NOLINTEND(readability-misleading-indentation)

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
