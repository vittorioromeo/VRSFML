#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLVAOGroup.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Rect.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/OffsetOf.hpp"
#include "SFML/Base/FastSinCos.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Lround.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/SizeT.hpp"

#include <atomic>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace RenderTargetImpl
{
////////////////////////////////////////////////////////////
// Type alias for a render target or context id
using IdType = unsigned int;


////////////////////////////////////////////////////////////
// Unique identifier, used for identifying RenderTargets when
// tracking the currently active RenderTarget within a given context
constinit std::atomic<IdType> nextUniqueId{1ul};


////////////////////////////////////////////////////////////
// Invalid/null render target or context id value
constexpr IdType invalidId{0ul};


////////////////////////////////////////////////////////////
// Maximum supported number of render targets or contexts
constexpr sf::base::SizeT maxIdCount{256ul};


////////////////////////////////////////////////////////////
// Map to help us detect whether a different RenderTarget has been activated within a single context
constinit std::atomic<IdType> contextRenderTargetMap[maxIdCount]{};


////////////////////////////////////////////////////////////
// Check if a render target with the given ID is active in the current context
[[nodiscard]] bool isActive(const IdType contextId, const IdType id)
{
    SFML_BASE_ASSERT(contextId < maxIdCount);

    const auto renderTargetId = contextRenderTargetMap[contextId].load();
    return (renderTargetId != invalidId) && (renderTargetId == id);
}


////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(fnName, sfEnumType, ...)                                        \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr GLenum fnName(const sfEnumType sfEnumValue) \
    {                                                                                                                 \
        constexpr GLenum glValues[] __VA_ARGS__;                                                                      \
                                                                                                                      \
        SFML_BASE_ASSERT(static_cast<unsigned int>(sfEnumValue) < ::sf::base::getArraySize(glValues));                \
        return glValues[static_cast<unsigned int>(sfEnumValue)];                                                      \
    }


////////////////////////////////////////////////////////////
// Convert an sf::BlendMode::Factor constant to the corresponding OpenGL constant.
SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    factorToGlConstant,
    sf::BlendMode::Factor,
    {GL_ZERO,
     GL_ONE,
     GL_SRC_COLOR,
     GL_ONE_MINUS_SRC_COLOR,
     GL_DST_COLOR,
     GL_ONE_MINUS_DST_COLOR,
     GL_SRC_ALPHA,
     GL_ONE_MINUS_SRC_ALPHA,
     GL_DST_ALPHA,
     GL_ONE_MINUS_DST_ALPHA});


////////////////////////////////////////////////////////////
// Convert an sf::BlendMode::Equation constant to the corresponding OpenGL constant.
SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(equationToGlConstant,
                                              sf::BlendMode::Equation,
                                              {GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX});


////////////////////////////////////////////////////////////
// Convert an UpdateOperation constant to the corresponding OpenGL constant.
SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(stencilOperationToGlConstant,
                                              sf::StencilUpdateOperation,
                                              {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT});


////////////////////////////////////////////////////////////
// Convert a Comparison constant to the corresponding OpenGL constant.
SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    stencilFunctionToGlConstant,
    sf::StencilComparison,
    {GL_NEVER, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS});


////////////////////////////////////////////////////////////
SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    primitiveTypeToOpenGLMode,
    sf::PrimitiveType,
    {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN});


////////////////////////////////////////////////////////////
#undef SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN

////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline sf::IntRect getMultipliedBySizeAndRoundedRect(
    const sf::Vector2u   renderTargetSize,
    const sf::FloatRect& inputRect)
{
    const auto [width, height] = renderTargetSize.toVector2f();

    return sf::Rect<long>({SFML_BASE_MATH_LROUNDF(width * inputRect.position.x),
                           SFML_BASE_MATH_LROUNDF(height * inputRect.position.y)},
                          {SFML_BASE_MATH_LROUNDF(width * inputRect.size.x),
                           SFML_BASE_MATH_LROUNDF(height * inputRect.size.y)})
        .to<sf::IntRect>();
}


////////////////////////////////////////////////////////////
enum : bool
{
#ifdef SFML_OPENGL_ES
    isOpenGLES = true
#else
    isOpenGLES = false
#endif
};


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamToGPU(const unsigned int    bufferId,
                                                             const void* const     data,
                                                             const sf::base::SizeT dataByteCount)
{
    if constexpr (isOpenGLES)
    {
        // On OpenGL ES, the "naive" method seems faster, also named buffers are not supported
        glCheck(glBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), data, GL_STREAM_DRAW));
    }
    else
    {
        // For small batches, the "naive" method also seems faster
        if (dataByteCount < sizeof(sf::Vertex) * 64u)
        {
            glCheck(glNamedBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), nullptr, GL_STREAM_DRAW)); // Must orphan first
            glCheck(glNamedBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), data, GL_STREAM_DRAW));
            return;
        }

        // For larger batches, memcpying into a transient mapped buffer seems faster
        glCheck(glNamedBufferData(bufferId, static_cast<GLsizeiptr>(dataByteCount), nullptr, GL_STREAM_DRAW));

        void* const ptr = glCheck(
            glMapNamedBufferRange(bufferId,
                                  0u,
                                  static_cast<GLsizeiptr>(dataByteCount),
                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_INVALIDATE_RANGE_BIT));

        SFML_BASE_MEMCPY(ptr, data, dataByteCount);

        [[maybe_unused]] const auto rc = glCheck(glUnmapNamedBuffer(bufferId));
        SFML_BASE_ASSERT(rc == GL_TRUE);
    }
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamVerticesToGPU([[maybe_unused]] const unsigned int bufferId,
                                                                     const sf::Vertex*                   vertexData,
                                                                     const sf::base::SizeT               vertexCount)
{
    streamToGPU(isOpenGLES ? GL_ARRAY_BUFFER : bufferId, vertexData, sizeof(sf::Vertex) * vertexCount);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamIndicesToGPU([[maybe_unused]] const unsigned int bufferId,
                                                                    const sf::IndexType*                indexData,
                                                                    const sf::base::SizeT               indexCount)
{
    streamToGPU(isOpenGLES ? GL_ELEMENT_ARRAY_BUFFER : bufferId, indexData, sizeof(sf::IndexType) * indexCount);
}


////////////////////////////////////////////////////////////
void setupVertexAttribPointers()
{
#define SFML_PRIV_OFFSETOF(...) reinterpret_cast<const void*>(SFML_BASE_OFFSETOF(__VA_ARGS__))

    // Hardcoded layout location `0u` for `sf_a_position`
    glCheck(glEnableVertexAttribArray(0u));
    glCheck(glVertexAttribPointer(/*      index */ 0u,
                                  /*       size */ 2,
                                  /*       type */ GL_FLOAT,
                                  /* normalized */ GL_FALSE,
                                  /*     stride */ sizeof(sf::Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(sf::Vertex, position)));

    // Hardcoded layout location `1u` for `sf_a_color`
    glCheck(glEnableVertexAttribArray(1u));
    glCheck(glVertexAttribPointer(/*      index */ 1u,
                                  /*       size */ 4,
                                  /*       type */ GL_UNSIGNED_BYTE,
                                  /* normalized */ GL_TRUE,
                                  /*     stride */ sizeof(sf::Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(sf::Vertex, color)));

    // Hardcoded layout location `2u` for `sf_a_texCoord`
    glCheck(glEnableVertexAttribArray(2u));
    glCheck(glVertexAttribPointer(/*      index */ 2u,
                                  /*       size */ 2,
                                  /*       type */ GL_FLOAT,
                                  /* normalized */ GL_FALSE,
                                  /*     stride */ sizeof(sf::Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(sf::Vertex, texCoords)));

#undef SFML_PRIV_OFFSETOF
}


////////////////////////////////////////////////////////////
constexpr unsigned int precomputedQuadIndices[]{
#include "SFML/Graphics/PrecomputedQuadIndices.inl"
};

} // namespace RenderTargetImpl
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Render states cache
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] StatesCache
{
    bool enable{false};      //!< Is the cache enabled?
    bool glStatesSet{false}; //!< Are our internal GL states set yet?

    bool      viewChanged{false}; //!< Has the current view changed since last draw?
    Transform lastViewTransform;  //!< Cached transform of latest view

    Transform lastRenderStatesTransform; //!< Cached renderstates transform

    bool scissorEnabled{false}; //!< Is scissor testing enabled?
    bool stencilEnabled{false}; //!< Is stencil testing enabled?

    unsigned int lastVaoGroup{0u};          //!< Last bound vertex array object id
    unsigned int lastVaoGroupContextId{0u}; //!< Last bound vertex array object context id

    BlendMode   lastBlendMode{BlendAlpha}; //!< Cached blending mode
    StencilMode lastStencilMode{};         //!< Cached stencil
    base::U64   lastTextureId{0u};         //!< Cached texture

    GLuint lastProgramId{0u}; //!< GL id of the last used shader program
};


////////////////////////////////////////////////////////////
struct RenderTarget::Impl
{
    ////////////////////////////////////////////////////////////
    View                     view;     //!< Current view
    StatesCache              cache{};  //!< Render states cache
    RenderTargetImpl::IdType id{};     //!< Unique number that identifies the render target
    GLVAOGroup               vaoGroup; //!< VAO, VBO, and EBO associated with the render target (non-persistent storage)

    ////////////////////////////////////////////////////////////
    CPUDrawableBatch cpuDrawableBatch;   //!< Internal CPU drawable batch (autobatching) (TODO P0: use GPU?)
    bool             autoBatch = true;   //!< Enable automatic batching of draw calls
    RenderStates     lastRenderStates;   //!< Cached render states (autobatching)
    DrawStatistics   currentDrawStats{}; //!< Statistics for current draw calls
    base::SizeT      autoBatchVertexThreshold{32'768u}; //!< Threshold for batch vertex count

    ////////////////////////////////////////////////////////////
#ifndef SFML_OPENGL_ES
    GLsync persistentBufferFence{};   //!< Fence for persistent buffer synchronization
    bool   persistentWaitDone{false}; //!< Indicates if a sync is needed for persistent buffer
#endif

    ////////////////////////////////////////////////////////////
    explicit Impl(const View& theView) :
    view(theView),
    id(RenderTargetImpl::nextUniqueId.fetch_add(1u, std::memory_order::relaxed))
    {
    }

    ////////////////////////////////////////////////////////////
    void bindGLObjects(const GLVAOGroup& theVAOGroup)
    {
        theVAOGroup.bind();

        cache.lastVaoGroup          = theVAOGroup.getId();
        cache.lastVaoGroupContextId = GraphicsContext::getActiveThreadLocalGlContextId();

        RenderTargetImpl::setupVertexAttribPointers();
    }
};


////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(const View& currentView) : m_impl(currentView)
{
}


////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget() = default;


////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(RenderTarget&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTarget& RenderTarget::operator=(RenderTarget&&) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] bool RenderTarget::clearImpl()
{
    if (!setActive(true))
    {
        priv::err() << "Failed to activate render target in `clearImpl`";
        return false;
    }

    syncGPUStartFrame();

    m_impl->currentDrawStats = {};

    // Unbind texture to fix RenderTexture preventing clear
    unapplyTexture(); // See https://en.sfml-dev.org/forums/index.php?topic=9350

    // Apply the view (scissor testing can affect clearing)
    if (!m_impl->cache.enable || m_impl->cache.viewChanged)
        applyView(m_impl->view);

    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color color)
{
    if (!clearImpl())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clearStencil(const StencilValue stencilValue)
{
    if (!clearImpl())
        return;

    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color color, const StencilValue stencilValue)
{
    if (!clearImpl())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::setView(const View& view)
{
    if (view == m_impl->view)
        return;

    flush();

    m_impl->view              = view;
    m_impl->cache.viewChanged = true;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getView() const
{
    return m_impl->view;
}


////////////////////////////////////////////////////////////
void RenderTarget::setAutoBatchEnabled(const bool enabled)
{
    if (m_impl->autoBatch == enabled)
        return;

    flush();
    m_impl->autoBatch = enabled;
}


////////////////////////////////////////////////////////////
bool RenderTarget::isAutoBatchEnabled() const
{
    return m_impl->autoBatch;
}


////////////////////////////////////////////////////////////
void RenderTarget::setAutoBatchVertexThreshold(const base::SizeT threshold)
{
    if (m_impl->autoBatchVertexThreshold == threshold)
        return;

    flush();
    m_impl->autoBatchVertexThreshold = threshold;
}


////////////////////////////////////////////////////////////
base::SizeT RenderTarget::getAutoBatchVertexThreshold() const
{
    return m_impl->autoBatchVertexThreshold;
}


////////////////////////////////////////////////////////////
IntRect RenderTarget::getViewport(const View& view) const
{
    return RenderTargetImpl::getMultipliedBySizeAndRoundedRect(getSize(), view.viewport);
}


////////////////////////////////////////////////////////////
IntRect RenderTarget::getScissor(const View& view) const
{
    return RenderTargetImpl::getMultipliedBySizeAndRoundedRect(getSize(), view.scissor);
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(const Vector2i point) const
{
    return mapPixelToCoords(point, getView());
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(const Vector2i point, const View& view) const
{
    // First, convert from viewport coordinates to homogeneous coordinates
    const auto     viewport   = getViewport(view).to<FloatRect>();
    const Vector2f normalized = Vector2f(-1.f, 1.f) + Vector2f(2.f, -2.f)
                                                          .componentWiseMul(point.toVector2f() - viewport.position)
                                                          .componentWiseDiv(viewport.size);

    // Then transform by the inverse of the view matrix
    return view.getInverseTransform().transformPoint(normalized);
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f point) const
{
    return mapCoordsToPixel(point, getView());
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(const Vector2f point, const View& view) const
{
    // First, transform the point by the view matrix
    const Vector2f normalized = view.getTransform().transformPoint(point);

    // Then convert to viewport coordinates
    const auto viewport = getViewport(view).to<FloatRect>();
    return ((normalized.componentWiseMul({1.f, -1.f}) + sf::Vector2f{1.f, 1.f})
                .componentWiseMul({0.5f, 0.5f})
                .componentWiseMul(viewport.size) +
            viewport.position)
        .toVector2i();
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Texture& texture, RenderStates states)
{
    states.texture = &texture;

    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);
        m_impl->cpuDrawableBatch.add(Sprite{.textureRect = texture.getRect()});
    }
    else
    {
        Vertex buffer[4];

        appendPreTransformedSpriteVertices(Transform::from(/* position */ {0.f, 0.f},
                                                           /* scale */ {1.f, 1.f},
                                                           /* origin */ {0.f, 0.f}),
                                           texture.getRect(),
                                           Color::White,
                                           buffer);

        draw(buffer, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Texture& texture, const TextureDrawParams& params, RenderStates states)
{
    states.texture = &texture;

    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);

        m_impl->cpuDrawableBatch.add(Sprite{
            .position    = params.position,
            .scale       = params.scale,
            .origin      = params.origin,
            .rotation    = params.rotation,
            .textureRect = (params.textureRect == FloatRect{}) ? texture.getRect() : params.textureRect,
            .color       = params.color,
        });
    }
    else
    {
        const auto [sine, cosine] = base::fastSinCos(params.rotation.wrapUnsigned().asRadians());

        Vertex buffer[4];

        appendPreTransformedSpriteVertices(Transform::from(params.position, params.scale, params.origin, sine, cosine),
                                           (params.textureRect == FloatRect{}) ? texture.getRect() : params.textureRect,
                                           params.color,
                                           buffer);

        draw(buffer, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Sprite& sprite, const RenderStates& states)
{
    SFML_BASE_ASSERT(states.texture != nullptr);

    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);
        m_impl->cpuDrawableBatch.add(sprite);
    }
    else
    {
        Vertex buffer[4];
        appendPreTransformedSpriteVertices(sprite.getTransform(), sprite.textureRect, sprite.color, buffer);
        draw(buffer, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Shape& shape, RenderStates states)
{
    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);
        m_impl->cpuDrawableBatch.add(shape);
    }
    else
    {
        states.transform *= shape.getTransform();

        const auto [fillData, fillSize]       = shape.getFillVertices();
        const auto [outlineData, outlineSize] = shape.getOutlineVertices();

        drawVertices(fillData, fillSize, PrimitiveType::TriangleFan, states);

        if (shape.getOutlineThickness() != 0.f)
            drawVertices(outlineData, outlineSize, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Text& text, RenderStates states)
{
    states.texture = &text.getFont().getTexture();

    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);
        m_impl->cpuDrawableBatch.add(text);
    }
    else
    {
        text.draw(*this, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::drawVertices(const Vertex* vertexData, base::SizeT vertexCount, PrimitiveType type, const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (vertexData == nullptr || vertexCount == 0u || !setActive(true))
        return;

    setupDraw(m_impl->vaoGroup, states);
    m_impl->lastRenderStates = states;

    RenderTargetImpl::streamVerticesToGPU(m_impl->vaoGroup.vbo.getId(), vertexData, vertexCount);

    drawPrimitives(type, 0u, vertexCount);
    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawIndexedVertices(
    const Vertex*       vertexData,
    const base::SizeT   vertexCount,
    const IndexType*    indexData,
    const base::SizeT   indexCount,
    const PrimitiveType type,
    const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (vertexData == nullptr || vertexCount == 0u || indexData == nullptr || indexCount == 0u || !setActive(true))
        return;

    setupDraw(m_impl->vaoGroup, states);
    m_impl->lastRenderStates = states;

    RenderTargetImpl::streamVerticesToGPU(m_impl->vaoGroup.vbo.getId(), vertexData, vertexCount);
    RenderTargetImpl::streamIndicesToGPU(m_impl->vaoGroup.ebo.getId(), indexData, indexCount);

    drawIndexedPrimitives(type, indexCount);
    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawIndexedQuads(const Vertex*       vertexData,
                                    const base::SizeT   vertexCount,
                                    const PrimitiveType type,
                                    const RenderStates& states)
{
    drawIndexedVertices(vertexData, vertexCount, RenderTargetImpl::precomputedQuadIndices, vertexCount / 4u * 6u, type, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawPersistentMappedVertices(const PersistentGPUDrawableBatch& batch,
                                                const base::SizeT                 vertexCount,
                                                const PrimitiveType               type,
                                                const RenderStates&               states)
{
    // Nothing to draw or inactive target
    if (vertexCount == 0u || !setActive(true))
        return;

    setupDraw(*static_cast<const GLVAOGroup*>(batch.m_storage.getVAOGroup()), states);
    m_impl->lastRenderStates = states;

    drawPrimitives(type, 0u, vertexCount);
    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawPersistentMappedIndexedVertices(const PersistentGPUDrawableBatch& batch,
                                                       const base::SizeT                 indexCount,
                                                       const PrimitiveType               type,
                                                       const RenderStates&               states)
{
    // Nothing to draw or inactive target
    if (indexCount == 0u || !setActive(true))
        return;

    setupDraw(*static_cast<const GLVAOGroup*>(batch.m_storage.getVAOGroup()), states);
    m_impl->lastRenderStates = states;

    drawIndexedPrimitives(type, indexCount);
    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const CPUDrawableBatch& drawableBatch, RenderStates states)
{
    if (m_impl->autoBatch && &drawableBatch != &m_impl->cpuDrawableBatch)
        flush();

    states.transform *= drawableBatch.getTransform();

    drawIndexedVertices(drawableBatch.m_storage.vertices.data(),
                        drawableBatch.m_storage.vertices.size(),
                        drawableBatch.m_storage.indices.data(),
                        drawableBatch.m_storage.indices.size(),
                        PrimitiveType::Triangles,
                        states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const PersistentGPUDrawableBatch& drawableBatch, RenderStates states)
{
    if (m_impl->autoBatch)
        flush();

    states.transform *= drawableBatch.getTransform();
    drawPersistentMappedIndexedVertices(drawableBatch, drawableBatch.m_storage.nIndices, PrimitiveType::Triangles, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const VertexBuffer& vertexBuffer, const RenderStates& states)
{
    draw(vertexBuffer, 0, vertexBuffer.getVertexCount(), states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const VertexBuffer& vertexBuffer,
                        const base::SizeT   firstVertex,
                        base::SizeT         vertexCount,
                        const RenderStates& states)
{
    if (m_impl->autoBatch)
        flush();

    // Sanity check
    if (firstVertex > vertexBuffer.getVertexCount())
        return;

    // Clamp vertexCount to something that makes sense
    vertexCount = base::min(vertexCount, vertexBuffer.getVertexCount() - firstVertex);

    // Nothing to draw or inactive target

    if (!vertexCount || !vertexBuffer.getNativeHandle() || !setActive(true))
        return;

    setupDraw(m_impl->vaoGroup, states);
    m_impl->lastRenderStates = states;

    // Bind vertex buffer
    vertexBuffer.bind();

    // Always enable texture coordinates (needed because different buffer is bound)
    RenderTargetImpl::setupVertexAttribPointers();

    drawPrimitives(vertexBuffer.getPrimitiveType(), firstVertex, vertexCount);

    // Unbind vertex buffer
    VertexBuffer::unbind();

    // Needed to restore attrib pointers on regular VBO
    m_impl->bindGLObjects(m_impl->vaoGroup);

    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawShapeData(const auto& shapeData, const RenderStates& states)
{
    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);
        m_impl->cpuDrawableBatch.add(shapeData);
        return;
    }

    SFML_BASE_ASSERT(m_impl->cpuDrawableBatch.isEmpty());

    m_impl->cpuDrawableBatch.add(shapeData);
    draw(m_impl->cpuDrawableBatch, states);
    m_impl->cpuDrawableBatch.clear();
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const CircleShapeData& sdCircle, const RenderStates& states)
{
    drawShapeData(sdCircle, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const EllipseShapeData& sdEllipse, const RenderStates& states)
{
    drawShapeData(sdEllipse, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const RectangleShapeData& sdRectangle, const RenderStates& states)
{
    drawShapeData(sdRectangle, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const RoundedRectangleShapeData& sdRoundedRectangle, const RenderStates& states)
{
    drawShapeData(sdRoundedRectangle, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Font& font, const TextData& textData, RenderStates states)
{
    states.texture = &font.getTexture();

    if (m_impl->autoBatch)
    {
        flushIfNeeded(states);
        m_impl->cpuDrawableBatch.add(font, textData);
        return;
    }

    SFML_BASE_ASSERT(m_impl->cpuDrawableBatch.isEmpty());

    m_impl->cpuDrawableBatch.add(font, textData);
    draw(m_impl->cpuDrawableBatch, states);
    m_impl->cpuDrawableBatch.clear();
}


////////////////////////////////////////////////////////////
bool RenderTarget::isSrgb() const
{
    // By default sRGB encoding is not enabled for an arbitrary RenderTarget
    return false;
}


////////////////////////////////////////////////////////////
bool RenderTarget::setActive(bool active)
{
    // Mark this RenderTarget as active or no longer active in the tracking map
    const RenderTargetImpl::IdType contextId = GraphicsContext::getActiveThreadLocalGlContextId();

    // If this RenderTarget is already active on the current GL context, do nothing
    if (const bool isAlreadyActive = RenderTargetImpl::isActive(contextId, m_impl->id);
        (active && isAlreadyActive) || (!active && !isAlreadyActive))
        return true;

    SFML_BASE_ASSERT(contextId < RenderTargetImpl::maxIdCount);
    std::atomic<RenderTargetImpl::IdType>& renderTargetId = RenderTargetImpl::contextRenderTargetMap[contextId];

    const auto loadedRenderTargetId = renderTargetId.load();

    // Deactivation
    if (!active)
    {
        SFML_BASE_ASSERT(loadedRenderTargetId != RenderTargetImpl::invalidId);
        renderTargetId.store(RenderTargetImpl::invalidId);

        m_impl->cache.enable = false;
        return true;
    }

    // First ever activation or different RT activated on same context
    if (loadedRenderTargetId != m_impl->id)
    {
        renderTargetId.store(m_impl->id);

        if (loadedRenderTargetId == RenderTargetImpl::invalidId)
            m_impl->cache.glStatesSet = false; // First-time activation

        m_impl->cache.enable = false;
        return true;
    }

    // Activation on a different context
    SFML_BASE_ASSERT(loadedRenderTargetId != RenderTargetImpl::invalidId);
    SFML_BASE_ASSERT(loadedRenderTargetId != m_impl->id);
    renderTargetId.store(m_impl->id);

    m_impl->cache.enable = false;
    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::resetGLStates()
{
    flush();
    resetGLStatesImpl();
}


////////////////////////////////////////////////////////////
void RenderTarget::resetGLStatesImpl()
{
// Workaround for states not being properly reset on
// macOS unless a context switch really takes place
#if defined(SFML_SYSTEM_MACOS)
    if (!setActive(false))
        priv::err() << "Failed to set render target inactive";
#endif

    if (!setActive(true))
        return;

#ifdef SFML_DEBUG
    // Make sure that the user didn't leave an unchecked OpenGL error
    if (const GLenum error = glGetError(); error != GL_NO_ERROR)
        priv::err() << "OpenGL error (" << error << ") detected in user code, you should check for errors with glGetError()";
#endif

    // Make sure that the texture unit which is active is the number 0
    glCheck(glActiveTexture(GL_TEXTURE0));

    // Define the default OpenGL states
    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_STENCIL_TEST));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glDisable(GL_SCISSOR_TEST));
    glCheck(glEnable(GL_BLEND));
    glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    m_impl->cache.scissorEnabled = false;
    m_impl->cache.stencilEnabled = false;
    m_impl->cache.lastVaoGroup   = 0u;

    m_impl->cache.glStatesSet = true;

    // Apply the default SFML states
    applyBlendMode(BlendAlpha);
    applyStencilMode(StencilMode{});
    unapplyTexture();

    {
        Shader::unbind();
        m_impl->cache.lastProgramId = 0u;
    }

    VertexBuffer::unbind();

    // Set the default view
    setView(getView());

    m_impl->cache.enable = true;
}


////////////////////////////////////////////////////////////
RenderTarget::DrawStatistics RenderTarget::flush()
{
    draw(m_impl->cpuDrawableBatch, m_impl->lastRenderStates);
    m_impl->cpuDrawableBatch.clear();

    return m_impl->currentDrawStats;
}


////////////////////////////////////////////////////////////
void RenderTarget::syncGPUStartFrame()
{
#ifndef SFML_OPENGL_ES
    if (m_impl->persistentWaitDone || m_impl->persistentBufferFence == GLsync{})
        return;

    glCheck(glClientWaitSync(m_impl->persistentBufferFence, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED));
    glCheck(glDeleteSync(m_impl->persistentBufferFence));

    m_impl->persistentBufferFence = GLsync{};
    m_impl->persistentWaitDone    = true;
#endif
}


////////////////////////////////////////////////////////////
void RenderTarget::syncGPUEndFrame()
{
#ifndef SFML_OPENGL_ES
    if (m_impl->persistentBufferFence != GLsync{})
        glCheck(glDeleteSync(m_impl->persistentBufferFence));

    m_impl->persistentBufferFence = glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
    m_impl->persistentWaitDone    = false;
#endif
}


////////////////////////////////////////////////////////////
void RenderTarget::flushIfNeeded(const RenderStates& states)
{
    if (m_impl->cpuDrawableBatch.getNumVertices() >= m_impl->autoBatchVertexThreshold || m_impl->lastRenderStates != states)
    {
        flush();
        m_impl->lastRenderStates = states;
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::applyView(const View& view)
{
    // Set the viewport
    const IntRect viewport    = getViewport(view);
    const int     viewportTop = static_cast<int>(getSize().y) - (viewport.position.y + viewport.size.y);
    glCheck(glViewport(viewport.position.x, viewportTop, viewport.size.x, viewport.size.y));

    // Set the scissor rectangle and enable/disable scissor testing
    if (view.scissor == FloatRect{{0.f, 0.f}, {1.f, 1.f}})
    {
        if (!m_impl->cache.enable || m_impl->cache.scissorEnabled)
        {
            glCheck(glDisable(GL_SCISSOR_TEST));
            m_impl->cache.scissorEnabled = false;
        }
    }
    else
    {
        const IntRect pixelScissor = getScissor(view);
        const int     scissorTop   = static_cast<int>(getSize().y) - (pixelScissor.position.y + pixelScissor.size.y);
        glCheck(glScissor(pixelScissor.position.x, scissorTop, pixelScissor.size.x, pixelScissor.size.y));

        if (!m_impl->cache.enable || !m_impl->cache.scissorEnabled)
        {
            glCheck(glEnable(GL_SCISSOR_TEST));
            m_impl->cache.scissorEnabled = true;
        }
    }

    m_impl->cache.lastViewTransform = view.getTransform();
    m_impl->cache.viewChanged       = false;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyBlendMode(const BlendMode& mode)
{
    using RenderTargetImpl::equationToGlConstant;
    using RenderTargetImpl::factorToGlConstant;

    glCheck(glBlendFuncSeparate(factorToGlConstant(mode.colorSrcFactor),
                                factorToGlConstant(mode.colorDstFactor),
                                factorToGlConstant(mode.alphaSrcFactor),
                                factorToGlConstant(mode.alphaDstFactor)));

    glCheck(glBlendEquationSeparate(equationToGlConstant(mode.colorEquation), equationToGlConstant(mode.alphaEquation)));

    m_impl->cache.lastBlendMode = mode;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyStencilMode(const StencilMode& mode)
{
    using RenderTargetImpl::stencilFunctionToGlConstant;
    using RenderTargetImpl::stencilOperationToGlConstant;

    m_impl->cache.lastStencilMode = mode;

    // Fast path if we have a default (disabled) stencil mode
    if (mode == StencilMode{})
    {
        if (!m_impl->cache.enable || m_impl->cache.stencilEnabled)
        {
            glCheck(glDisable(GL_STENCIL_TEST));
            glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

            m_impl->cache.stencilEnabled = false;
        }

        return;
    }

    // Apply the stencil mode
    if (!m_impl->cache.enable || !m_impl->cache.stencilEnabled)
        glCheck(glEnable(GL_STENCIL_TEST));

    glCheck(glStencilOp(GL_KEEP,
                        stencilOperationToGlConstant(mode.stencilUpdateOperation),
                        stencilOperationToGlConstant(mode.stencilUpdateOperation)));

    glCheck(glStencilFunc(stencilFunctionToGlConstant(mode.stencilComparison),
                          static_cast<int>(mode.stencilReference.value),
                          mode.stencilMask.value));

    m_impl->cache.stencilEnabled = true;
}


////////////////////////////////////////////////////////////
void RenderTarget::unapplyTexture()
{
    Texture::unbind();
    m_impl->cache.lastTextureId = 0ul;
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDraw(const GLVAOGroup& vaoGroup, const RenderStates& states)
{
    // GL_FRAMEBUFFER_SRGB is not available on OpenGL ES
    // If a framebuffer supports sRGB, it will always be enabled on OpenGL ES
#ifndef SFML_OPENGL_ES
    // Enable or disable sRGB encoding
    // This is needed for drivers that do not check the format of the surface drawn to before applying sRGB conversion
    if (!m_impl->cache.enable)
    {
        if (isSrgb())
            glCheck(glEnable(GL_FRAMEBUFFER_SRGB));
        else
            glCheck(glDisable(GL_FRAMEBUFFER_SRGB));
    }
#endif

    // First set the persistent OpenGL states if it's the very first call
    if (!m_impl->cache.glStatesSet)
        resetGLStatesImpl();

    // Bind GL objects
    {
        const bool mustRebindVAO = m_impl->cache.lastVaoGroup == 0u || m_impl->cache.lastVaoGroup != vaoGroup.getId() ||
                                   m_impl->cache.lastVaoGroupContextId == 0u ||
                                   m_impl->cache.lastVaoGroupContextId != GraphicsContext::getActiveThreadLocalGlContextId();

        if (!m_impl->cache.enable || mustRebindVAO)
            m_impl->bindGLObjects(vaoGroup);
    }

    // Select shader to be used
    const Shader& usedShader = states.shader != nullptr ? *states.shader : GraphicsContext::getInstalledBuiltInShader();

    // Update shader
    const auto usedNativeHandle = usedShader.getNativeHandle();
    const bool shaderChanged    = !m_impl->cache.enable || m_impl->cache.lastProgramId != usedNativeHandle;

    if (shaderChanged)
    {
        usedShader.bind();
        m_impl->cache.lastProgramId = usedNativeHandle;
    }

    // Apply the view
    const bool viewChanged = !m_impl->cache.enable || m_impl->cache.viewChanged;
    if (viewChanged)
        applyView(m_impl->view);

    // Set the model-view-projection matrix
    setupDrawMVP(states.transform, m_impl->cache.lastViewTransform, viewChanged, shaderChanged);

    // Apply the blend mode
    if (!m_impl->cache.enable || (states.blendMode != m_impl->cache.lastBlendMode))
        applyBlendMode(states.blendMode);

    // Apply the stencil mode
    if (!m_impl->cache.enable || (states.stencilMode != m_impl->cache.lastStencilMode))
        applyStencilMode(states.stencilMode);

    // Mask the color buffer off if necessary
    if (states.stencilMode.stencilOnly)
        glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    // Deal with texture
    setupDrawTexture(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDrawMVP(const Transform& renderStatesTransform,
                                const Transform& viewTransform,
                                const bool       viewChanged,
                                const bool       shaderChanged)
{
    // If there's no difference from the cached one, exit early
    if (!shaderChanged &&
        (m_impl->cache.enable && !viewChanged && renderStatesTransform == m_impl->cache.lastRenderStatesTransform))
        return;

    // Compute the final draw transform
    const Transform trsfm = viewTransform * /* model-view matrix */ renderStatesTransform;

    // Update the cached transform
    m_impl->cache.lastRenderStatesTransform = renderStatesTransform;

    // clang-format off
    const float transformMatrixBuffer[]{trsfm.a00, trsfm.a10, 0.f, 0.f,
                                        trsfm.a01, trsfm.a11, 0.f, 0.f,
                                        0.f,       0.f,       1.f, 0.f,
                                        trsfm.a02, trsfm.a12, 0.f, 1.f};
    // clang-format on

    // Upload uniform data to GPU (hardcoded layout location `0u` for `sf_u_mvpMatrix`)
    glCheck(glUniformMatrix4fv(/* location */ 0u, /* count */ 1, /* transpose */ GL_FALSE, transformMatrixBuffer));
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDrawTexture(const RenderStates& states)
{
    // Select texture to be used
    const Texture& usedTexture = states.texture != nullptr ? *states.texture
                                                           : GraphicsContext::getInstalledBuiltInWhiteDotTexture();

    // If the texture is an FBO attachment, always rebind it in order to inform the OpenGL driver that we
    // want changes made to it in other contexts to be visible here as well. This saves us from having to
    // call `glFlush()` in `RenderTextureImplFBO` which can be quite costly
    //
    // See: https://www.khronos.org/opengl/wiki/Memory_Model

    // Should the texture be bound?
    const bool mustApplyTexture = !m_impl->cache.enable || usedTexture.m_fboAttachment ||
                                  usedTexture.m_cacheId != m_impl->cache.lastTextureId;

    // If not, exit early
    if (!mustApplyTexture)
        return;

    // Bind the texture
    usedTexture.bind();

    // Update basic cache texture stuff
    m_impl->cache.lastTextureId = usedTexture.m_cacheId;
}


////////////////////////////////////////////////////////////
void RenderTarget::drawPrimitives(const PrimitiveType type, const base::SizeT firstVertex, const base::SizeT vertexCount)
{
    ++m_impl->currentDrawStats.drawCalls;

    glCheck(glDrawArrays(/*     primitive type */ RenderTargetImpl::primitiveTypeToOpenGLMode(type),
                         /* first vertex index */ static_cast<GLint>(firstVertex),
                         /*       vertex count */ static_cast<GLsizei>(vertexCount)));
}


////////////////////////////////////////////////////////////
void RenderTarget::drawIndexedPrimitives(const PrimitiveType type, const base::SizeT indexCount)
{
    ++m_impl->currentDrawStats.drawCalls;

    static_assert(SFML_BASE_IS_SAME(IndexType, unsigned int));

    glCheck(glDrawElements(/* primitive type */ RenderTargetImpl::primitiveTypeToOpenGLMode(type),
                           /*    index count */ static_cast<GLsizei>(indexCount),
                           /*     index type */ GL_UNSIGNED_INT,
                           /*   index offset */ nullptr));
}


////////////////////////////////////////////////////////////
void RenderTarget::cleanupDraw(const RenderStates& states)
{
    // Do not unbind the shader here, as it could be reused for the next draw call.

    // If the texture we used to draw belonged to a RenderTexture, then forcibly unbind that texture.
    // This prevents a bug where some drivers do not clear RenderTextures properly.
    if (states.texture != nullptr && states.texture->m_fboAttachment)
        unapplyTexture();

    // Mask the color buffer back on if necessary
    if (states.stencilMode.stencilOnly)
        glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    // Re-enable the cache at the end of the draw if it was disabled
    m_impl->cache.enable = true;
}

} // namespace sf

////////////////////////////////////////////////////////////
// Render states caching strategies
//
// * View
//   If SetView was called since last draw, the projection
//   matrix is updated. We don't need more, the view doesn't
//   change frequently.
//
// * Blending mode
//   Since it overloads the == operator, we can easily check
//   whether any of the 6 blending components changed and,
//   thus, whether we need to update the blend mode.
//
// * Texture
//   Storing the pointer or OpenGL ID of the last used texture
//   is not enough; if the sf::Texture instance is destroyed,
//   both the pointer and the OpenGL ID might be recycled in
//   a new texture instance. We need to use our own unique
//   identifier system to ensure consistent caching.
//
// * Shader
//   Shaders are very hard to optimize, because they have
//   parameters that can be hard (if not impossible) to track,
//   like matrices or textures. The only optimization that we
//   do is that we avoid setting a null shader if there was
//   already none for the previous draw.
//
////////////////////////////////////////////////////////////
