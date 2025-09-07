#pragma once

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsMemberPointer.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"
#include "SFML/Base/Traits/RemoveReference.hpp"
#include "SFML/Base/TypePackElement.hpp"
#include "SFML/Base/Vector.hpp"


namespace minipfr
{
namespace priv
{
////////////////////////////////////////////////////////////
template <sf::base::SizeT N, typename T>
struct TupleMember
{
    ////////////////////////////////////////////////////////////
    [[no_unique_address]] T value;
};


////////////////////////////////////////////////////////////
template <typename IdxSeq, typename... Ts>
struct TupleBase;


////////////////////////////////////////////////////////////
template <sf::base::SizeT... Is, typename... Ts>
struct TupleBase<sf::base::IndexSequence<Is...>, Ts...> : TupleMember<Is, Ts>...
{
    ////////////////////////////////////////////////////////////
    enum : sf::base::SizeT
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
struct TupleBase<sf::base::IndexSequence<>>
{
    ////////////////////////////////////////////////////////////
    enum : sf::base::SizeT
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
consteval sf::base::SizeT countFields()
{
    Anything x;

    // NOLINTBEGIN(readability-misleading-indentation)

    // clang-format off
         if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};  }) { return 24u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};    }) { return 23u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};      }) { return 22u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};        }) { return 21u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};          }) { return 20u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};            }) { return 19u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};              }) { return 18u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                }) { return 17u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                  }) { return 16u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x,x};                    }) { return 15u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x,x};                      }) { return 14u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x,x};                        }) { return 13u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x,x};                          }) { return 12u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x,x};                            }) { return 11u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x,x};                              }) { return 10u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x,x};                                }) { return 9u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x,x};                                  }) { return 8u; }
    else if constexpr (requires { T{x,x,x,x,x,x,x};                                    }) { return 7u; }
    else if constexpr (requires { T{x,x,x,x,x,x};                                      }) { return 6u; }
    else if constexpr (requires { T{x,x,x,x,x};                                        }) { return 5u; }
    else if constexpr (requires { T{x,x,x,x};                                          }) { return 4u; }
    else if constexpr (requires { T{x,x,x};                                            }) { return 3u; }
    else if constexpr (requires { T{x,x};                                              }) { return 2u; }
    else if constexpr (requires { T{x};                                                }) { return 1u; }
    else if constexpr (requires { T{};                                                 }) { return 0u; }
    // clang-format on

    else
    {
        static_assert(sizeof(T) == 0, "Type is not aggregate initializable or has more than 32 fields.");
    }

    // NOLINTEND(readability-misleading-indentation)
}


////////////////////////////////////////////////////////////
template <sf::base::SizeT I, typename T>
[[gnu::always_inline]] constexpr T& getImpl(TupleMember<I, T>& t)
{
    return t.value;
}


////////////////////////////////////////////////////////////
template <sf::base::SizeT I, typename T>
[[gnu::always_inline]] constexpr T&& getImpl(TupleMember<I, T>&& t)
{
    return t.value;
}


////////////////////////////////////////////////////////////
template <sf::base::SizeT I, typename T>
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
template <class T>
struct FakeObjectWrapper
{
    const T value;
};


////////////////////////////////////////////////////////////
template <class T>
extern const FakeObjectWrapper<T> fakeObjectImpl;


////////////////////////////////////////////////////////////
template <class T>
consteval const T& getFakeObject() noexcept
{
    return fakeObjectImpl<T>.value;
}


////////////////////////////////////////////////////////////
#ifdef __clang__
    #pragma clang diagnostic pop
#endif


////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr auto makeRefTuple(auto&... args)
{
    return Tuple<decltype(args)&...>{args...};
}

} // namespace priv


////////////////////////////////////////////////////////////
template <typename T>
inline constexpr sf::base::SizeT numFields = priv::countFields<T>();


/*
////////////////////////////////////////////////////////////
#define MINIPFR_DEFINE_TIE_AS_TUPLE(N, ...)                           \
    template <typename T>                                             \
    constexpr auto tieAsTuple(T&& obj)                                \
        requires(priv::countFields<SFML_BASE_REMOVE_CVREF(T)>() == N) \
    {                                                                 \
        auto& [__VA_ARGS__] = obj;                                    \
        return priv::makeRefTuple(__VA_ARGS__);                       \
    }
*/

////////////////////////////////////////////////////////////
template <typename T>
constexpr auto tieAsTuple(T&& obj)
{
    enum : sf::base::SizeT
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
        static_assert(sizeof(T) == 0, "Type has more than 16 fields, tieAsTuple() supports up to 16 fields.");
    }

    // NOLINTEND(readability-misleading-indentation)

#undef MINIPFR_RETURN_TIE_AS_TUPLE
}


////////////////////////////////////////////////////////////
template <sf::base::SizeT I>
[[gnu::always_inline]] constexpr auto&& getField(auto&& obj)
{
    return priv::getImpl<I>(tieAsTuple(SFML_BASE_FORWARD(obj)));
}


////////////////////////////////////////////////////////////
template <sf::base::SizeT I, typename T>
using FieldType = SFML_BASE_REMOVE_REFERENCE(decltype(getField<I>(sf::base::declVal<T&>())));


////////////////////////////////////////////////////////////
[[gnu::always_inline]] constexpr void forEachField(auto&& obj, auto&& f)
{
    tieAsTuple(SFML_BASE_FORWARD(obj)).forEach(SFML_BASE_FORWARD(f));
}

} // namespace minipfr


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
    [[gnu::always_inline]] static consteval sf::base::SizeT fieldIndexFromPM()
    {
        // const auto& obj = minipfr::priv::getFakeObject<T>();
        constexpr T obj{};

        auto result = static_cast<sf::base::SizeT>(-1);

        (...,
         ((static_cast<const void*>(&(minipfr::getField<Is>(obj))) == static_cast<const void*>(&(obj.*PM)))
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
        emplaceBack(minipfr::getField<Is>(aos)...);
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
            (..., (SOA_ALL_BASES().data[i] = SFML_BASE_MOVE(SOA_ALL_BASES().data[n])));

            // Do not increment `i`; check the new element at `i`.
        }

        // Resize all columns to the new size.
        (..., SOA_ALL_BASES().data.resize(n));
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
    using Type = SoA<T, sf::base::IndexSequence<Is...>, minipfr::FieldType<Is, T>...>;
};

////////////////////////////////////////////////////////////
template <typename T>
using SoAFor = typename SoAHelper<SFML_BASE_MAKE_INDEX_SEQUENCE(minipfr::numFields<T>), T>::Type;
