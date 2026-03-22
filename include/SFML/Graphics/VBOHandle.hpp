#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/InPlacePImpl.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
struct InstanceAttributeBinder;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] VBOHandle
{
public:
    VBOHandle();
    ~VBOHandle();

    VBOHandle(const VBOHandle&)            = delete;
    VBOHandle& operator=(const VBOHandle&) = delete;

    VBOHandle(VBOHandle&&) noexcept;
    VBOHandle& operator=(VBOHandle&&) noexcept;

private:
    friend InstanceAttributeBinder;

    void bind();

    struct Impl;
    base::InPlacePImpl<Impl, 64> m_impl;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderTarget
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see TODO P1: docs
///
////////////////////////////////////////////////////////////
