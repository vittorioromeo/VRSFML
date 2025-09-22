#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/GlDataType.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/System/Rect.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsSame.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CPUDrawableBatch;

template <typename TBufferObject>
class GLPersistentBuffer;

class Font;
class PersistentGPUDrawableBatch;
class Shader;
class Shape;
class Text;
class Texture;
class VertexBuffer;

struct ArrowShapeData;
struct CircleShapeData;
struct CurvedArrowShapeData;
struct EllipseShapeData;
struct PieSliceShapeData;
struct RectangleShapeData;
struct RingShapeData;
struct RingPieSliceShapeData;
struct RoundedRectangleShapeData;
struct StarShapeData;

struct BlendMode;
struct GLElementBufferObject;
struct GLVAOGroup;
struct GLVertexBufferObject;
struct Sprite;
struct StencilMode;
struct StencilValue;
struct TextData;
struct Transform;
struct Vertex;
struct View;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
concept ShapeDataConcept =                             //
    SFML_BASE_IS_SAME(T, ArrowShapeData) ||            //
    SFML_BASE_IS_SAME(T, CircleShapeData) ||           //
    SFML_BASE_IS_SAME(T, CurvedArrowShapeData) ||      //
    SFML_BASE_IS_SAME(T, EllipseShapeData) ||          //
    SFML_BASE_IS_SAME(T, PieSliceShapeData) ||         //
    SFML_BASE_IS_SAME(T, RectangleShapeData) ||        //
    SFML_BASE_IS_SAME(T, RingShapeData) ||             //
    SFML_BASE_IS_SAME(T, RingPieSliceShapeData) ||     //
    SFML_BASE_IS_SAME(T, RoundedRectangleShapeData) || //
    SFML_BASE_IS_SAME(T, StarShapeData);

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Base class for all render targets (window, texture, etc...)
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API RenderTarget
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Struct to hold the statistics of a render target
    ///
    ////////////////////////////////////////////////////////////
    struct DrawStatistics
    {
        unsigned int drawCalls{0u};     //!< Number of draw calls
        base::SizeT  drawnVertices{0u}; //!< Number of vertices drawn
    };

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~RenderTarget();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget(const RenderTarget&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget& operator=(const RenderTarget&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget(RenderTarget&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget& operator=(RenderTarget&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P0:
    ///
    /// TODO P0: no need to call if clear is called
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool prepare();

    ////////////////////////////////////////////////////////////
    /// \brief Clear the entire target with a single color
    ///
    /// This function is usually called once every frame,
    /// to clear the previous contents of the target.
    ///
    /// \param color Fill color to use to clear the render target
    ///
    ////////////////////////////////////////////////////////////
    void clear(Color color = Color::Black);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the stencil buffer to a specific value
    ///
    /// The specified value is truncated to the bit width of
    /// the current stencil buffer.
    ///
    /// \param stencilValue Stencil value to clear to
    ///
    ////////////////////////////////////////////////////////////
    void clearStencil(StencilValue stencilValue);

    ////////////////////////////////////////////////////////////
    /// \brief Clear the entire target with a single color and stencil value
    ///
    /// The specified stencil value is truncated to the bit
    /// width of the current stencil buffer.
    ///
    /// \param color        Fill color to use to clear the render target
    /// \param stencilValue Stencil value to clear to
    ///
    ////////////////////////////////////////////////////////////
    void clear(Color color, StencilValue stencilValue);

    ////////////////////////////////////////////////////////////
    /// \brief Change the current active view
    ///
    /// The view is like a 2D camera, it controls which part of
    /// the 2D scene is visible, and how it is viewed in the
    /// render target.
    /// The new view will affect everything that is drawn, until
    /// another view is set.
    /// The render target keeps its own copy of the view object,
    /// so it is not necessary to keep the original one alive
    /// after calling this function.
    ///
    /// \param view New view to use
    ///
    /// \see `getView`
    ///
    ////////////////////////////////////////////////////////////
    void setView(const View& view);

    ////////////////////////////////////////////////////////////
    /// \brief Get the view currently in use in the render target
    ///
    /// \return The view object that is currently used
    ///
    /// \see `setView`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const View& getView() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    enum class [[nodiscard]] AutoBatchMode : unsigned char
    {
        Disabled,   //!< Auto-batching is disabled
        CPUStorage, //!< Auto-batching is enabled with CPU storage
        GPUStorage, //!< Auto-batching is enabled with GPU storage (fallback to CPU if GPU storage is not available)
    };

    ////////////////////////////////////////////////////////////
    /// \brief Set the current auto-batching mode
    ///
    /// Auto-batching is a performance optimization that groups
    /// draw calls together to reduce the number of state changes
    /// and improve rendering performance. When enabled, the
    /// render target will automatically batch draw calls
    /// together when possible, reducing the overhead of
    /// individual draw calls.
    ///
    /// \param mode The auto-batching mode to set
    ///
    /// \see `getAutoBatchMode`
    ///
    ////////////////////////////////////////////////////////////
    void setAutoBatchMode(AutoBatchMode mode);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current auto-batching mode
    ///
    /// \return The current auto-batching mode
    ///
    /// \see `setAutoBatchMode`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] AutoBatchMode getAutoBatchMode() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the vertex threshold for auto-batching
    ///
    /// When the current number of vertices in the batch exceeds
    /// this threshold, the batch will be flushed and a new one
    /// will be started. This can help to control the size of
    /// the batches and optimize performance.
    ///
    /// \param threshold The vertex threshold for auto-batching
    ///
    /// \see `getAutoBatchVertexThreshold`
    ///
    ////////////////////////////////////////////////////////////
    void setAutoBatchVertexThreshold(base::SizeT threshold);

    ////////////////////////////////////////////////////////////
    /// \brief Get the vertex threshold for auto-batching
    ///
    /// \return The vertex threshold for auto-batching
    ///
    /// \see `setAutoBatchVertexThreshold`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::SizeT getAutoBatchVertexThreshold() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the viewport of a view, applied to this render target
    ///
    /// The viewport is defined in the view as a ratio, this function
    /// simply applies this ratio to the current dimensions of the
    /// render target to calculate the pixels rectangle that the viewport
    /// actually covers in the target.
    ///
    /// \param view The view for which we want to compute the viewport
    ///
    /// \return Viewport rectangle, expressed in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] IntRect getViewport(const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the scissor rectangle of a view, applied to this render target
    ///
    /// The scissor rectangle is defined in the view as a ratio. This
    /// function simply applies this ratio to the current dimensions
    /// of the render target to calculate the pixels rectangle
    /// that the scissor rectangle actually covers in the target.
    ///
    /// \param view The view for which we want to compute the scissor rectangle
    ///
    /// \return Scissor rectangle, expressed in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] IntRect getScissor(const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from target coordinates to world
    ///        coordinates, using the current view
    ///
    /// This function is an overload of the mapPixelToCoords
    /// function that implicitly uses the current view.
    /// It is equivalent to:
    /// \code
    /// target.mapPixelToCoords(point, target.getView());
    /// \endcode
    ///
    /// \param point Pixel to convert
    ///
    /// \return The converted point, in "world" coordinates
    ///
    /// \see `mapCoordsToPixel`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f mapPixelToCoords(Vec2i point) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from target coordinates to world coordinates
    ///
    /// This function finds the 2D position that matches the
    /// given pixel of the render target. In other words, it does
    /// the inverse of what the graphics card does, to find the
    /// initial position of a rendered pixel.
    ///
    /// Initially, both coordinate systems (world units and target pixels)
    /// match perfectly. But if you define a custom view or resize your
    /// render target, this assertion is not `true` anymore, i.e. a point
    /// located at (10, 50) in your render target may map to the point
    /// (150, 75) in your 2D world -- if the view is translated by (140, 25).
    ///
    /// For render-windows, this function is typically used to find
    /// which point (or object) is located below the mouse cursor.
    ///
    /// This version uses a custom view for calculations, see the other
    /// overload of the function if you want to use the current view of the
    /// render target.
    ///
    /// \param point Pixel to convert
    /// \param view The view to use for converting the point
    ///
    /// \return The converted point, in "world" units
    ///
    /// \see `mapCoordsToPixel`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2f mapPixelToCoords(Vec2i point, const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from world coordinates to target
    ///        coordinates, using the current view
    ///
    /// This function is an overload of the `mapCoordsToPixel`
    /// function that implicitly uses the current view.
    /// It is equivalent to:
    /// \code
    /// target.mapCoordsToPixel(point, target.getView());
    /// \endcode
    ///
    /// \param point Point to convert
    ///
    /// \return The converted point, in target coordinates (pixels)
    ///
    /// \see `mapPixelToCoords`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2i mapCoordsToPixel(Vec2f point) const;

    ////////////////////////////////////////////////////////////
    /// \brief Convert a point from world coordinates to target coordinates
    ///
    /// This function finds the pixel of the render target that matches
    /// the given 2D point. In other words, it goes through the same process
    /// as the graphics card, to compute the final position of a rendered point.
    ///
    /// Initially, both coordinate systems (world units and target pixels)
    /// match perfectly. But if you define a custom view or resize your
    /// render target, this assertion is not `true` anymore, i.e. a point
    /// located at (150, 75) in your 2D world may map to the pixel
    /// (10, 50) of your render target -- if the view is translated by (140, 25).
    ///
    /// This version uses a custom view for calculations, see the other
    /// overload of the function if you want to use the current view of the
    /// render target.
    ///
    /// \param point Point to convert
    /// \param view The view to use for converting the point
    ///
    /// \return The converted point, in target coordinates (pixels)
    ///
    /// \see `mapPixelToCoords`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2i mapCoordsToPixel(Vec2f point, const View& view) const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct TextureDrawParams
    {
        Vec2f position{};      //!< Position of the object in the 2D world
        Vec2f scale{1.f, 1.f}; //!< Scale of the object
        Vec2f origin{};        //!< Origin of translation/rotation/scaling of the object

        // NOLINTNEXTLINE(readability-redundant-member-init)
        Angle rotation{}; //!< Orientation of the object

        FloatRect textureRect{};       //!< Rectangle defining the area of the source texture to display
        Color     color{Color::White}; //!< Color of the sprite
    };

    ////////////////////////////////////////////////////////////
    /// \brief Draw a texture to the render target
    ///
    /// The full texture is drawn at position `{0.f, 0.f}` with default origin,
    /// rotation, scale, and color
    ///
    /// \param sprite Texture to draw
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Texture& texture, RenderStates states = {}); // TODO P1: RenderStatesWithoutTexture?

    ////////////////////////////////////////////////////////////
    /// \brief Draw a texture to the render target
    ///
    /// \param sprite Texture to draw
    /// \param params Drawing parameters
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Texture&           texture,
              const TextureDrawParams& params,
              RenderStates             states = {}); // TODO P1: RenderStatesWithoutTexture?

    ////////////////////////////////////////////////////////////
    /// \brief Draw a sprite object to the render target
    ///
    /// The texture associated with a sprite must be passed while drawing.
    ///
    /// \param sprite Sprite to draw
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Sprite& sprite, const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Draw a shape object to the render target
    ///
    /// A texture associated with a shape can be passed while drawing.
    ///
    /// \param shape  Shape to draw
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Shape& shape, RenderStates states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw a text object to the render target
    ///
    /// \param text Text to draw
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Text& text, RenderStates states = {});

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void draw(const CPUDrawableBatch& drawableBatch, const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void draw(const PersistentGPUDrawableBatch& drawableBatch, RenderStates states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a vertex buffer
    ///
    /// \param vertexBuffer Vertex buffer
    /// \param states       Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const VertexBuffer& vertexBuffer, const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a vertex buffer
    ///
    /// \param vertexBuffer Vertex buffer
    /// \param firstVertex  Index of the first vertex to render
    /// \param vertexCount  Number of vertices to render
    /// \param states       Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const VertexBuffer& vertexBuffer,
              base::SizeT         firstVertex,
              base::SizeT         vertexCount,
              const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw a shape from its relevant data
    ///
    /// \return Span pointing to vertices in the batch
    ///         (WARNING: the span is only valid until the next draw call)
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan draw(const priv::ShapeDataConcept auto& shapeData, const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw a text from a font and its relevant data
    ///
    /// \return Span pointing to vertices in the batch
    ///         (WARNING: the span is only valid until the next draw call)
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan draw(const Font& font, const TextData& textData, RenderStates states = {}); // TODO P1: RenderStatesWithoutTexture?

    ////////////////////////////////////////////////////////////
    /// \brief Settings used to draw an array of vertices
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        const Vertex* vertexData;
        base::SizeT   vertexCount;
        PrimitiveType primitiveType;
        RenderStates  renderStates;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by an array of vertices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawVertices(const DrawVerticesSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Settings used to draw an array of indices and vertices
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        const Vertex*    vertexData;
        base::SizeT      vertexCount;
        const IndexType* indexData;
        base::SizeT      indexCount;
        PrimitiveType    primitiveType;
        RenderStates     renderStates;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by an array of indices and vertices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawIndexedVertices(const DrawIndexedVerticesSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Settings used to draw an array of vertices and precomputed quad indices
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawQuadsSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        const Vertex* vertexData;
        base::SizeT   vertexCount;
        PrimitiveType primitiveType;
        RenderStates  renderStates;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Draw quads defined by an array of vertices and precomputed quad indices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawQuads(const DrawQuadsSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Settings used to draw a persistent mapped buffer and indices
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawPersistentMappedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        const PersistentGPUDrawableBatch& gpuDrawableBatch;
        base::SizeT                       indexCount;
        base::SizeT                       indexOffset;
        base::SizeT                       vertexOffset;
        PrimitiveType                     primitiveType;
        RenderStates                      renderStates;
    };

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a persistent mapped buffer and indices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawPersistentMappedIndexedVertices(const DrawPersistentMappedIndexedVerticesSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct InstanceAttributeBinder;

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

        struct Impl;
        base::InPlacePImpl<Impl, 128> m_impl;
    };

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

        struct Impl;
        base::InPlacePImpl<Impl, 64> m_impl;
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawInstancedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        VAOHandle&    vaoHandle;
        const Vertex* vertexData;
        base::SizeT   vertexCount;
        base::SizeT   instanceCount;
        PrimitiveType primitiveType;
        RenderStates  renderStates;
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] DrawInstancedIndexedVerticesSettings // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        VAOHandle&       vaoHandle;
        const Vertex*    vertexData;
        base::SizeT      vertexCount;
        const IndexType* indexData;
        base::SizeT      indexCount;
        base::SizeT      instanceCount;
        PrimitiveType    primitiveType;
        RenderStates     renderStates;
    };

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawInstancedVertices(const DrawInstancedVerticesSettings&                           settings,
                               const base::FixedFunction<void(InstanceAttributeBinder&), 64>& setupFn);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawInstancedIndexedVertices(const DrawInstancedIndexedVerticesSettings&                    settings,
                                      const base::FixedFunction<void(InstanceAttributeBinder&), 64>& setupFn);

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the rendering region of the target
    ///
    /// \return Size in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Vec2u getSize() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the render target will use sRGB encoding when drawing on it
    ///
    /// \return `true` if the render target use sRGB encoding, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool isSrgb() const;

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the render target for rendering
    ///
    /// This function makes the render target's context current for
    /// future OpenGL rendering operations (so you shouldn't care
    /// about it if you're not doing direct OpenGL stuff).
    /// A render target's context is active only on the current thread,
    /// if you want to make it active on another thread you have
    /// to deactivate it on the previous thread first if it was active.
    /// Only one context can be current in a thread, so if you
    /// want to draw OpenGL geometry to another render target
    /// don't forget to activate it again. Activating a render
    /// target will automatically deactivate the previously active
    /// context (if any).
    ///
    /// \param active `true` to activate, `false` to deactivate
    ///
    /// \return `true` if operation was successful, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool setActive(bool active = true);

    ////////////////////////////////////////////////////////////
    /// \brief Reset the internal OpenGL states so that the target is ready for drawing
    ///
    /// This function can be used when you mix SFML drawing
    /// and direct OpenGL rendering. It makes sure that all OpenGL
    /// states needed by SFML are set, so that subsequent `draw()`
    /// calls will work as expected.
    ///
    /// Example:
    /// \code
    /// // OpenGL code here...
    /// glPushAttrib(...);
    /// window.resetGLStates();
    /// window.draw(...);
    /// window.draw(...);
    /// glPopAttrib(...);
    /// // OpenGL code here...
    /// \endcode
    ///
    ////////////////////////////////////////////////////////////
    void resetGLStates();

    ////////////////////////////////////////////////////////////
    /// \brief Draws the currently autobatched drawables (if any)
    ///
    /// \return Statistics about the draw calls that were made
    ///
    ////////////////////////////////////////////////////////////
    RenderTarget::DrawStatistics flush();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
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
        void bindVBO(VBOHandle& vboHandle);

        ////////////////////////////////////////////////////////////
        void uploadData(base::SizeT instanceCount, const void* data, base::SizeT stride);

        ////////////////////////////////////////////////////////////
        template <typename T>
        void uploadContiguousData(const base::SizeT instanceCount, const T* const data)
        {
            uploadData(instanceCount, data, sizeof(T));
        }

        ////////////////////////////////////////////////////////////
        void setup(unsigned int location,
                   unsigned int size,
                   GlDataType   type,
                   bool         normalized,
                   base::SizeT  stride,
                   base::SizeT  fieldOffset);
    };

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor from view
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderTarget(const View& currentView);

    ////////////////////////////////////////////////////////////
    /// \brief Synchronize the GPU with the CPU (beginning of a frame)
    ///
    /// Only useful when working with persistently mapped buffers.
    ///
    ////////////////////////////////////////////////////////////
    void syncGPUStartFrame();

    ////////////////////////////////////////////////////////////
    /// \brief Synchronize the GPU with the CPU (end of a frame)
    ///
    /// Only useful when working with persistently mapped buffers.
    ///
    ////////////////////////////////////////////////////////////
    void syncGPUEndFrame();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Immediately draw primitives defined by an array of vertices
    ///
    /// Will result in an OpenGL draw call.
    /// Does not flush any batch in-flight.
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawVertices(const DrawVerticesSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Immediately draw primitives defined by an array of indices and vertices
    ///
    /// Will result in an OpenGL draw call.
    /// Does not flush any batch in-flight.
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawIndexedVertices(const DrawIndexedVerticesSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Immediately draw primitives defined by a persistent mapped buffer and indices
    ///
    /// Will result in an OpenGL draw call.
    /// Does not flush any batch in-flight.
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawPersistentMappedIndexedVertices(const DrawPersistentMappedIndexedVerticesSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawDrawableBatch(const CPUDrawableBatch& drawableBatch, RenderStates states);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawInstancedVertices(const DrawInstancedVerticesSettings&                    settings,
                                        base::FixedFunction<void(InstanceAttributeBinder&), 64> setupFn);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawInstancedIndexedVertices(const DrawInstancedIndexedVerticesSettings&             settings,
                                               base::FixedFunction<void(InstanceAttributeBinder&), 64> setupFn);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushIfNeeded(const RenderStates& states)
    {
        if (m_numAutoBatchVertices >= m_autoBatchVertexThreshold || m_lastRenderStates != states)
        {
            flush();
            m_lastRenderStates = states;
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void resetGLStatesImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Apply a view
    ///
    ////////////////////////////////////////////////////////////
    void applyView(const View& view);

    ////////////////////////////////////////////////////////////
    /// \brief Apply a new blending mode
    ///
    /// \param mode Blending mode to apply
    ///
    ////////////////////////////////////////////////////////////
    void applyBlendMode(const BlendMode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Apply a new stencil mode
    ///
    /// \param mode Stencil mode to apply
    ///
    ////////////////////////////////////////////////////////////
    void applyStencilMode(const StencilMode& mode);

    ////////////////////////////////////////////////////////////
    /// \brief Unbind any bound texture
    ///
    ////////////////////////////////////////////////////////////
    void unapplyTexture();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    struct DrawGuard;
    friend DrawGuard;

    ////////////////////////////////////////////////////////////
    /// \brief Setup environment for drawing
    ///
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void setupDraw(const GLVAOGroup& vaoGroup, const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Setup environment for drawing: MVP matrix
    ///
    /// \param renderStatesTransform Transform to use for the MVP matrix
    /// \param viewTransform         View transform to use for the MVP matrix
    /// \param viewChanged           Whether the view has changed
    /// \param shaderChanged         Whether the shader has changed
    ///
    ////////////////////////////////////////////////////////////
    void setupDrawMVP(const Transform& renderStatesTransform, const Transform& viewTransform, bool viewChanged, bool shaderChanged);

    ////////////////////////////////////////////////////////////
    /// \brief Setup environment for drawing: texture
    ///
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void setupDrawTexture(const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Clean up environment after drawing
    ///
    /// \param states Render states used for drawing
    ///
    ////////////////////////////////////////////////////////////
    void cleanupDraw(const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Invoke primitive draw call: non-indexed
    ///
    /// Immediately executes an OpenGL draw call.
    /// This function is not intended to be used directly.
    /// Does not flush any batch in-flight.
    ///
    /// \param type        Type of primitives to draw
    /// \param firstVertex Index of the first vertex to use when drawing
    /// \param vertexCount Number of vertices to use when drawing
    ///
    ////////////////////////////////////////////////////////////
    void invokePrimitiveDrawCall(PrimitiveType type, base::SizeT firstVertex, base::SizeT vertexCount);

    ////////////////////////////////////////////////////////////
    /// \brief Invoke primitive draw call: indexed
    ///
    /// Immediately executes an OpenGL draw call.
    /// This function is not intended to be used directly.
    /// Does not flush any batch in-flight.
    ///
    /// \param type        Type of primitives to draw
    /// \param indexCount  Number of indices to use when drawing
    /// \param indexOffset Offset of the first index to use when drawing
    ///
    ////////////////////////////////////////////////////////////
    void invokePrimitiveDrawCallIndexed(PrimitiveType type, base::SizeT indexCount, base::SizeT indexOffset);

    ////////////////////////////////////////////////////////////
    /// \brief Invoke primitive draw call: indexed with base vertex (not supported on OpenGL ES 3.1)
    ///
    /// Immediately executes an OpenGL draw call.
    /// This function is not intended to be used directly.
    /// Does not flush any batch in-flight.
    ///
    /// \param type         Type of primitives to draw
    /// \param indexCount   Number of indices to use when drawing
    /// \param indexOffset  Offset of the first index to use when drawing
    /// \param vertexOffset Offset of the first vertex to use when drawing
    ///
    ////////////////////////////////////////////////////////////
    void invokePrimitiveDrawCallIndexedBaseVertex(PrimitiveType type,
                                                  base::SizeT   indexCount,
                                                  base::SizeT   indexOffset,
                                                  base::SizeT   vertexOffset);

    ////////////////////////////////////////////////////////////
    /// \brief Invoke primitive draw call: instanced, non-indexed
    ///
    /// Immediately executes an OpenGL draw call.
    /// This function is not intended to be used directly.
    /// Does not flush any batch in-flight.
    ///
    /// \param type          Type of primitives to draw
    /// \param vertexOffset  Offset of the first vertex to use when drawing
    /// \param vertexCount   Number of vertices to use when drawing
    /// \param instanceCount Number of instances to draw
    ///
    ////////////////////////////////////////////////////////////
    void invokeInstancedPrimitiveDrawCall(PrimitiveType type,
                                          base::SizeT   vertexOffset,
                                          base::SizeT   vertexCount,
                                          base::SizeT   instanceCount);

    ////////////////////////////////////////////////////////////
    /// \brief Invoke primitive draw call: instanced, indexed
    ///
    /// Immediately executes an OpenGL draw call.
    /// This function is not intended to be used directly.
    /// Does not flush any batch in-flight.
    ///
    /// \param type          Type of primitives to draw
    /// \param indexOffset   Offset of the first index to use when drawing
    /// \param indexCount    Number of indices to use when drawing
    /// \param instanceCount Number of instances to draw
    ///
    ////////////////////////////////////////////////////////////
    void invokeInstancedPrimitiveDrawCallIndexed(PrimitiveType type,
                                                 base::SizeT   indexOffset,
                                                 base::SizeT   indexCount,
                                                 base::SizeT   instanceCount);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    auto addToAutoBatch(auto&&... xs);

public:
    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a contiguous container of vertices
    ///
    /// \tparam ContiguousVertexRange Type of the contiguous container,
    ///         must support `.data()` and `.size()` operations.
    ///
    /// \param vertices Reference to the contiguous vertex container
    /// \param type     Type of primitives to draw
    /// \param states   Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    template <typename ContiguousVertexRange>
    [[gnu::always_inline]] void draw(const ContiguousVertexRange& vertices, PrimitiveType type, const RenderStates& states = {})
        requires(requires { drawVertices({vertices.data(), vertices.size(), type, states}); })
    {
        drawVertices({
            .vertexData    = vertices.data(),
            .vertexCount   = vertices.size(),
            .primitiveType = type,
            .renderStates  = states,
        });
    }

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a C-style array of vertices
    ///
    /// \param vertices Reference to the C-style vertex array
    /// \param type     Type of primitives to draw
    /// \param states   Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    template <base::SizeT N>
    [[gnu::always_inline]] void draw(const Vertex (&vertices)[N], PrimitiveType type, const RenderStates& states = {})
    {
        drawVertices({
            .vertexData    = vertices,
            .vertexCount   = N,
            .primitiveType = type,
            .renderStates  = states,
        });
    }

    ////////////////////////////////////////////////////////////
    /// \brief Draw a drawable object to the render target
    ///
    /// \tparam DrawableObject Type of the drawable object,
    ///         must support `draw(RenderTarget&, RenderStates&)` method.
    ///
    /// \param drawable Object to draw
    /// \param states   Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    template <typename DrawableObject>
    [[gnu::always_inline]] void draw(const DrawableObject& drawableObject, const RenderStates& states = {})
        requires(requires { drawableObject.draw(*this, states); })
    {
        flushIfNeeded(states);
        drawableObject.draw(*this, states);
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    DrawStatistics m_currentDrawStats{};                       //!< Statistics for current draw calls
    AutoBatchMode  m_autoBatchMode{AutoBatchMode::GPUStorage}; //!< Enable automatic batching of draw calls
    base::SizeT    m_numAutoBatchVertices{0u};                 //!< Number of vertices in the current autobatch
    base::SizeT    m_autoBatchVertexThreshold{32'768u};        //!< Threshold for batch vertex count
    RenderStates   m_lastRenderStates;                         //!< Cached render states (autobatching)

    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 1024> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RenderTarget
/// \ingroup graphics
///
/// `sf::RenderTarget` defines the common behavior of all the
/// 2D render targets usable in the graphics module. It makes
/// it possible to draw 2D entities like sprites, shapes, text
/// without using any OpenGL command directly.
///
/// A `sf::RenderTarget` is also able to use views (`sf::View`),
/// which are a kind of 2D cameras. With views you can globally
/// scroll, rotate or zoom everything that is drawn,
/// without having to transform every single entity. See the
/// documentation of `sf::View` for more details and sample pieces of
/// code about this class.
///
/// On top of that, render targets are still able to render direct
/// OpenGL stuff. It is even possible to mix together OpenGL calls
/// and regular SFML drawing commands. When doing so, make sure that
/// OpenGL states are not messed up by calling `resetGLStates`.
///
/// While render targets are moveable, it is not valid to move them
/// between threads. This will cause your program to crash. The
/// problem boils down to OpenGL being limited with regard to how it
/// works in multithreaded environments. Please ensure you only move
/// render targets within the same thread.
///
/// \see `sf::RenderWindow`, `sf::RenderTexture`, `sf::View`
///
////////////////////////////////////////////////////////////
