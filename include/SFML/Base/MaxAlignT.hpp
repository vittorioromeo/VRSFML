#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief `std::max_align_t` replacement that does not require `<cstddef>`
///
/// Has the strictest fundamental alignment of any scalar type, suitable
/// as the alignment for raw storage that may hold any standard-layout
/// object (e.g. inside `InPlacePImpl`).
///
////////////////////////////////////////////////////////////
struct MaxAlignT
{
    alignas(alignof(long long)) long long a;
    alignas(alignof(long double)) long double b;
};

} // namespace sf::base
