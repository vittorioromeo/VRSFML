#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


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
#include "ZancleBase/IndexSequence.hpp"
#include "ZancleBase/SizeT.hpp"


#if __has_builtin(__integer_pack)

    ////////////////////////////////////////////////////////////
    #define ZB_MAKE_INDEX_SEQUENCE(N) ::zb::IndexSequence<__integer_pack(N)...>

namespace zb
{
////////////////////////////////////////////////////////////
template <SizeT N>
using MakeIndexSequence = ZB_MAKE_INDEX_SEQUENCE(N);

} // namespace zb

#elif __has_builtin(__make_integer_seq)

namespace zb::priv
{
////////////////////////////////////////////////////////////
template <typename T, T... Is>
struct MakeIndexSequenceHelper
{
    using type = IndexSequence<Is...>;
};

} // namespace zb::priv

namespace zb
{
////////////////////////////////////////////////////////////
template <SizeT N>
using MakeIndexSequence = typename __make_integer_seq<priv::MakeIndexSequenceHelper, SizeT, N>::type;

} // namespace zb

    ////////////////////////////////////////////////////////////
    #define ZB_MAKE_INDEX_SEQUENCE(N) ::zb::MakeIndexSequence<N>

#else

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
    #include <utility>


namespace zb
{
////////////////////////////////////////////////////////////
template <SizeT N>
using MakeIndexSequence = std::make_index_sequence<N>;

} // namespace zb

    ////////////////////////////////////////////////////////////
    #define ZB_MAKE_INDEX_SEQUENCE(N) ::zb::MakeIndexSequence<N>

#endif


////////////////////////////////////////////////////////////
/// \brief Macro form of `std::index_sequence_for`: produces an index sequence matching a parameter pack
///
////////////////////////////////////////////////////////////
#define ZB_INDEX_SEQUENCE_FOR(...) ZB_MAKE_INDEX_SEQUENCE(sizeof...(__VA_ARGS__))
