#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Returns an rvalue reference to a hypothetical `T`
///        without requiring `T` to be constructible
///
////////////////////////////////////////////////////////////
template <typename T>
T&& declVal() noexcept;

} // namespace za
