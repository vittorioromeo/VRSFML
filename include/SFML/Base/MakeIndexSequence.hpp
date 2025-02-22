#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
