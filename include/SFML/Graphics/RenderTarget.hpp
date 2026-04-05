#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawInstancedIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawInstancedVerticesSettings.hpp"
#include "SFML/Graphics/DrawPersistentMappedIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawQuadsSettings.hpp"
#include "SFML/Graphics/DrawTextureSettings.hpp"
#include "SFML/Graphics/DrawVerticesSettings.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Priv/ShapeDataConcept.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/VertexSpan.hpp"

#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class CPUDrawableBatch;
class Font;
class FontFace;
class GlyphMappedText;
class PersistentGPUDrawableBatch;
class Shader;
class Shape;
class Text;
class Texture;
class VAOHandle;
class VertexBuffer;

struct BlendMode;
struct GLElementBufferObject;
struct GLVAOGroup;
struct GLVertexBufferObject;
struct GlyphMapping;
struct GlyphMappedTextData;
struct InstanceAttributeBinder;
struct Sprite;
struct StencilMode;
struct StencilValue;
struct TextData;
struct Transform;
struct Vertex;
struct View;
} // namespace sf


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
    /// \brief Draw a texture to the render target
    ///
    /// The full texture is drawn at position `{0.f, 0.f}` with default origin,
    /// rotation, scale, and color
    ///
    /// \param sprite Texture to draw
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Texture& texture, RenderStates states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw a texture to the render target
    ///
    /// \param sprite Texture to draw
    /// \param params Drawing parameters
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Texture& texture, const DrawTextureSettings& params, RenderStates states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw a sprite object to the render target
    ///
    /// The texture associated with a sprite must be passed while drawing.
    ///
    /// \param sprite Sprite to draw
    /// \param states Render states to use for drawing
    ///
    ////////////////////////////////////////////////////////////
    void draw(const Sprite& sprite, const RenderStates& states = {});

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
    /// \brief Draw a GlyphMappedText
    ///
    ////////////////////////////////////////////////////////////
    void draw(const GlyphMappedText& text, RenderStates states = {});

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
    VertexSpan draw(const Font& font, const TextData& textData, RenderStates states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw text using a glyph mapping (stateless)
    ///
    ////////////////////////////////////////////////////////////
    VertexSpan draw(const FontFace&            fontFace,
                    const GlyphMapping&        glyphMapping,
                    const GlyphMappedTextData& textData,
                    const RenderStates&        states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by an array of vertices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawVertices(const DrawVerticesSettings& settings, const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by an array of indices and vertices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawIndexedVertices(const DrawIndexedVerticesSettings& settings, const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw quads defined by an array of vertices and precomputed quad indices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawQuads(const DrawQuadsSettings& settings, const RenderStates& states = {});

    ////////////////////////////////////////////////////////////
    /// \brief Draw primitives defined by a persistent mapped buffer and indices
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void drawPersistentMappedIndexedVertices(const DrawPersistentMappedIndexedVerticesSettings& settings,
                                             const RenderStates&                                states = {});

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawInstancedVertices(const DrawInstancedVerticesSettings&                           settings,
                               const base::FixedFunction<void(InstanceAttributeBinder&), 64>& setupFn,
                               const RenderStates&                                            states = {});

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void drawInstancedIndexedVertices(const DrawInstancedIndexedVerticesSettings&                    settings,
                                      const base::FixedFunction<void(InstanceAttributeBinder&), 64>& setupFn,
                                      const RenderStates&                                            states = {});

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
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] View computeView() const;

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
    void flushGPUCommands();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void finishGPUCommands();

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <bool TLocked>
    class [[nodiscard]] WithRenderStatesContext
    {
    private:
        ////////////////////////////////////////////////////////////
        RenderTarget* const m_rt;
        const RenderStates  m_states;

    public:
        ////////////////////////////////////////////////////////////
        WithRenderStatesContext(RenderTarget& rt, const RenderStates& states);
        ~WithRenderStatesContext();

        ////////////////////////////////////////////////////////////
        WithRenderStatesContext(const WithRenderStatesContext&)            = delete;
        WithRenderStatesContext& operator=(const WithRenderStatesContext&) = delete;

        ////////////////////////////////////////////////////////////
        WithRenderStatesContext(WithRenderStatesContext&&)            = delete;
        WithRenderStatesContext& operator=(WithRenderStatesContext&&) = delete;

        ////////////////////////////////////////////////////////////
        [[gnu::always_inline]] const WithRenderStatesContext& draw(const auto& drawable) const
        {
            m_rt->draw(drawable, m_states);
            return *this;
        }

        ////////////////////////////////////////////////////////////
        [[gnu::always_inline]] const WithRenderStatesContext& drawAll(const auto&... drawables) const
        {
            (..., draw(drawables));
            return *this;
        }

        ////////////////////////////////////////////////////////////
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        [[gnu::always_inline]] const WithRenderStatesContext& draw(const Texture& texture, const DrawTextureSettings& params) const
        {
            m_rt->draw(texture, params, m_states);
            return *this;
        }

        ////////////////////////////////////////////////////////////
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        [[gnu::always_inline]] const WithRenderStatesContext& draw(const Font& font, const TextData& textData) const
        {
            m_rt->draw(font, textData, m_states);
            return *this;
        }

        ////////////////////////////////////////////////////////////
        // NOLINTNEXTLINE(modernize-use-nodiscard)
        [[gnu::always_inline]] const WithRenderStatesContext& draw(const FontFace&            fontFace,
                                                                   const GlyphMapping&        glyphMapping,
                                                                   const GlyphMappedTextData& textData) const
        {
            m_rt->draw(fontFace, glyphMapping, textData, m_states);
            return *this;
        }
    };

    ////////////////////////////////////////////////////////////
    template <bool>
    friend class WithRenderStatesContext;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] WithRenderStatesContext<false> withRenderStates(const RenderStates& states)
    {
        return {*this, states};
    }

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] WithRenderStatesContext<true> withLockedRenderStates(const RenderStates& states)
    {
        return {*this, states};
    }

protected:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RenderTarget();

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
    /// \brief TODO P1: docs
    ///
    /// no need to call if clear is called
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool prepare();

    ////////////////////////////////////////////////////////////
    /// \brief Immediately draw primitives defined by an array of vertices
    ///
    /// Will result in an OpenGL draw call.
    /// Does not flush any batch in-flight.
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawVertices(const DrawVerticesSettings& settings, const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Immediately draw primitives defined by an array of indices and vertices
    ///
    /// Will result in an OpenGL draw call.
    /// Does not flush any batch in-flight.
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawIndexedVertices(const DrawIndexedVerticesSettings& settings, const RenderStates& states);

    ////////////////////////////////////////////////////////////
    /// \brief Immediately draw primitives defined by a persistent mapped buffer and indices
    ///
    /// Will result in an OpenGL draw call.
    /// Does not flush any batch in-flight.
    ///
    /// \param settings Draw settings
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawPersistentMappedIndexedVertices(const DrawPersistentMappedIndexedVerticesSettings& settings,
                                                      const RenderStates&                                states);

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
                                        base::FixedFunction<void(InstanceAttributeBinder&), 64> setupFn,
                                        const RenderStates&                                     states);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    void immediateDrawInstancedIndexedVertices(const DrawInstancedIndexedVerticesSettings&             settings,
                                               base::FixedFunction<void(InstanceAttributeBinder&), 64> setupFn,
                                               const RenderStates&                                     states);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void flushIfNeeded(const RenderStates& states)
    {
        if (m_isStateLocked)
        {
            SFML_BASE_ASSERT(m_lastRenderStates == states &&
                             "State mutation detected while inside a 'withLockedRenderStates' context!\n If you are "
                             "drawing Text, Shapes, or Sprites, you must explicitly bind their Texture to the context "
                             "upfront.");

            if (m_numAutoBatchVertices >= m_autoBatchVertexThreshold)
                flush();
        }
        else
        {
            if (m_numAutoBatchVertices >= m_autoBatchVertexThreshold || m_lastRenderStates != states)
            {
                flush();
                m_lastRenderStates = states;
            }
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
    ///
    ////////////////////////////////////////////////////////////
    void setupDrawMVP(const Transform& renderStatesTransform, const Transform& viewTransform);

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
        requires(requires { drawVertices({vertices.data(), vertices.size(), type}, states); })
    {
        drawVertices(
            {
                .vertexData    = vertices.data(),
                .vertexCount   = vertices.size(),
                .primitiveType = type,
            },
            states);
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
        drawVertices(
            {
                .vertexData    = vertices,
                .vertexCount   = N,
                .primitiveType = type,
            },
            states);
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

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    template <typename DrawableObjectRange>
    void draw(const DrawableObjectRange& drawableObjectRange, const RenderStates& states = {})
        requires(requires { draw(*drawableObjectRange.begin(), states); })
    {
        for (const auto& drawable : drawableObjectRange)
            draw(drawable, states);
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    DrawStatistics m_currentDrawStats{};                       //!< Statistics for current draw calls
    AutoBatchMode  m_autoBatchMode{AutoBatchMode::GPUStorage}; //!< Enable automatic batching of draw calls
    base::SizeT    m_numAutoBatchVertices{0u};                 //!< Number of vertices in the current autobatch
    base::SizeT    m_autoBatchVertexThreshold{32'768u};        //!< Threshold for batch vertex count
    RenderStates   m_lastRenderStates{};                       //!< Cached render states (autobatching)
    bool           m_isStateLocked{false}; //!< Whether render states are currently bound via `withLockedRenderStates`

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
