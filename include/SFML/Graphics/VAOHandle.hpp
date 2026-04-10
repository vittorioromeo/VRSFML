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
/// \brief Owning handle to an OpenGL Vertex Array Object
///
/// `sf::VAOHandle` wraps a single OpenGL VAO. It is the VAO
/// that the instanced draw paths
/// (`sf::RenderTarget::drawInstancedVertices` and
/// `sf::RenderTarget::drawInstancedIndexedVertices`) bind
/// before issuing the draw call. Per-instance data is stored
/// in separate `sf::VBOHandle` objects managed by the user.
///
/// `VAOHandle` is move-only. Construct one per "instance set"
/// you want to render and reuse it across frames; the
/// underlying GPU object stays alive for the lifetime of the
/// handle.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] VAOHandle
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates a fresh OpenGL VAO with no attribute streams
    /// configured. Configure it from a `setupFn` callback passed
    /// to one of the instanced draw paths.
    ///
    ////////////////////////////////////////////////////////////
    VAOHandle();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Releases the underlying OpenGL VAO.
    ///
    ////////////////////////////////////////////////////////////
    ~VAOHandle();

    ////////////////////////////////////////////////////////////
    VAOHandle(const VAOHandle&)            = delete;
    VAOHandle& operator=(const VAOHandle&) = delete;

    ////////////////////////////////////////////////////////////
    VAOHandle(VAOHandle&&) noexcept;
    VAOHandle& operator=(VAOHandle&&) noexcept;

private:
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    /// \brief Internal accessor used by `sf::RenderTarget` during draw
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const GLVAOGroup& asVAOGroup() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl; //!< Implementation details (PImpl)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::VAOHandle
/// \ingroup graphics
///
/// `sf::VAOHandle` is the user-facing wrapper around an
/// OpenGL Vertex Array Object. It is used together with
/// `sf::DrawInstancedVerticesSettings` and
/// `sf::DrawInstancedIndexedVerticesSettings` to drive the
/// instanced rendering path of `sf::RenderTarget`. Per-instance
/// data lives in user-managed `sf::VBOHandle` objects, which are
/// passed to `sf::InstanceAttributeBinder::uploadData` from
/// inside the `setupFn` callback that the draw functions take.
///
/// `VAOHandle` is move-only and should be cached -- recreating
/// it every frame defeats the entire point of instanced
/// rendering.
///
/// \see `sf::RenderTarget`, `sf::InstanceAttributeBinder`,
///      `sf::VBOHandle`, `sf::DrawInstancedVerticesSettings`,
///      `sf::DrawInstancedIndexedVerticesSettings`
///
////////////////////////////////////////////////////////////
