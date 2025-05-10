#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/PrimitiveType.hpp"

#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class RenderTarget;
struct RenderStates;
struct Vertex;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Vertex buffer storage for one or more 2D primitives
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API VertexBuffer
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Usage specifiers
    ///
    /// If data is going to be updated once or more every frame,
    /// set the usage to Stream. If data is going to be set once
    /// and used for a long time without being modified, set the
    /// usage to Static. For everything else Dynamic should be a
    /// good compromise.
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] Usage
    {
        Stream,  //!< Constantly changing data
        Dynamic, //!< Occasionally changing data
        Static   //!< Rarely changing data
    };

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates an empty vertex buffer.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit VertexBuffer();

    ////////////////////////////////////////////////////////////
    /// \brief Construct a `VertexBuffer` with a specific `PrimitiveType`
    ///
    /// Creates an empty vertex buffer and sets its primitive type to \p type.
    ///
    /// \param type Type of primitive
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit VertexBuffer(PrimitiveType type);

    ////////////////////////////////////////////////////////////
    /// \brief Construct a `VertexBuffer` with a specific usage specifier
    ///
    /// Creates an empty vertex buffer and sets its usage to \p usage.
    ///
    /// \param usage Usage specifier
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit VertexBuffer(Usage usage);

    ////////////////////////////////////////////////////////////
    /// \brief Construct a `VertexBuffer` with a specific `PrimitiveType` and usage specifier
    ///
    /// Creates an empty vertex buffer and sets its primitive type
    /// to \p type and usage to \p usage.
    ///
    /// \param type  Type of primitive
    /// \param usage Usage specifier
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit VertexBuffer(PrimitiveType type, Usage usage);

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    /// \param rhs instance to copy
    ///
    ////////////////////////////////////////////////////////////
    VertexBuffer(const VertexBuffer& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~VertexBuffer();

    ////////////////////////////////////////////////////////////
    /// \brief Create the vertex buffer
    ///
    /// Creates the vertex buffer and allocates enough graphics
    /// memory to hold `vertexCount` vertices. Any previously
    /// allocated memory is freed in the process.
    ///
    /// In order to deallocate previously allocated memory pass 0
    /// as `vertexCount`. Don't forget to recreate with a non-zero
    /// value when graphics memory should be allocated again.
    ///
    /// \param vertexCount Number of vertices worth of memory to allocate
    ///
    /// \return `true` if creation was successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool create(base::SizeT vertexCount);

    ////////////////////////////////////////////////////////////
    /// \brief Return the vertex count
    ///
    /// \return Number of vertices in the vertex buffer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT getVertexCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the whole buffer from an array of vertices
    ///
    /// The vertex array is assumed to have the same size as
    /// the created buffer.
    ///
    /// No additional check is performed on the size of the vertex
    /// array. Passing invalid arguments will lead to undefined
    /// behavior.
    ///
    /// This function does nothing if `vertices` is null or if the
    /// buffer was not previously created.
    ///
    /// \param vertices Array of vertices to copy to the buffer
    ///
    /// \return `true` if the update was successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool update(const Vertex* vertices);

    ////////////////////////////////////////////////////////////
    /// \brief Update a part of the buffer from an array of vertices
    ///
    /// `offset` is specified as the number of vertices to skip
    /// from the beginning of the buffer.
    ///
    /// If `offset` is 0 and `vertexCount` is equal to the size of
    /// the currently created buffer, its whole contents are replaced.
    ///
    /// If `offset` is 0 and `vertexCount` is greater than the
    /// size of the currently created buffer, a new buffer is created
    /// containing the vertex data.
    ///
    /// If `offset` is 0 and `vertexCount` is less than the size of
    /// the currently created buffer, only the corresponding region
    /// is updated.
    ///
    /// If `offset` is not 0 and `offset` + `vertexCount` is greater
    /// than the size of the currently created buffer, the update fails.
    ///
    /// No additional check is performed on the size of the vertex
    /// array. Passing invalid arguments will lead to undefined
    /// behavior.
    ///
    /// \param vertices    Array of vertices to copy to the buffer
    /// \param vertexCount Number of vertices to copy
    /// \param offset      Offset in the buffer to copy to
    ///
    /// \return `true` if the update was successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool update(const Vertex* vertices, base::SizeT vertexCount, unsigned int offset);

    ////////////////////////////////////////////////////////////
    /// \brief Copy the contents of another buffer into this buffer
    ///
    /// \param vertexBuffer Vertex buffer whose contents to copy into this vertex buffer
    ///
    /// \return `true` if the copy was successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool update(const VertexBuffer& vertexBuffer) const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of assignment operator
    ///
    /// \param rhs Instance to assign
    ///
    /// \return Reference to self
    ///
    ////////////////////////////////////////////////////////////
    VertexBuffer& operator=(const VertexBuffer& rhs);

    ////////////////////////////////////////////////////////////
    /// \brief Swap the contents of this vertex buffer with those of another
    ///
    /// \param right Instance to swap with
    ///
    ////////////////////////////////////////////////////////////
    void swap(VertexBuffer& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the underlying OpenGL handle of the vertex buffer.
    ///
    /// You shouldn't need to use this function, unless you have
    /// very specific stuff to implement that SFML doesn't support,
    /// or implement a temporary workaround until a bug is fixed.
    ///
    /// \return OpenGL handle of the vertex buffer or 0 if not yet created
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getNativeHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the type of primitives to draw
    ///
    /// This function defines how the vertices must be interpreted
    /// when it's time to draw them.
    ///
    /// The default primitive type is `sf::PrimitiveType::Points`.
    ///
    /// \param type Type of primitive
    ///
    ////////////////////////////////////////////////////////////
    void setPrimitiveType(PrimitiveType type);

    ////////////////////////////////////////////////////////////
    /// \brief Get the type of primitives drawn by the vertex buffer
    ///
    /// \return Primitive type
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] PrimitiveType getPrimitiveType() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the usage specifier of this vertex buffer
    ///
    /// This function provides a hint about how this vertex buffer is
    /// going to be used in terms of data update frequency.
    ///
    /// After changing the usage specifier, the vertex buffer has
    /// to be updated with new data for the usage specifier to
    /// take effect.
    ///
    /// The default usage type is `sf::VertexBuffer::Usage::Stream`.
    ///
    /// \param usage Usage specifier
    ///
    ////////////////////////////////////////////////////////////
    void setUsage(Usage usage);

    ////////////////////////////////////////////////////////////
    /// \brief Get the usage specifier of this vertex buffer
    ///
    /// \return Usage specifier
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Usage getUsage() const;

    ////////////////////////////////////////////////////////////
    /// \brief Draw the vertex buffer to a render target
    ///
    /// \param target Render target to draw to
    /// \param states Current render states
    ///
    ////////////////////////////////////////////////////////////
    void draw(RenderTarget& target, RenderStates states) const;

    ////////////////////////////////////////////////////////////
    /// \brief Bind a vertex buffer for rendering
    ///
    /// This function is not part of the graphics API, it mustn't be
    /// used when drawing SFML entities. It must be used only if you
    /// mix `sf::VertexBuffer` with OpenGL code.
    ///
    /// \code
    /// sf::VertexBuffer vb1, vb2;
    /// ...
    /// sf::VertexBuffer::bind(&vb1);
    /// // draw OpenGL stuff that use vb1...
    /// sf::VertexBuffer::bind(&vb2);
    /// // draw OpenGL stuff that use vb2...
    /// sf::VertexBuffer::bind(nullptr);
    /// // draw OpenGL stuff that use no vertex buffer...
    /// \endcode
    ///
    /// \param vertexBuffer Pointer to the vertex buffer to bind, can be null to use no vertex buffer
    ///
    ////////////////////////////////////////////////////////////
    void bind() const;

    ////////////////////////////////////////////////////////////
    /// \brief Unbind any bound vertex buffer
    ///
    ////////////////////////////////////////////////////////////
    static void unbind();

private:
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int  m_buffer{};                             //!< Internal buffer identifier
    base::SizeT   m_size{};                               //!< Size in Vertices of the currently allocated buffer
    PrimitiveType m_primitiveType{PrimitiveType::Points}; //!< Type of primitives to draw
    Usage         m_usage{Usage::Stream};                 //!< How this vertex buffer is to be used
};

////////////////////////////////////////////////////////////
/// \brief Swap the contents of one vertex buffer with those of another
///
/// \param lhs First instance to swap
/// \param rhs Second instance to swap
///
////////////////////////////////////////////////////////////
SFML_GRAPHICS_API void swap(VertexBuffer& lhs, VertexBuffer& rhs) noexcept;

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::VertexBuffer
/// \ingroup graphics
///
/// `sf::VertexBuffer` is a simple wrapper around a dynamic
/// buffer of vertices and a primitives type.
///
/// The vertex data is stored in graphics memory.
///
/// In situations where a large amount of vertex data would
/// have to be transferred from system memory to graphics memory
/// every frame, using `sf::VertexBuffer` can help. By using a
/// `sf::VertexBuffer`, data that has not been changed between frames
/// does not have to be re-transferred from system to graphics
/// memory as would be the case with a `std::vector<sf::Vertex>`.
/// If data transfer is a bottleneck, this can lead to performance gains.
///
/// Using `sf::VertexBuffer`, the user also has the ability to only modify
/// a portion of the buffer in graphics memory. This way, a large buffer
/// can be allocated at the start of the application and only the
/// applicable portions of it need to be updated during the course of
/// the application. This allows the user to take full control of data
/// transfers between system and graphics memory if they need to.
///
/// In special cases, the user can make use of multiple threads to update
/// vertex data in multiple distinct regions of the buffer simultaneously.
/// This might make sense when e.g. the position of multiple objects has to
/// be recalculated very frequently. The computation load can be spread
/// across multiple threads as long as there are no other data dependencies.
///
/// Simultaneous updates to the vertex buffer are not guaranteed to be
/// carried out by the driver in any specific order. Updating the same
/// region of the buffer from multiple threads will not cause undefined
/// behavior, however the final state of the buffer will be unpredictable.
///
/// Simultaneous updates of distinct non-overlapping regions of the buffer
/// are also not guaranteed to complete in a specific order. However, in
/// this case the user can make sure to synchronize the writer threads at
/// well-defined points in their code. The driver will make sure that all
/// pending data transfers complete before the vertex buffer is sourced
/// by the rendering pipeline.
///
/// It inherits `sf::Drawable`, but unlike other drawables it
/// is not transformable.
///
/// Example:
/// \code
/// sf::Vertex vertices[15];
/// ...
/// sf::VertexBuffer triangles(sf::PrimitiveType::Triangles);
/// triangles.create(15);
/// triangles.update(vertices);
/// ...
/// window.draw(triangles);
/// \endcode
///
/// \see `sf::Vertex`
///
////////////////////////////////////////////////////////////
