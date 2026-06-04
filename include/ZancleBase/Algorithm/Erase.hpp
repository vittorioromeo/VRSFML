#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Algorithm/Remove.hpp"
#include "ZancleBase/SizeT.hpp"


namespace zb
{
////////////////////////////////////////////////////////////
/// \brief Stable in-place equivalent of `std::erase_if` for vectors; returns the number of elements removed
///
////////////////////////////////////////////////////////////
template <typename Vector, typename Predicate>
[[gnu::always_inline]] inline constexpr SizeT vectorEraseIf(Vector& vector, Predicate&& predicate)
{
    const auto it       = removeIf(vector.begin(), vector.end(), predicate);
    const auto nRemoved = static_cast<SizeT>(vector.end() - it);

    vector.erase(it, vector.end());
    return nRemoved;
}

} // namespace zb
