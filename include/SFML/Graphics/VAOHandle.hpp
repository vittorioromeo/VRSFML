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
class RenderTarget;
struct GLVAOGroup;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] VAOHandle
{
public:
    VAOHandle();
    ~VAOHandle();

    VAOHandle(const VAOHandle&)            = delete;
    VAOHandle& operator=(const VAOHandle&) = delete;

    VAOHandle(VAOHandle&&) noexcept;
    VAOHandle& operator=(VAOHandle&&) noexcept;

private:
    friend RenderTarget;

    [[nodiscard]] const GLVAOGroup& asVAOGroup() const;

    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl;
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
