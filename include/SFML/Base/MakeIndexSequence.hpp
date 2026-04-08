#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
/// \file
/// \brief `std::make_index_sequence<N>` replacement
///
/// `MakeIndexSequence<N>` evaluates to `IndexSequence<0, 1, ..., N-1>`.
/// The implementation prefers compiler builtins (`__integer_pack` on
/// GCC, `__make_integer_seq` on Clang/MSVC) so that the alias does not
/// drag in `<utility>`. Falls back to `std::make_index_sequence` only
/// when no builtin is available.
///
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/SizeT.hpp"


#if __has_builtin(__integer_pack)

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MAKE_INDEX_SEQUENCE(N) ::sf::base::IndexSequence<__integer_pack(N)...>

namespace sf::base
{
////////////////////////////////////////////////////////////
template <SizeT N>
using MakeIndexSequence = SFML_BASE_MAKE_INDEX_SEQUENCE(N);

} // namespace sf::base

#elif __has_builtin(__make_integer_seq)

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, T... Is>
struct MakeIndexSequenceHelper
{
    using type = IndexSequence<Is...>;
};

} // namespace sf::base::priv

namespace sf::base
{
////////////////////////////////////////////////////////////
template <SizeT N>
using MakeIndexSequence = typename __make_integer_seq<priv::MakeIndexSequenceHelper, SizeT, N>::type;

} // namespace sf::base

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MAKE_INDEX_SEQUENCE(N) ::sf::base::MakeIndexSequence<N>

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <utility>


namespace sf::base
{
////////////////////////////////////////////////////////////
template <SizeT N>
using MakeIndexSequence = std::make_index_sequence<N>;

} // namespace sf::base

    ////////////////////////////////////////////////////////////
    #define SFML_BASE_MAKE_INDEX_SEQUENCE(N) ::sf::base::MakeIndexSequence<N>

#endif


////////////////////////////////////////////////////////////
/// \brief Macro form of `std::index_sequence_for`: produces an index sequence matching a parameter pack
///
////////////////////////////////////////////////////////////
#define SFML_BASE_INDEX_SEQUENCE_FOR(...) SFML_BASE_MAKE_INDEX_SEQUENCE(sizeof...(__VA_ARGS__))
