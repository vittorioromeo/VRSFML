#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Transformable.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Font;
class RenderTarget;
class Shape;
class Text;

struct ArrowShapeData;
struct CircleShapeData;
struct CurvedArrowShapeData;
struct EllipseShapeData;
struct PieSliceShapeData;
struct RectangleShapeData;
struct RingPieSliceShapeData;
struct RingShapeData;
struct RoundedRectangleShapeData;
struct Sprite;
struct StarShapeData;
struct TextData;
struct Transform;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Internal storage strategy for `DrawableBatchImpl` using CPU-side memory
///
/// `CPUStorage` manages vertex and index data in standard `sf::base::Vector`
/// containers residing in CPU memory. This storage is suitable when
/// batch data is frequently updated or when GPU resources are constrained.
/// Data is typically uploaded to the GPU on each draw call.
///
/// This struct is used as a template parameter for `sf::priv::DrawableBatchImpl`.
///
////////////////////////////////////////////////////////////
struct CPUStorage
{
    ////////////////////////////////////////////////////////////
    /// \brief Clears all vertex and index data from the storage
    ///
    /// Resets the internal vertex and index vectors to be empty.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void clear()
    {
        vertices.clear();
        indices.clear();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Reserves capacity for more vertices and returns a pointer to the new region
    ///
    /// Ensures that the internal vertex vector has enough capacity to
    /// store at least `count` additional vertices.
    ///
    /// \param count Number of additional vertices to reserve space for
    /// \return Pointer to the beginning of the reserved (but uncommitted) vertex region
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] Vertex* reserveMoreVertices(const base::SizeT count)
    {
        return vertices.reserveMore(count);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Reserves capacity for more indices and returns a pointer to the new region
    ///
    /// Ensures that the internal index vector has enough capacity to
    /// store at least `count` additional indices.
    ///
    /// \param count Number of additional indices to reserve space for
    /// \return Pointer to the beginning of the reserved (but uncommitted) index region
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] IndexType* reserveMoreIndices(const base::SizeT count)
    {
        return indices.reserveMore(count);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Commits a number of previously reserved vertices
    ///
    /// Updates the size of the vertex vector to include `count`
    /// more vertices, which are assumed to have been written into
    /// the memory obtained from a prior `reserveMoreVertices` call.
    /// This is an unsafe operation if `count` exceeds reserved capacity.
    ///
    /// \param count Number of vertices to commit
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void commitMoreVertices(const base::SizeT count) noexcept
    {
        vertices.unsafeSetSize(vertices.size() + count);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Commits a number of previously reserved indices
    ///
    /// Updates the size of the index vector to include `count`
    /// more indices, which are assumed to have been written into
    /// the memory obtained from a prior `reserveMoreIndices` call.
    /// This is an unsafe operation if `count` exceeds reserved capacity.
    ///
    /// \param count Number of indices to commit
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void commitMoreIndices(const base::SizeT count) noexcept
    {
        indices.unsafeSetSize(indices.size() + count);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets the current number of committed vertices
    ///
    /// \return The total number of vertices currently stored
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const noexcept
    {
        return static_cast<IndexType>(vertices.size());
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets the current number of committed indices
    ///
    /// \return The total number of indices currently stored
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const noexcept
    {
        return static_cast<IndexType>(indices.size());
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::Vector<Vertex>    vertices; //!< CPU buffer for vertices
    base::Vector<IndexType> indices;  //!< CPU buffer for indices
};

////////////////////////////////////////////////////////////
/// \brief Internal storage strategy for `DrawableBatchImpl` using persistent GPU memory
///
/// `PersistentGPUStorage` manages vertex and index data in GPU buffers
/// that are mapped to CPU address space for direct writing. This strategy
/// is designed for performance when batch data changes, as it can avoid
/// per-draw GPU uploads by writing directly to mapped memory.
/// The `flushXXX` member functions are used to signal the GPU that writes are complete.
///
/// This struct is used as a template parameter for `sf::priv::DrawableBatchImpl`.
/// It involves more complex GPU memory management, including Vertex Array Objects (VAOs).
///
////////////////////////////////////////////////////////////
struct PersistentGPUStorage
{
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Initializes the GPU storage, potentially allocating GPU resources.
    ///
    ////////////////////////////////////////////////////////////
    explicit PersistentGPUStorage();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Releases GPU resources managed by this storage object.
    ///
    ////////////////////////////////////////////////////////////
    ~PersistentGPUStorage();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    PersistentGPUStorage(const PersistentGPUStorage&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    PersistentGPUStorage& operator=(const PersistentGPUStorage&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    PersistentGPUStorage(PersistentGPUStorage&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    PersistentGPUStorage& operator=(PersistentGPUStorage&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Clears the number of active vertices and indices
    ///
    /// Resets the counts of vertices and indices to zero.
    /// Does not deallocate or overwrite GPU memory, but marks it as unused.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        nVertices = nIndices = 0u;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Reserves capacity for more vertices and returns a pointer to the mapped region
    ///
    /// Ensures that the GPU vertex buffer has enough capacity and returns
    /// a CPU-writable pointer to a region where `count` new vertices can be written.
    /// May involve buffer resizing or re-mapping.
    ///
    /// \param count Number of additional vertices to reserve space for
    /// \return Pointer to the beginning of the CPU-writable vertex region on the GPU buffer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vertex* reserveMoreVertices(base::SizeT count);

    ////////////////////////////////////////////////////////////
    /// \brief Reserves capacity for more indices and returns a pointer to the mapped region
    ///
    /// Ensures that the GPU index buffer has enough capacity and returns
    /// a CPU-writable pointer to a region where `count` new indices can be written.
    /// May involve buffer resizing or re-mapping.
    ///
    /// \param count Number of additional indices to reserve space for
    /// \return Pointer to the beginning of the CPU-writable index region on the GPU buffer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] IndexType* reserveMoreIndices(base::SizeT count);

    ////////////////////////////////////////////////////////////
    /// \brief Commits a number of previously reserved vertices
    ///
    /// Updates the count of active vertices. The actual data is assumed
    /// to have been written directly to the GPU-mapped memory obtained
    /// from `reserveMoreVertices`.
    ///
    /// \param count Number of vertices to commit
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commitMoreVertices(const base::SizeT count) noexcept
    {
        nVertices += static_cast<IndexType>(count);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Commits a number of previously reserved indices
    ///
    /// Updates the count of active indices. The actual data is assumed
    /// to have been written directly to the GPU-mapped memory obtained
    /// from `reserveMoreIndices`.
    ///
    /// \param count Number of indices to commit
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void commitMoreIndices(const base::SizeT count) noexcept
    {
        nIndices += static_cast<IndexType>(count);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets the current number of active vertices in the GPU buffer
    ///
    /// \return The total number of vertices currently marked as active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const noexcept
    {
        return nVertices;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets the current number of active indices in the GPU buffer
    ///
    /// \return The total number of indices currently marked as active
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const noexcept
    {
        return nIndices;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets a pointer to the Vertex Array Object (VAO) group
    /// \warning Internal SFML detail, subject to change.
    ///
    /// Returns a pointer representing the VAO configuration for this storage.
    /// This is used by the rendering system to set up vertex attribute pointers.
    ///
    /// \return Const void pointer to the VAO group information.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const void* getVAOGroup() const;

    ////////////////////////////////////////////////////////////
    /// \brief Flushes a range of vertex writes to the GPU
    /// \warning Advanced use: ensure correct synchronization and range.
    ///
    /// Signals to the GPU that a specific range of the vertex buffer,
    /// previously written to via a mapped pointer, has been modified
    /// and needs to be synchronized.
    ///
    /// \param count Number of vertices in the range to flush
    /// \param offset Offset (in number of vertices) from the beginning of the buffer
    ///
    ////////////////////////////////////////////////////////////
    void flushVertexWritesToGPU(base::SizeT count, base::SizeT offset) const;

    ////////////////////////////////////////////////////////////
    /// \brief Flushes a range of index writes to the GPU
    /// \warning Advanced use: ensure correct synchronization and range.
    ///
    /// Signals to the GPU that a specific range of the index buffer,
    /// previously written to via a mapped pointer, has been modified
    /// and needs to be synchronized.
    ///
    /// \param count Number of indices in the range to flush
    /// \param offset Offset (in number of indices) from the beginning of the buffer
    ///
    ////////////////////////////////////////////////////////////
    void flushIndexWritesToGPU(base::SizeT count, base::SizeT offset) const;

    // TODO P0: docs, refactor
    void startSync();
    void endSync();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> impl; //!< Implementation details

    IndexType nVertices{}; //!< Number of "active" vertices in the buffer
    IndexType nIndices{};  //!< Number of "active" indices in the buffer
};

////////////////////////////////////////////////////////////
/// \brief Base class template for drawable batches
/// \ingroup graphics
///
/// `DrawableBatchImpl` is a templated base class that provides the core
/// functionality for batching multiple drawable objects into a single
/// draw call. It is designed to improve rendering performance by reducing
/// the number of state changes and draw commands sent to the GPU.
///
/// This class manages a collection of vertices and indices, using a
/// storage strategy defined by the `TStorage` template parameter
/// (e.g., `sf::priv::CPUStorage` or `sf::priv::PersistentGPUStorage`).
/// It inherits from `sf::Transformable` to allow the entire batch
/// to be transformed (translated, rotated, scaled).
///
/// Users typically interact with derived classes like `sf::CPUDrawableBatch`
/// or `sf::PersistentGPUDrawableBatch` rather than this template directly.
///
/// \tparam TStorage The storage strategy for vertex and index data
///
/// \see sf::CPUDrawableBatch, sf::PersistentGPUDrawableBatch
///
////////////////////////////////////////////////////////////
template <typename TStorage>
class [[nodiscard]] SFML_GRAPHICS_API DrawableBatchImpl : public Transformable
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor
    ///
    /// Initializes the drawable batch, forwarding any arguments
    /// to the constructor of the underlying storage object (`TStorage`).
    ///
    /// \param storageArgs Arguments to pass to the `TStorage` constructor
    ///
    ////////////////////////////////////////////////////////////
    template <typename... TStorageArgs>
    explicit DrawableBatchImpl(TStorageArgs&&... storageArgs) : m_storage(SFML_BASE_FORWARD(storageArgs)...)
    {
    }

    ////////////////////////////////////////////////////////////
    /// \brief Reserves space for a given number of triangles
    ///
    /// Pre-allocates memory in the internal storage for the specified
    /// number of triangles (3 vertices and 3 indices per triangle).
    /// This can help avoid reallocations when adding many triangles.
    ///
    /// \param triangleCount Number of triangles to reserve space for
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reserveTriangles(const base::SizeT triangleCount)
    {
        (void)m_storage.reserveMoreIndices(3u * triangleCount);
        (void)m_storage.reserveMoreVertices(3u * triangleCount);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Reserves space for a given number of quads
    ///
    /// Pre-allocates memory in the internal storage for the specified
    /// number of quads (4 vertices and 6 indices per quad, typically
    /// rendered as two triangles). This can help avoid reallocations
    /// when adding many quads.
    ///
    /// \param quadCount Number of quads to reserve space for
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reserveQuads(const base::SizeT quadCount)
    {
        (void)m_storage.reserveMoreIndices(6u * quadCount);
        (void)m_storage.reserveMoreVertices(4u * quadCount);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Adds raw vertex data to the batch
    ///
    /// Appends vertices to the batch. The `type` parameter currently
    /// only supports `sf::PrimitiveType::Triangles`, `sf::PrimitiveType::TriangleFan`,
    /// `sf::PrimitiveType::TriangleFan` and `sf::PrimitiveType::TriangleStrip`.
    /// Other types may lead to undefined behavior or be ignored.
    ///
    /// \param vertexData Pointer to the array of vertices
    /// \param vertexCount Number of vertices in the array
    /// \param type Primitive type
    ///
    /// \warning Only supports triangle, fan, or strip, primitives.
    ///
    ////////////////////////////////////////////////////////////
    void add(const Vertex* vertexData, base::SizeT vertexCount, PrimitiveType type);

    ////////////////////////////////////////////////////////////
    /// \brief Adds raw indexed vertex data to the batch
    ///
    /// Appends indexed vertices to the batch. The `type` parameter currently
    /// only supports `sf::PrimitiveType::Triangles`, `sf::PrimitiveType::TriangleFan`,
    /// `sf::PrimitiveType::TriangleFan` and `sf::PrimitiveType::TriangleStrip`.
    /// Other types may lead to undefined behavior or be ignored.
    ///
    /// Indices are adjusted to be relative to the start of the newly added vertices.
    ///
    /// \param vertexData Pointer to the array of vertices
    /// \param vertexCount Number of vertices in the array
    /// \param indexData Pointer to the array of indices
    /// \param indexCount Number of indices in the array
    /// \param type Primitive type
    ///
    /// \warning Only supports triangle, fan, or strip, primitives.
    ///
    ////////////////////////////////////////////////////////////
    void add(const Vertex*    vertexData,
             base::SizeT      vertexCount,
             const IndexType* indexData,
             base::SizeT      indexCount,
             PrimitiveType    type);

    ////////////////////////////////////////////////////////////
    /// \brief Adds an `sf::Sprite` to the batch
    ///
    /// Extracts vertex data from the sprite and appends it to the batch.
    ///
    /// \param sprite The sprite to add
    ///
    ////////////////////////////////////////////////////////////
    void add(const Sprite& sprite);

    ////////////////////////////////////////////////////////////
    /// \brief Adds an `sf::Shape` to the batch
    ///
    /// Extracts vertex data from the shape (both fill and outline)
    /// and appends it to the batch.
    ///
    /// \param shape The shape to add
    ///
    ////////////////////////////////////////////////////////////
    void add(const Shape& shape);

    ////////////////////////////////////////////////////////////
    /// \brief Adds an `sf::Text` object to the batch
    ///
    /// Extracts vertex data from the text object (including fill and outline)
    /// and appends it to the batch.
    ///
    /// \param text The text object to add
    ///
    ////////////////////////////////////////////////////////////
    void add(const Text& text);

    // TODO P0: should return two vertex spans, one for fill and one for outline

    ////////////////////////////////////////////////////////////
    /// \brief Adds an arrow shape defined by `sf::ArrowShapeData` to the batch
    ///
    /// Generates vertices for an arrow based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdArrow Data defining the arrow shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const ArrowShapeData& sdArrow);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a circle shape defined by `sf::CircleShapeData` to the batch
    ///
    /// Generates vertices for a circle based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdCircle Data defining the circle shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const CircleShapeData& sdCircle);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a curved arrow shape defined by `sf::CurvedArrowShapeData` to the batch
    ///
    /// Generates vertices for a curved arrow based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdCurvedArrow Data defining the curved arrow shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const CurvedArrowShapeData& sdCurvedArrow);

    ////////////////////////////////////////////////////////////
    /// \brief Adds an ellipse shape defined by `sf::EllipseShapeData` to the batch
    ///
    /// Generates vertices for an ellipse based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdEllipse Data defining the ellipse shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const EllipseShapeData& sdEllipse);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a pie slice shape defined by `sf::PieSliceShapeData` to the batch
    ///
    /// Generates vertices for a pie slice based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdPieSlice Data defining the pie slice shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const PieSliceShapeData& sdPieSlice);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a rectangle shape defined by `sf::RectangleShapeData` to the batch
    ///
    /// Generates vertices for a rectangle based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdRectangle Data defining the rectangle shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RectangleShapeData& sdRectangle);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a ring shape defined by `sf::RingShapeData` to the batch
    ///
    /// Generates vertices for a ring based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdRing Data defining the ring shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RingShapeData& sdRing);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a ring pie slice shape defined by `sf::RingPieSliceShapeData` to the batch
    ///
    /// Generates vertices for a ring pie slice based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdRingPieSlice Data defining the ring pie slice shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RingPieSliceShapeData& sdRingPieSlice);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a rounded rectangle shape defined by `sf::RoundedRectangleShapeData` to the batch
    ///
    /// Generates vertices for a rounded rectangle based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdRoundedRectangle Data defining the rounded rectangle shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const RoundedRectangleShapeData& sdRoundedRectangle);

    ////////////////////////////////////////////////////////////
    /// \brief Adds a star shape defined by `sf::StarShapeData` to the batch
    ///
    /// Generates vertices for a star based on the provided data
    /// and appends them to the batch.
    ///
    /// \param sdStarShape Data defining the star shape
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const StarShapeData& sdStarShape);

    ////////////////////////////////////////////////////////////
    /// \brief Adds text geometry defined by `sf::Font` and `sf::TextData` to the batch
    ///
    /// Generates vertices for text based on the provided font and text data
    /// (which includes string, character size, style, fill/outline colors)
    /// and appends them to the batch.
    ///
    /// \param font The font to use for generating text geometry
    /// \param textData Data defining the text to render
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan add(const Font& font, const TextData& textData);

    ////////////////////////////////////////////////////////////
    /// \brief Clears all geometry from the batch
    ///
    /// Removes all vertices and indices from the batch, making it empty.
    /// This calls the `clear` method of the underlying storage.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear()
    {
        m_storage.clear();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Checks if the batch is empty
    ///
    /// \return `true` if the batch contains no vertices or indices, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] bool isEmpty() const noexcept
    {
        return m_storage.getNumVertices() == 0u && m_storage.getNumIndices() == 0u;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets the total number of vertices in the batch
    ///
    /// \return The current number of vertices stored in the batch
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumVertices() const noexcept
    {
        return m_storage.getNumVertices();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Gets the total number of indices in the batch
    ///
    /// \return The current number of indices stored in the batch
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] IndexType getNumIndices() const noexcept
    {
        return m_storage.getNumIndices();
    }

private:
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    /// \brief Generates and adds vertices for a shape resembling a triangle fan
    ///
    /// This utility function is used internally to create geometry for shapes
    /// that can be represented as a central point connected to a series of
    /// outer points (like circles, regular polygons, pie slices).
    ///
    /// \param nPoints Number of points on the shape's perimeter (excluding center)
    /// \param descriptor Shape-specific data (e.g., `CircleShapeData`)
    /// \param pointFn A function that takes an index and returns the coordinate of an outer point
    /// \param centerOffset Offset for the central point of the fan, relative to the shape's origin
    ///
    /// \return A `VertexSpan` referring to the added vertices.
    ///
    /// \warning The returned span is invalidated after the next call to `add` or batch flush.
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan drawTriangleFanShapeFromPoints(base::SizeT nPoints,
                                              const auto& descriptor,
                                              auto&&      pointFn,
                                              Vec2f       centerOffset = {});

    ////////////////////////////////////////////////////////////
    /// \brief Adds vertices for a shape's fill to the batch
    ///
    /// Transforms and appends pre-calculated fill vertices.
    ///
    /// \param transform The transform to apply to the vertices
    /// \param data Pointer to the fill vertex data
    /// \param size Number of fill vertices
    ///
    ////////////////////////////////////////////////////////////
    void addShapeFill(const Transform& transform, const Vertex* data, base::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief Adds vertices for a shape's outline to the batch
    ///
    /// Transforms and appends pre-calculated outline vertices.
    ///
    /// \param transform The transform to apply to the vertices
    /// \param data Pointer to the outline vertex data
    /// \param size Number of outline vertices
    ///
    ////////////////////////////////////////////////////////////
    void addShapeOutline(const Transform& transform, const Vertex* data, base::SizeT size);

protected:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    TStorage m_storage;
};

////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////
extern template class DrawableBatchImpl<CPUStorage>;
extern template class DrawableBatchImpl<PersistentGPUStorage>;

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief A drawable batch that stores vertex data in CPU memory
/// \ingroup graphics
///
/// `CPUDrawableBatch` is a specialization of `DrawableBatchImpl` that
/// uses `sf::priv::CPUStorage`. This means all vertex and index data
/// is stored in system memory (RAM).
///
/// Example:
/// \code
/// sf::CPUDrawableBatch batch;
/// batch.add(sf::Sprite{/* ... */});
/// batch.add(sf::TextData{/* ... */});
/// batch.add(sf::CircleShape{/* ... */});
/// // ... add more drawables
///
/// window.draw(batch); // Data uploaded to GPU here
/// \endcode
///
/// \see sf::PersistentGPUDrawableBatch, sf::priv::DrawableBatchImpl, sf::priv::CPUStorage
///
////////////////////////////////////////////////////////////
class CPUDrawableBatch : public priv::DrawableBatchImpl<priv::CPUStorage>
{
    using DrawableBatchImpl<priv::CPUStorage>::DrawableBatchImpl;
};

////////////////////////////////////////////////////////////
/// \brief A drawable batch that stores vertex data in persistent GPU memory
/// \ingroup graphics
///
/// `PersistentGPUDrawableBatch` is a specialization of `DrawableBatchImpl`
/// that uses `sf::priv::PersistentGPUStorage`. This means vertex and index
/// data is stored in GPU memory that is persistently mapped to the
/// application's address space.
///
/// Example:
/// \code
/// sf::PersistentGPUDrawableBatch batch;
/// batch.add(sf::Sprite{/* ... */});
/// batch.add(sf::TextData{/* ... */});
/// batch.add(sf::CircleShape{/* ... */});
/// // ... add more drawables
///
/// window.draw(batch);
/// \endcode
///
/// \see sf::CPUDrawableBatch, sf::priv::DrawableBatchImpl, sf::priv::PersistentGPUStorage
///
////////////////////////////////////////////////////////////
class PersistentGPUDrawableBatch : public priv::DrawableBatchImpl<priv::PersistentGPUStorage>
{
    using DrawableBatchImpl<priv::PersistentGPUStorage>::DrawableBatchImpl;

public:
    ////////////////////////////////////////////////////////////
    /// \brief Flushes a range of vertex writes to the GPU
    /// \warning Advanced use: ensure correct synchronization and range.
    ///
    /// Signals to the GPU that a specific range of the vertex buffer,
    /// previously written to via a mapped pointer (e.g., by modifying
    /// vertices returned in a `VertexSpan`), has been modified and its
    /// changes need to be made visible to the GPU.
    ///
    /// This method is specific to `PersistentGPUDrawableBatch` due to its
    /// direct GPU memory mapping strategy.
    ///
    /// \param count Number of vertices in the range to flush, starting from `offset`.
    /// \param offset Offset (in number of vertices) from the beginning of the batch's
    ///               vertex buffer where the modified region starts.
    ///
    /// \see sf::priv::PersistentGPUStorage::flushVertexWritesToGPU
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushVertexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
    {
        m_storage.flushVertexWritesToGPU(count, offset);
    }

    ////////////////////////////////////////////////////////////
    /// \brief Flushes a range of index writes to the GPU
    /// \warning Advanced use: ensure correct synchronization and range.
    ///
    /// Signals to the GPU that a specific range of the index buffer,
    /// previously written to via a mapped pointer, has been modified
    /// and its changes need to be made visible to the GPU.
    ///
    /// This method is specific to `PersistentGPUDrawableBatch`.
    ///
    /// \param count Number of indices in the range to flush, starting from `offset`.
    /// \param offset Offset (in number of indices) from the beginning of the batch's
    ///               index buffer where the modified region starts.
    ///
    /// \see sf::priv::PersistentGPUStorage::flushIndexWritesToGPU
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushIndexWritesToGPU(const base::SizeT count, const base::SizeT offset) const
    {
        m_storage.flushIndexWritesToGPU(count, offset);
    }

    // TODO P0: docs, refactor
    void startSync()
    {
        m_storage.startSync();
    }

    // TODO P0: docs, refactor
    void endSync()
    {
        m_storage.endSync();
    }
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::DrawableBatch
/// \ingroup graphics
///
/// `sf::DrawableBatch` refers to a concept of batching drawables for performance,
/// realized through concrete classes like `sf::CPUDrawableBatch` and
/// `sf::PersistentGPUDrawableBatch`.
///
/// Batching draw calls is a common optimization technique in graphics programming.
/// Instead of drawing each object (sprite, shape, text) individually, which
/// can lead to many separate commands sent to the graphics card, objects
/// are collected into a single "batch". This batch is then drawn with one
/// (or very few) commands, reducing CPU overhead and often improving GPU efficiency.
///
/// SFML provides two main types of drawable batches:
///
/// - `sf::CPUDrawableBatch`: Stores vertex data in CPU memory. Data is typically
///   uploaded to the GPU when the batch is drawn. Simpler to manage for
///   highly dynamic data. This is the only supported batch type for OpenGL ES.
///
/// - `sf::PersistentGPUDrawableBatch`: Stores vertex data in persistently mapped
///   GPU memory. Can be faster for large or less frequently updated batches
///   as it allows direct modification of GPU data.
///
/// Both batch types inherit from `sf::Transformable`, allowing the entire
/// group of batched objects to be transformed as a single unit.
///
/// Usage example (using `sf::CPUDrawableBatch`):
/// \code
/// sf::CPUDrawableBatch batch;
///
/// sf::CircleShape circle(50.f);
/// circle.position = {100, 100};
/// circle.setFillColor(sf::Color::Green);
/// batch.add(circle);
///
/// // The batch itself can be transformed
/// batch.move({5, 5});
/// batch.setRotation(10); // Rotates all contained items around the batch's origin
///
/// // Draw the entire batch with one call to sf::RenderTarget
/// window.draw(batch);
/// \endcode
///
/// Choose the batch type based on your specific needs for update frequency
/// and performance characteristics.
///
/// \see sf::CPUDrawableBatch, sf::PersistentGPUDrawableBatch
///
////////////////////////////////////////////////////////////
