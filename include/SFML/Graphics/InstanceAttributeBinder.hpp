#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GlDataType.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class VBOHandle;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Helper used to bind per-instance attribute streams during instanced draws
///
/// `InstanceAttributeBinder` is the parameter type of the
/// `setupFn` callback that
/// `sf::RenderTarget::drawInstancedVertices` and
/// `sf::RenderTarget::drawInstancedIndexedVertices` invoke right
/// before issuing the draw call. Inside the callback, you bind
/// a VBO holding the per-instance data and configure one or
/// more vertex attribute streams that pull from it.
///
/// The binder is non-copyable and non-movable: it must be used
/// only within the scope of the callback that received it.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] InstanceAttributeBinder
{
    ////////////////////////////////////////////////////////////
    InstanceAttributeBinder() = default;

    ////////////////////////////////////////////////////////////
    InstanceAttributeBinder(const InstanceAttributeBinder&)            = delete;
    InstanceAttributeBinder& operator=(const InstanceAttributeBinder&) = delete;

    ////////////////////////////////////////////////////////////
    InstanceAttributeBinder(InstanceAttributeBinder&&)            = delete;
    InstanceAttributeBinder& operator=(InstanceAttributeBinder&&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Bind a VBO that the next `setup` calls will source data from
    ///
    /// \param vboHandle VBO to bind as the active per-instance source
    ///
    ////////////////////////////////////////////////////////////
    void bindVBO(VBOHandle& vboHandle);

    ////////////////////////////////////////////////////////////
    /// \brief Upload `instanceCount * stride` bytes from `data` into the bound VBO
    ///
    /// Convenience method that combines a buffer upload with the
    /// implicit "the next instanced draw will read this many
    /// instances" bookkeeping.
    ///
    /// \param instanceCount Number of instances the data covers
    /// \param data          Pointer to the source bytes
    /// \param stride        Size, in bytes, of one per-instance record
    ///
    ////////////////////////////////////////////////////////////
    void uploadData(base::SizeT instanceCount, const void* data, base::SizeT stride);

    ////////////////////////////////////////////////////////////
    /// \brief Type-safe wrapper around `uploadData` for contiguous arrays
    ///
    /// Equivalent to `uploadData(instanceCount, data, sizeof(T))`.
    ///
    /// \tparam T            Per-instance record type
    /// \param instanceCount Number of instances the array covers
    /// \param data          Pointer to the contiguous per-instance data
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    void uploadContiguousData(const base::SizeT instanceCount, const T* const data)
    {
        uploadData(instanceCount, data, sizeof(T));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Configure a per-instance vertex attribute stream
    ///
    /// Calls `glVertexAttribPointer` and `glVertexAttribDivisor`
    /// (with divisor `1`) to set up an attribute that advances
    /// once per instance.
    ///
    /// \param location    Shader attribute location to bind to
    /// \param size        Number of components (1-4)
    /// \param type        Data type of each component
    /// \param normalized  Whether integer types should be normalized to `[0, 1]` / `[-1, 1]`
    /// \param stride      Byte stride between consecutive instances
    /// \param fieldOffset Byte offset of this attribute within a single instance record
    ///
    ////////////////////////////////////////////////////////////
    void setup(unsigned int location, unsigned int size, GlDataType type, bool normalized, base::SizeT stride, base::SizeT fieldOffset);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::InstanceAttributeBinder
/// \ingroup graphics
///
/// `sf::InstanceAttributeBinder` is the configuration helper
/// passed to the `setupFn` callback of
/// `sf::RenderTarget::drawInstancedVertices` and
/// `sf::RenderTarget::drawInstancedIndexedVertices`. It exposes
/// the minimal API needed to bind a per-instance VBO, upload
/// per-instance data, and declare the vertex attribute streams
/// that will read from it during the upcoming instanced draw
/// call.
///
/// \see `sf::RenderTarget`, `sf::DrawInstancedVerticesSettings`,
///      `sf::DrawInstancedIndexedVerticesSettings`,
///      `sf::VAOHandle`, `sf::VBOHandle`
///
////////////////////////////////////////////////////////////
