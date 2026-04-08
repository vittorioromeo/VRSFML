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
/// \brief Owning handle to an OpenGL Vertex Buffer Object
///
/// `sf::VBOHandle` wraps a single OpenGL VBO. It is intended
/// to be passed to `sf::InstanceAttributeBinder::bindVBO`
/// inside the `setupFn` callback of an instanced draw call,
/// after which `setup` declares one or more attribute streams
/// that pull from this VBO.
///
/// `VBOHandle` is move-only and should be cached across frames.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] VBOHandle
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates a fresh OpenGL VBO with no data.
    ///
    ////////////////////////////////////////////////////////////
    VBOHandle();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Releases the underlying OpenGL VBO.
    ///
    ////////////////////////////////////////////////////////////
    ~VBOHandle();

    ////////////////////////////////////////////////////////////
    VBOHandle(const VBOHandle&)            = delete;
    VBOHandle& operator=(const VBOHandle&) = delete;

    ////////////////////////////////////////////////////////////
    VBOHandle(VBOHandle&&) noexcept;
    VBOHandle& operator=(VBOHandle&&) noexcept;

private:
    friend InstanceAttributeBinder;

    ////////////////////////////////////////////////////////////
    /// \brief Internal helper that binds the VBO as `GL_ARRAY_BUFFER`
    ///
    ////////////////////////////////////////////////////////////
    void bind();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 64> m_impl; //!< Implementation details (PImpl)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::VBOHandle
/// \ingroup graphics
///
/// `sf::VBOHandle` is the user-facing wrapper around an
/// OpenGL Vertex Buffer Object. It is the storage that
/// `sf::InstanceAttributeBinder::uploadData` writes per-instance
/// data into and that `sf::InstanceAttributeBinder::setup`
/// then reads from.
///
/// `VBOHandle` is move-only and should be cached across frames
/// to avoid unnecessary OpenGL allocations.
///
/// \see `sf::VAOHandle`, `sf::InstanceAttributeBinder`,
///      `sf::RenderTarget`
///
////////////////////////////////////////////////////////////
