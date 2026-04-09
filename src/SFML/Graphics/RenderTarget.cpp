// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/RenderTarget.hpp"

#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/DrawIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawInstancedIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawInstancedVerticesSettings.hpp"
#include "SFML/Graphics/DrawPersistentMappedIndexedVerticesSettings.hpp"
#include "SFML/Graphics/DrawQuadsSettings.hpp"
#include "SFML/Graphics/DrawTextureSettings.hpp"
#include "SFML/Graphics/DrawVerticesSettings.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/DrawableBatchUtils.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/GlyphMappedText.hpp"
#include "SFML/Graphics/GlyphMappedTextData.hpp"
#include "SFML/Graphics/GlyphMapping.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/IndexType.hpp"
#include "SFML/Graphics/InstanceAttributeBinder.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Priv/EnumToGlEnumConversions.hpp"
#include "SFML/Graphics/Priv/ShapeDataConcept.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/VAOHandle.hpp"
#include "SFML/Graphics/VBOHandle.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "SFML/Graphics/VertexSpan.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLVAOGroup.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2Base.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Array.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/OffsetOf.hpp"
#include "SFML/Base/FixedFunction.hpp"
#include "SFML/Base/GetArraySize.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SinCosLookup.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Span.hpp"

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
constinit sf::base::Array<std::atomic<IdType>, maxIdCount> contextRenderTargetMap{};


////////////////////////////////////////////////////////////
// Check if a render target with the given ID is active in the current context
[[nodiscard]] bool isActive(const IdType contextId, const IdType id)
{
    SFML_BASE_ASSERT(contextId < maxIdCount);

    const auto renderTargetId = contextRenderTargetMap[contextId].load();
    return (renderTargetId != invalidId) && (renderTargetId == id);
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamVerticesToGPU(sf::base::Span<const sf::Vertex> vertexSpan)
{
    glCheck(glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(sizeof(sf::Vertex) * vertexSpan.size()),
                         vertexSpan.data(),
                         GL_STREAM_DRAW));
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamIndicesToGPU(sf::base::Span<const sf::IndexType> indexSpan)
{
    glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(sizeof(sf::IndexType) * indexSpan.size()),
                         indexSpan.data(),
                         GL_STREAM_DRAW));
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


#ifndef SFML_OPENGL_ES
////////////////////////////////////////////////////////////
enum : sf::base::SizeT
{
    maxGPUAutoBatchFramesInFlight = 3u //!< Number of frames in flight for GPU autobatching
};

////////////////////////////////////////////////////////////
struct [[nodiscard]] PersistentGPUAutoBatchState
{
    sf::PersistentGPUDrawableBatch batch;            //!< Internal GPU autobatch
    GLsync                         fence{};          //!< Fences for GPU autobatching
    sf::base::SizeT                indexOffset{0u};  //!< Index offset for GPU autobatching
    sf::base::SizeT                vertexOffset{0u}; //!< Vertex offset for GPU autobatching
};
#endif


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] inline constexpr bool isPrimitiveTypeSupportedByBatchStorage(
    const sf::PrimitiveType type) noexcept
{
    return type == sf::PrimitiveType::Triangles || type == sf::PrimitiveType::TriangleStrip ||
           type == sf::PrimitiveType::TriangleFan;
}


#ifndef SFML_OPENGL_ES
////////////////////////////////////////////////////////////
[[nodiscard]] GLsync makeFence()
{
    GLsync fenceToCreate = glCheck(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

    if (fenceToCreate == nullptr) [[unlikely]]
    {
        sf::priv::err() << "FATAL ERROR: Error creating fence sync object";
        sf::base::abort();
    }

    return fenceToCreate;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool waitOnFence(GLsync& fenceToWaitOn)
{
    if (!fenceToWaitOn) // No need to wait
        return false;

    const GLenum waitResult = glCheck(glClientWaitSync(fenceToWaitOn, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED));

    if (waitResult == GL_WAIT_FAILED) [[unlikely]]
    {
        sf::priv::err() << "FATAL ERROR: Error waiting on GPU fence";
        sf::base::abort();
    }

    if (waitResult == GL_TIMEOUT_EXPIRED) [[unlikely]]
    {
        sf::priv::err() << "FATAL ERROR: Fence wait timed out";
        sf::base::abort();
    }

    // Delete the fence now that we're done with it
    glCheck(glDeleteSync(fenceToWaitOn));
    fenceToWaitOn = nullptr;

    return true;
}
#endif

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

    View lastView; //!< Cached latest view

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
struct [[nodiscard]] RenderTarget::Impl
{
    ////////////////////////////////////////////////////////////
    StatesCache              cache{};  //!< Render states cache
    RenderTargetImpl::IdType id{};     //!< Unique number that identifies the render target
    GLVAOGroup               vaoGroup; //!< Associated VAO, VBO, and EBO (non-persistent storage)

    ////////////////////////////////////////////////////////////
    CPUDrawableBatch cpuAutoBatch; //!< Internal CPU autobatch

#ifndef SFML_OPENGL_ES
    RenderTargetImpl::PersistentGPUAutoBatchState gpuAutoBatchStates[RenderTargetImpl::maxGPUAutoBatchFramesInFlight]{};
    sf::base::SizeT currentGPUAutoBatchIndex{0u}; //!< Cycles `0`, `1`, ..., `maxGPUAutoBatchFramesInFlight - 1`

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] auto& currentGPUAutoBatchState() noexcept
    {
        return gpuAutoBatchStates[currentGPUAutoBatchIndex];
    }
#endif

    ////////////////////////////////////////////////////////////
    explicit Impl() : id(RenderTargetImpl::nextUniqueId.fetch_add(1u, std::memory_order::relaxed))
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
auto RenderTarget::addToAutoBatch(auto&&... xs)
{
    SFML_BASE_ASSERT(m_autoBatchMode != AutoBatchMode::Disabled);

    const auto addImpl = [&](auto& batch) SFML_BASE_LAMBDA_ALWAYS_INLINE
    {
        const auto prevVertices = batch.getNumVertices();
        SFML_BASE_SCOPE_GUARD({ m_numAutoBatchVertices += batch.getNumVertices() - prevVertices; });

        return batch.add(SFML_BASE_FORWARD(xs)...);
    };

#ifdef SFML_OPENGL_ES
    return addImpl(m_impl->cpuAutoBatch);
#else
    if (m_autoBatchMode == AutoBatchMode::CPUStorage)
        return addImpl(m_impl->cpuAutoBatch);

    SFML_BASE_ASSERT(m_autoBatchMode == AutoBatchMode::GPUStorage);
    return addImpl(m_impl->currentGPUAutoBatchState().batch);
#endif
}


////////////////////////////////////////////////////////////
RenderTarget::RenderTarget() = default;


////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget()                                  = default;
RenderTarget::RenderTarget(RenderTarget&&) noexcept            = default;
RenderTarget& RenderTarget::operator=(RenderTarget&&) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] bool RenderTarget::prepare()
{
    if (!setActive(true))
    {
        priv::err() << "Failed to activate render target in `prepare`";
        return false;
    }

    syncGPUStartFrame();

    m_currentDrawStats = {};

    // Unbind texture to fix RenderTexture preventing clear
    unapplyTexture(); // See https://en.sfml-dev.org/forums/index.php?topic=9350

    // Disable scissor so `clear` always affects the whole target.
    // The next `setupDraw` will automatically re-enable it if the view demands it.
    if (!m_impl->cache.enable || m_impl->cache.scissorEnabled)
    {
        glCheck(glDisable(GL_SCISSOR_TEST));
        m_impl->cache.scissorEnabled = false;
    }


    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color color)
{
    if (!prepare())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clearStencil(const StencilValue stencilValue)
{
    if (!prepare())
        return;

    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color color, const StencilValue stencilValue)
{
    if (!prepare())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::setAutoBatchMode(const AutoBatchMode mode)
{
    if (m_autoBatchMode == mode)
        return;

    flush();
    m_autoBatchMode = mode;
}


////////////////////////////////////////////////////////////
RenderTarget::AutoBatchMode RenderTarget::getAutoBatchMode() const
{
    return m_autoBatchMode;
}


////////////////////////////////////////////////////////////
void RenderTarget::setAutoBatchVertexThreshold(const base::SizeT threshold)
{
    if (m_autoBatchVertexThreshold == threshold)
        return;

    flush();
    m_autoBatchVertexThreshold = threshold;
}


////////////////////////////////////////////////////////////
base::SizeT RenderTarget::getAutoBatchVertexThreshold() const
{
    return m_autoBatchVertexThreshold;
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Texture& texture, RenderStates states)
{
    states.texture = &texture;

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(Sprite{.textureRect = texture.getRect()});
    }
    else
    {
        Vertex buffer[4];
        DrawableBatchUtils::appendPreTransformedSpriteQuadVertices(Transform{}, texture.getRect(), Color::White, buffer);
        draw(buffer, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Texture& texture, const DrawTextureSettings& params, RenderStates states)
{
    states.texture = &texture;

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);

        addToAutoBatch(Sprite{
            .position    = params.position,
            .scale       = params.scale,
            .origin      = params.origin,
            .rotation    = params.rotation,
            .textureRect = (params.textureRect == Rect2f{}) ? texture.getRect() : params.textureRect,
            .color       = params.color,
        });
    }
    else
    {
        const auto [sine, cosine] = base::sinCosLookup(params.rotation.asRadians());

        Vertex buffer[4];

        DrawableBatchUtils::appendPreTransformedSpriteQuadVertices(Transform::fromPositionScaleOriginSinCos(params.position,
                                                                                                            params.scale,
                                                                                                            params.origin,
                                                                                                            sine,
                                                                                                            cosine),
                                                                   (params.textureRect == Rect2f{}) ? texture.getRect()
                                                                                                    : params.textureRect,
                                                                   params.color,
                                                                   buffer);

        draw(buffer, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Sprite& sprite, const RenderStates& states)
{
    SFML_BASE_ASSERT(states.texture != nullptr);

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(sprite);
    }
    else
    {
        Vertex buffer[4];
        DrawableBatchUtils::appendPreTransformedSpriteQuadVertices(sprite.getTransform(), sprite.textureRect, sprite.color, buffer);
        draw(buffer, PrimitiveType::TriangleStrip, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Shape& shape, RenderStates states)
{
    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(shape);
    }
    else
    {
        states.transform *= shape.getTransform();

        immediateDrawVertices(
            {
                .vertexSpan    = shape.getFillVertices(),
                .primitiveType = PrimitiveType::TriangleFan,
            },
            states);

        if (shape.getOutlineThickness() != 0.f)
            immediateDrawVertices(
                {
                    .vertexSpan    = shape.getOutlineVertices(),
                    .primitiveType = PrimitiveType::TriangleStrip,
                },
                states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Text& text, RenderStates states)
{
    states.texture = &text.getTexture();

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(text);
    }
    else
    {
        text.draw(*this, states);
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const GlyphMappedText& text, RenderStates states)
{
    states.texture = &text.getTexture();

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(text);
    }
    else
    {
        text.draw(*this, states);
    }
}


////////////////////////////////////////////////////////////
struct [[nodiscard]] RenderTarget::DrawGuard
{
    RenderTarget& renderTarget;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] explicit DrawGuard(RenderTarget&       theRenderTarget,
                                                         const RenderStates& theRenderStates,
                                                         const GLVAOGroup&   vaoGroup) :
        renderTarget(theRenderTarget)
    {
        renderTarget.setupDraw(vaoGroup, theRenderStates);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~DrawGuard()
    {
        renderTarget.cleanupDraw(renderTarget.m_lastRenderStates);
    }
};


////////////////////////////////////////////////////////////
void RenderTarget::immediateDrawVertices(const DrawVerticesSettings& settings, const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (settings.vertexSpan.isNullOrEmpty() || !setActive(true))
        return;

    const DrawGuard drawGuard{*this, states, m_impl->vaoGroup};

    RenderTargetImpl::streamVerticesToGPU(settings.vertexSpan);
    invokePrimitiveDrawCall(settings.primitiveType, 0u, settings.vertexSpan.size());
}


////////////////////////////////////////////////////////////
void RenderTarget::immediateDrawIndexedVertices(const DrawIndexedVerticesSettings& settings, const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (settings.vertexSpan.isNullOrEmpty() || settings.indexSpan.isNullOrEmpty() || !setActive(true))
        return;

    const DrawGuard drawGuard{*this, states, m_impl->vaoGroup};

    RenderTargetImpl::streamVerticesToGPU(settings.vertexSpan);
    RenderTargetImpl::streamIndicesToGPU(settings.indexSpan);

    invokePrimitiveDrawCallIndexed(settings.primitiveType, settings.indexSpan.size(), /* indexOffset */ 0u);
}


////////////////////////////////////////////////////////////
void RenderTarget::immediateDrawPersistentMappedIndexedVertices(
    [[maybe_unused]] const DrawPersistentMappedIndexedVerticesSettings& settings,
    [[maybe_unused]] const RenderStates&                                states)
{
#ifdef SFML_OPENGL_ES
    priv::err() << "FATAL ERROR: Persistent OpenGL buffers are not available in OpenGL ES";
    base::abort();
#else
    // Nothing to draw or inactive target
    if (settings.indexCount == 0u || !setActive(true))
        return;

    const DrawGuard drawGuard{*this,
                              states,
                              *static_cast<const GLVAOGroup*>(settings.gpuDrawableBatch.m_storage.getVAOGroup())};

    invokePrimitiveDrawCallIndexedBaseVertex(settings.primitiveType, settings.indexCount, settings.indexOffset, settings.vertexOffset);
#endif
}


////////////////////////////////////////////////////////////
void RenderTarget::immediateDrawDrawableBatch(const CPUDrawableBatch& drawableBatch, RenderStates states)
{
    SFML_BASE_ASSERT(drawableBatch.m_storage.indices.size() % 3u == 0u);

    states.transform *= drawableBatch.getTransform();

    immediateDrawIndexedVertices(
        {
            .vertexSpan    = drawableBatch.m_storage.vertices,
            .indexSpan     = drawableBatch.m_storage.indices,
            .primitiveType = PrimitiveType::Triangles,
        },
        states);
}


////////////////////////////////////////////////////////////
void RenderTarget::immediateDrawInstancedVertices(const DrawInstancedVerticesSettings&                    settings,
                                                  base::FixedFunction<void(InstanceAttributeBinder&), 64> setupFn,
                                                  const RenderStates&                                     states)
{
    // Nothing to draw or inactive target
    if (settings.vertexSpan.isNullOrEmpty() || settings.instanceCount == 0u || !setActive(true))
        return;

    const DrawGuard drawGuard{*this, states, settings.vaoHandle.asVAOGroup()};

    RenderTargetImpl::streamVerticesToGPU(settings.vertexSpan);

    InstanceAttributeBinder iab;
    setupFn(iab);

    invokeInstancedPrimitiveDrawCall(settings.primitiveType, 0, settings.vertexSpan.size(), settings.instanceCount);
}


////////////////////////////////////////////////////////////
void RenderTarget::immediateDrawInstancedIndexedVertices(const DrawInstancedIndexedVerticesSettings& settings,
                                                         base::FixedFunction<void(InstanceAttributeBinder&), 64> setupFn,
                                                         const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (settings.vertexSpan.isNullOrEmpty() || settings.indexSpan.isNullOrEmpty() || settings.instanceCount == 0u ||
        !setActive(true))
        return;

    const DrawGuard drawGuard{*this, states, settings.vaoHandle.asVAOGroup()};

    RenderTargetImpl::streamVerticesToGPU(settings.vertexSpan);
    RenderTargetImpl::streamIndicesToGPU(settings.indexSpan);

    InstanceAttributeBinder iab;
    setupFn(iab);

    invokeInstancedPrimitiveDrawCallIndexed(settings.primitiveType, 0, settings.indexSpan.size(), settings.instanceCount);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const CPUDrawableBatch& drawableBatch, const RenderStates& states)
{
    if (m_autoBatchMode != AutoBatchMode::Disabled)
        flush();

    immediateDrawDrawableBatch(drawableBatch, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const PersistentGPUDrawableBatch& drawableBatch, RenderStates states)
{
    if (m_autoBatchMode != AutoBatchMode::Disabled)
        flush();

    states.transform *= drawableBatch.getTransform();

    drawableBatch.flushVertexWritesToGPU(drawableBatch.getNumVertices(), 0u);
    drawableBatch.flushIndexWritesToGPU(drawableBatch.getNumIndices(), 0u);

    immediateDrawPersistentMappedIndexedVertices(
        {
            .gpuDrawableBatch = drawableBatch,
            .indexCount       = drawableBatch.getNumIndices(),
            .indexOffset      = 0u,
            .vertexOffset     = 0u,
            .primitiveType    = PrimitiveType::Triangles,
        },
        states);
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
    m_impl->cache.lastVaoGroup = 0u; // Force VAO rebind (TODO P0: why is this needed?)

    if (m_autoBatchMode != AutoBatchMode::Disabled)
        flush();

    // Sanity check
    if (firstVertex > vertexBuffer.getVertexCount())
        return;

    // Clamp vertexCount to something that makes sense
    vertexCount = base::min(vertexCount, vertexBuffer.getVertexCount() - firstVertex);

    // Nothing to draw or inactive target

    if (!vertexCount || !vertexBuffer.getNativeHandle() || !setActive(true))
        return;

    const DrawGuard drawGuard{*this, states, m_impl->vaoGroup};

    // Bind vertex buffer
    vertexBuffer.bind();

    // Always enable texture coordinates (needed because different buffer is bound)
    RenderTargetImpl::setupVertexAttribPointers();

    invokePrimitiveDrawCall(vertexBuffer.getPrimitiveType(), firstVertex, vertexCount);

    // Unbind vertex buffer
    VertexBuffer::unbind();

    // Needed to restore attrib pointers on regular VBO
    m_impl->bindGLObjects(m_impl->vaoGroup);
}


////////////////////////////////////////////////////////////
VertexSpan RenderTarget::draw(const priv::ShapeDataConcept auto& shapeData, const RenderStates& states)
{
    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        return addToAutoBatch(shapeData);
    }

    m_impl->cpuAutoBatch.clear();

    SFML_BASE_SCOPE_GUARD({ immediateDrawDrawableBatch(m_impl->cpuAutoBatch, states); });
    return m_impl->cpuAutoBatch.add(shapeData);
}


////////////////////////////////////////////////////////////
template VertexSpan RenderTarget::draw(const ArrowShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const CircleShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const CurvedArrowShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const EllipseShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const PieSliceShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const RectangleShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const RingShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const RingPieSliceShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const RoundedRectangleShapeData&, const RenderStates&);
template VertexSpan RenderTarget::draw(const StarShapeData&, const RenderStates&);


////////////////////////////////////////////////////////////
VertexSpan RenderTarget::draw(const Font& font, const TextData& textData, RenderStates states)
{
    states.texture = &font.getTexture();

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        return addToAutoBatch(font, textData);
    }

    m_impl->cpuAutoBatch.clear();

    SFML_BASE_SCOPE_GUARD({ immediateDrawDrawableBatch(m_impl->cpuAutoBatch, states); });
    return m_impl->cpuAutoBatch.add(font, textData);
}


////////////////////////////////////////////////////////////
VertexSpan RenderTarget::draw(const FontFace&            fontFace,
                              const GlyphMapping&        glyphMapping,
                              const GlyphMappedTextData& textData,
                              const RenderStates&        states)
{
    SFML_BASE_ASSERT(states.texture != nullptr);

    if (m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        return addToAutoBatch(fontFace, glyphMapping, textData);
    }

    m_impl->cpuAutoBatch.clear();

    SFML_BASE_SCOPE_GUARD({ immediateDrawDrawableBatch(m_impl->cpuAutoBatch, states); });
    return m_impl->cpuAutoBatch.add(fontFace, glyphMapping, textData);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawVertices(const DrawVerticesSettings& settings, const RenderStates& states)
{
    if (RenderTargetImpl::isPrimitiveTypeSupportedByBatchStorage(settings.primitiveType) &&
        m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(settings);
        return;
    }

    flush();
    immediateDrawVertices(settings, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawIndexedVertices(const DrawIndexedVerticesSettings& settings, const RenderStates& states)
{
    if (RenderTargetImpl::isPrimitiveTypeSupportedByBatchStorage(settings.primitiveType) &&
        m_autoBatchMode != AutoBatchMode::Disabled)
    {
        flushIfNeeded(states);
        addToAutoBatch(settings);
        return;
    }

    flush();
    immediateDrawIndexedVertices(settings, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawQuads(const DrawQuadsSettings& settings, const RenderStates& states)
{
    const auto vertexCount = settings.vertexSpan.size();

    SFML_BASE_ASSERT(vertexCount % 4u == 0u);
    SFML_BASE_ASSERT(vertexCount < base::getArraySize(RenderTargetImpl::precomputedQuadIndices) / 6u * 4u);

    drawIndexedVertices(
        {
            .vertexSpan    = settings.vertexSpan,
            .indexSpan     = {RenderTargetImpl::precomputedQuadIndices, vertexCount / 4u * 6u},
            .primitiveType = settings.primitiveType,
        },
        states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawPersistentMappedIndexedVertices(const DrawPersistentMappedIndexedVerticesSettings& settings,
                                                       const RenderStates&                                states)
{
    flush();
    immediateDrawPersistentMappedIndexedVertices(settings, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawInstancedVertices(const DrawInstancedVerticesSettings&                           settings,
                                         const base::FixedFunction<void(InstanceAttributeBinder&), 64>& setupFn,
                                         const RenderStates&                                            states)
{
    flush();
    immediateDrawInstancedVertices(settings, setupFn, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawInstancedIndexedVertices(const DrawInstancedIndexedVerticesSettings&                    settings,
                                                const base::FixedFunction<void(InstanceAttributeBinder&), 64>& setupFn,
                                                const RenderStates&                                            states)
{
    flush();
    immediateDrawInstancedIndexedVertices(settings, setupFn, states);
}


////////////////////////////////////////////////////////////
bool RenderTarget::isSrgb() const
{
    // By default sRGB encoding is not enabled for an arbitrary RenderTarget
    return false;
}


////////////////////////////////////////////////////////////
bool RenderTarget::setActive(const bool active)
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
View RenderTarget::computeView() const
{
    return View::fromScreenSize(getSize().toVec2f());
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
#ifdef SFML_SYSTEM_MACOS
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

    m_impl->cache.enable = true;
}


////////////////////////////////////////////////////////////
RenderTarget::DrawStatistics RenderTarget::flush()
{
    SFML_BASE_SCOPE_GUARD({ m_numAutoBatchVertices = 0u; });

    if (m_autoBatchMode == AutoBatchMode::Disabled)
        return m_currentDrawStats;

#ifdef SFML_OPENGL_ES

    immediateDrawDrawableBatch(m_impl->cpuAutoBatch, m_lastRenderStates);
    m_impl->cpuAutoBatch.clear();

#else

    if (m_autoBatchMode == AutoBatchMode::CPUStorage)
    {
        immediateDrawDrawableBatch(m_impl->cpuAutoBatch, m_lastRenderStates);
        m_impl->cpuAutoBatch.clear();
    }
    else
    {
        SFML_BASE_ASSERT(m_autoBatchMode == AutoBatchMode::GPUStorage);

        auto& [batch, fence, indexOffset, vertexOffset] = m_impl->currentGPUAutoBatchState();

        const auto vertexCount = batch.getNumVertices() - vertexOffset;
        const auto indexCount  = batch.getNumIndices() - indexOffset;

        if (vertexCount == 0u || indexCount == 0u)
            return m_currentDrawStats;

        batch.flushVertexWritesToGPU(vertexCount, vertexOffset);
        batch.flushIndexWritesToGPU(indexCount, indexOffset);

        immediateDrawPersistentMappedIndexedVertices(
            {
                .gpuDrawableBatch = batch,
                .indexCount       = indexCount,
                .indexOffset      = indexOffset,
                .vertexOffset     = 0u, // Vertex offset is always `0` for GPU autobatching
                .primitiveType    = PrimitiveType::Triangles,
            },
            m_lastRenderStates);

        indexOffset  = batch.getNumIndices();
        vertexOffset = batch.getNumVertices();
    }

#endif

    return m_currentDrawStats;
}


////////////////////////////////////////////////////////////
void RenderTarget::invokeGlFlush()
{
    glCheck(glFlush());
}


////////////////////////////////////////////////////////////
void RenderTarget::invokeGlFinish()
{
    glCheck(glFinish());
}


////////////////////////////////////////////////////////////
RenderTarget::WithRenderStatesContext::WithRenderStatesContext(RenderTarget& rt, const RenderStates& states, const bool locked) :
    m_rt{&rt},
    m_states{states},
    m_locked{locked}
{
    if (m_rt->getAutoBatchMode() != AutoBatchMode::Disabled)
        m_rt->flushIfNeeded(m_states);

    if (m_locked)
    {
        SFML_BASE_ASSERT(!m_rt->m_isStateLocked && "Cannot create a context while another is active");
        m_rt->m_isStateLocked = true;
    }
}


////////////////////////////////////////////////////////////
RenderTarget::WithRenderStatesContext::WithRenderStatesContext::~WithRenderStatesContext()
{
    if (m_locked)
    {
        SFML_BASE_ASSERT(m_rt->m_isStateLocked && "Cannot destroy a context while no context is active");
        m_rt->m_isStateLocked = false;
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::syncGPUStartFrame()
{
#ifndef SFML_OPENGL_ES
    auto& [batch, fenceToWaitOn, indexOffset, vertexOffset] = m_impl->currentGPUAutoBatchState();

    if (!RenderTargetImpl::waitOnFence(fenceToWaitOn))
        return;

    batch.clear();

    indexOffset  = 0u;
    vertexOffset = 0u;
#endif
}


////////////////////////////////////////////////////////////
void RenderTarget::syncGPUEndFrame()
{
#ifndef SFML_OPENGL_ES
    auto& fenceToCreate = m_impl->currentGPUAutoBatchState().fence;
    SFML_BASE_ASSERT(fenceToCreate == nullptr);

    fenceToCreate = RenderTargetImpl::makeFence();

    // Advance to the next fence index for the *next* frame
    m_impl->currentGPUAutoBatchIndex = (m_impl->currentGPUAutoBatchIndex + 1u) %
                                       RenderTargetImpl::maxGPUAutoBatchFramesInFlight;
#endif
}


////////////////////////////////////////////////////////////
void RenderTarget::applyView(const View& view)
{
    const Vec2f targetSize = getSize().toVec2f();

    // Set the viewport
    const Rect2i viewport    = view.computePixelViewport(targetSize);
    const int    viewportTop = static_cast<int>(targetSize.y) - (viewport.position.y + viewport.size.y);

    glCheck(glViewport(viewport.position.x, viewportTop, viewport.size.x, viewport.size.y));

    // Set the scissor rectangle and enable/disable scissor testing
    if (view.scissor == Rect2f{{0.f, 0.f}, {1.f, 1.f}})
    {
        if (!m_impl->cache.enable || m_impl->cache.scissorEnabled)
        {
            glCheck(glDisable(GL_SCISSOR_TEST));
            m_impl->cache.scissorEnabled = false;
        }
    }
    else
    {
        const Rect2i pixelScissor = view.computePixelScissor(targetSize);
        const int    scissorTop   = static_cast<int>(targetSize.y) - (pixelScissor.position.y + pixelScissor.size.y);

        glCheck(glScissor(pixelScissor.position.x, scissorTop, pixelScissor.size.x, pixelScissor.size.y));

        if (!m_impl->cache.enable || !m_impl->cache.scissorEnabled)
        {
            glCheck(glEnable(GL_SCISSOR_TEST));
            m_impl->cache.scissorEnabled = true;
        }
    }

    m_impl->cache.lastView = view;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyBlendMode(const BlendMode& mode)
{
    glCheck(glBlendFuncSeparate(priv::factorToGlConstant(mode.colorSrcFactor),
                                priv::factorToGlConstant(mode.colorDstFactor),
                                priv::factorToGlConstant(mode.alphaSrcFactor),
                                priv::factorToGlConstant(mode.alphaDstFactor)));

    glCheck(glBlendEquationSeparate(priv::equationToGlConstant(mode.colorEquation),
                                    priv::equationToGlConstant(mode.alphaEquation)));

    m_impl->cache.lastBlendMode = mode;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyStencilMode(const StencilMode& mode)
{
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
                        priv::stencilOperationToGlConstant(mode.stencilUpdateOperation),
                        priv::stencilOperationToGlConstant(mode.stencilUpdateOperation)));

    glCheck(glStencilFunc(priv::stencilFunctionToGlConstant(mode.stencilComparison),
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
        else
        {
            // TODO P0: why is this needed??? prevents crash in Render Tests
            RenderTargetImpl::setupVertexAttribPointers();
        }
    }

    // Select shader to be used
    const Shader& usedShader = states.shader != nullptr ? *states.shader : GraphicsContext::getInstalledBuiltInShader();

    // Select view to be used
    const View usedView = states.view == View{} ? computeView() : states.view;

    // Update shader
    const auto usedNativeHandle = usedShader.getNativeHandle();
    const bool shaderChanged    = m_impl->cache.lastProgramId != usedNativeHandle;

    if (!m_impl->cache.enable || shaderChanged)
    {
        usedShader.bind();
        m_impl->cache.lastProgramId = usedNativeHandle;
    }

    // Apply the view
    const bool viewChanged = m_impl->cache.lastView != usedView;
    if (!m_impl->cache.enable || viewChanged)
        applyView(usedView);

    // Set the model-view-projection matrix
    if (!m_impl->cache.enable || shaderChanged || viewChanged || states.transform != m_impl->cache.lastRenderStatesTransform)
        setupDrawMVP(states.transform, m_impl->cache.lastView.getTransform());

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
    setupDrawTexture(states, shaderChanged);

    // Update last used render states
    m_lastRenderStates = states;
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDrawMVP(const Transform& renderStatesTransform, const Transform& viewTransform)
{
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
void RenderTarget::setupDrawTexture(const RenderStates& states, const bool shaderChanged)
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

    // Bind the texture if needed
    if (mustApplyTexture)
    {
        usedTexture.bind();
        m_impl->cache.lastTextureId = usedTexture.m_cacheId;
    }

    // Upload inverse texture size when texture or shader changed (hardcoded layout location `4u` for `sf_u_invTextureSize`)
    if (mustApplyTexture || shaderChanged)
    {
        const auto invTexSize = 1.f / usedTexture.getSize().toVec2f();
        glCheck(glUniform2f(/* location */ 4u, invTexSize.x, invTexSize.y));
    }
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


////////////////////////////////////////////////////////////
void RenderTarget::invokePrimitiveDrawCall(const PrimitiveType type, const base::SizeT firstVertex, const base::SizeT vertexCount)
{
    m_currentDrawStats.drawCalls += 1u;
    m_currentDrawStats.drawnVertices += vertexCount;

    glCheck(glDrawArrays(/*     primitive type */ priv::primitiveTypeToOpenGLMode(type),
                         /* first vertex index */ static_cast<GLint>(firstVertex),
                         /*       vertex count */ static_cast<GLsizei>(vertexCount)));
}


////////////////////////////////////////////////////////////
void RenderTarget::invokePrimitiveDrawCallIndexed(const PrimitiveType type, const base::SizeT indexCount, const base::SizeT indexOffset)
{
    m_currentDrawStats.drawCalls += 1u;
    m_currentDrawStats.drawnVertices += indexCount;

    glCheck(glDrawElements(/* primitive type */ priv::primitiveTypeToOpenGLMode(type),
                           /*    index count */ static_cast<GLsizei>(indexCount),
                           /*     index type */ GL_UNSIGNED_INT,
                           /*   index offset */ reinterpret_cast<void*>(indexOffset * sizeof(IndexType))));
}


////////////////////////////////////////////////////////////
void RenderTarget::invokePrimitiveDrawCallIndexedBaseVertex(
    [[maybe_unused]] const PrimitiveType type,
    [[maybe_unused]] const base::SizeT   indexCount,
    [[maybe_unused]] const base::SizeT   indexOffset,
    [[maybe_unused]] const base::SizeT   vertexOffset)
{
#ifdef SFML_OPENGL_ES
    priv::err() << "FATAL ERROR: `glDrawElementsBaseVertex` only available in OpenGL ES 3.2+ (unsupported)";
    base::abort();
#else
    m_currentDrawStats.drawCalls += 1u;
    m_currentDrawStats.drawnVertices += indexCount;

    glCheck(glDrawElementsBaseVertex(/* primitive type */ priv::primitiveTypeToOpenGLMode(type),
                                     /*    index count */ static_cast<GLsizei>(indexCount),
                                     /*     index type */ GL_UNSIGNED_INT,
                                     /*   index offset */ reinterpret_cast<void*>(indexOffset * sizeof(IndexType)),
                                     /*    base vertex */ static_cast<GLint>(vertexOffset)));
#endif
}


////////////////////////////////////////////////////////////
void RenderTarget::invokeInstancedPrimitiveDrawCall(const PrimitiveType type,
                                                    const base::SizeT   vertexOffset,
                                                    const base::SizeT   vertexCount,
                                                    const base::SizeT   instanceCount)
{
    m_currentDrawStats.drawCalls += 1u;
    m_currentDrawStats.drawnVertices += vertexCount * instanceCount;

    glCheck(glDrawArraysInstanced(/*      primitive type */ priv::primitiveTypeToOpenGLMode(type),
                                  /*       vertex offset */ static_cast<GLint>(vertexOffset),
                                  /*        vertex count */ static_cast<GLsizei>(vertexCount),
                                  /* number of instances */ static_cast<GLsizei>(instanceCount)));
}


////////////////////////////////////////////////////////////
void RenderTarget::invokeInstancedPrimitiveDrawCallIndexed(
    const PrimitiveType type,
    const base::SizeT   indexOffset,
    const base::SizeT   indexCount,
    const base::SizeT   instanceCount)
{
    m_currentDrawStats.drawCalls += 1u;
    m_currentDrawStats.drawnVertices += indexCount * instanceCount;

    glCheck(glDrawElementsInstanced(/*      primitive type */ priv::primitiveTypeToOpenGLMode(type),
                                    /*         index count */ static_cast<GLsizei>(indexCount),
                                    /*          index type */ GL_UNSIGNED_INT,
                                    /*        index offset */ reinterpret_cast<void*>(indexOffset * sizeof(IndexType)),
                                    /* number of instances */ static_cast<GLsizei>(instanceCount)));
}

} // namespace sf


////////////////////////////////////////////////////////////
// Render states caching strategies
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

// TODO P0: document autobatching limitations
// - shader uniform updates not tracked in same batch
// - texture updates not tracked in same batch
