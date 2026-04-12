#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"


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
/// `sf::VBOHandle` wraps a single OpenGL VBO. It is passed to
/// `sf::InstanceAttributeBinder::uploadData` inside the
/// `setupFn` callback of an instanced draw call, which uploads
/// per-instance data into it. On desktop GL, streaming uploads may
/// grow and replace the underlying buffer object, which is why
/// `InstanceAttributeBinder` defers the final `glVertexAttribPointer`
/// calls until every upload for the draw is complete.
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
    VBOHandle(VBOHandle&&) noexcept            = delete;
    VBOHandle& operator=(VBOHandle&&) noexcept = delete;

private:
    friend InstanceAttributeBinder;

    ////////////////////////////////////////////////////////////
    /// \brief Internal helper that binds the VBO as `GL_ARRAY_BUFFER`
    ///
    ////////////////////////////////////////////////////////////
    void bind();

    ////////////////////////////////////////////////////////////
    /// \brief Internal helper for per-frame streaming uploads
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT uploadStreamingData(const void* data, base::SizeT byteCount);

    ////////////////////////////////////////////////////////////
    /// \brief Commit the staged uploads associated with the current draw
    ///
    ////////////////////////////////////////////////////////////
    void commitPendingUploads();

    ////////////////////////////////////////////////////////////
    /// \brief Roll back staged uploads when a draw never gets submitted
    ///
    ////////////////////////////////////////////////////////////
    void rollbackPendingUploads() noexcept;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl; //!< Implementation details (PImpl)
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
