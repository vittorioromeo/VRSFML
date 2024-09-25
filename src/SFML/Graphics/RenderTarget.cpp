#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/BlendMode.hpp"
#include "SFML/Graphics/CoordinateType.hpp"
#include "SFML/Graphics/DrawableBatch.hpp"
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics/Shape.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/StencilMode.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/Graphics/VertexBuffer.hpp"
#include "SFML/Graphics/View.hpp"

#include "SFML/Window/GLCheck.hpp"
#include "SFML/Window/Glad.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Rect.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/OffsetOf.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Math/Lround.hpp"
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
[[nodiscard]] bool isActive(sf::GraphicsContext& graphicsContext, IdType id)
{
    const IdType contextId = graphicsContext.getActiveThreadLocalGlContextId();
    SFML_BASE_ASSERT(contextId < maxIdCount);

    const auto renderTargetId = contextRenderTargetMap[contextId].load();
    return (renderTargetId != invalidId) && (renderTargetId == id);
}


////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(fnName, sfEnumType, ...)                                  \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr GLenum fnName(sfEnumType sfEnumValue) \
    {                                                                                                           \
        constexpr GLenum glValues[] __VA_ARGS__;                                                                \
                                                                                                                \
        SFML_BASE_ASSERT(static_cast<unsigned int>(sfEnumValue) < ::sf::base::getArraySize(glValues));          \
        return glValues[static_cast<unsigned int>(sfEnumValue)];                                                \
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
    sf::Vector2u         renderTargetSize,
    const sf::FloatRect& inputRect)
{
    const auto [width, height] = renderTargetSize.toVector2f();

    return sf::Rect<long>({sf::base::lround(width * inputRect.position.x), sf::base::lround(height * inputRect.position.y)},
                          {sf::base::lround(width * inputRect.size.x), sf::base::lround(height * inputRect.size.y)})
        .to<sf::IntRect>();
}

////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void streamToGPU(GLenum bufferType, const void* data, sf::base::SizeT dataByteCount)
{
#ifdef SFML_OPENGL_ES
    glCheck(glBufferData(bufferType, static_cast<GLsizeiptr>(dataByteCount), data, GL_STREAM_DRAW));
#else
    glCheck(glBufferData(bufferType, dataByteCount, nullptr, GL_STREAM_DRAW));

    void* ptr = glMapBufferRange(bufferType,
                                 0u,
                                 dataByteCount,
                                 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_INVALIDATE_RANGE_BIT);

    SFML_BASE_MEMCPY(ptr, data, dataByteCount);
    glUnmapBuffer(bufferType);
#endif
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
    Transform viewTransform;      //!< Cached transform of latest view

    Transform lastDrawTransform; //!< Cached last draw transform

    Texture::MatrixElems lastTextureMatrixElems{}; //!< Cached last draw uploaded texture matrix elems

    bool scissorEnabled{false}; //!< Is scissor testing enabled?
    bool stencilEnabled{false}; //!< Is stencil testing enabled?

    bool vaoBound{false}; //!< Have buffer objects been bound?

    BlendMode      lastBlendMode{BlendAlpha};                      //!< Cached blending mode
    StencilMode    lastStencilMode;                                //!< Cached stencil
    base::U64      lastTextureId{0u};                              //!< Cached texture
    CoordinateType lastCoordinateType{CoordinateType::Normalized}; //!< Cached texture coordinate type

    GLuint lastUsedProgramId{0u}; //!< GL id of the last used shader program
};


////////////////////////////////////////////////////////////
template <auto FnGen, auto FnBind, auto FnGet, auto FnDelete>
class [[nodiscard]] OpenGLRAII
{
public:
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit OpenGLRAII(GraphicsContext&)
    {
        SFML_BASE_ASSERT(m_id == 0u);
        FnGen(m_id);
        SFML_BASE_ASSERT(m_id != 0u);
    }

    [[gnu::always_inline, gnu::flatten]] ~OpenGLRAII()
    {
        if (m_id != 0u)
            FnDelete(m_id);
    }

    [[gnu::always_inline, gnu::flatten]] OpenGLRAII(OpenGLRAII&& rhs) noexcept : m_id(base::exchange(rhs.m_id, 0u))
    {
    }

    [[gnu::always_inline, gnu::flatten]] OpenGLRAII& operator=(OpenGLRAII&& rhs) noexcept
    {
        if (&rhs != this)
            m_id = base::exchange(rhs.m_id, 0u);

        return *this;
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isBound() const
    {
        int out{};
        FnGet(out);
        return out != 0u;
    }

    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(m_id != 0u);
        FnBind(m_id);

        SFML_BASE_ASSERT(isBound());
    }

    [[gnu::always_inline, gnu::flatten]] void unbind() const
    {
        FnBind(0u);
        SFML_BASE_ASSERT(isBound());
    }

    OpenGLRAII(const OpenGLRAII&)            = delete;
    OpenGLRAII& operator=(const OpenGLRAII&) = delete;


private:
    unsigned int m_id{};
};


////////////////////////////////////////////////////////////
using VAO = OpenGLRAII<[](auto& id) { glCheck(glGenVertexArrays(1, &id)); },
                       [](auto id) { glCheck(glBindVertexArray(id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteVertexArrays(1, &id)); }>;


////////////////////////////////////////////////////////////
using VBO = OpenGLRAII<[](auto& id) { glCheck(glGenBuffers(1, &id)); },
                       [](auto id) { glCheck(glBindBuffer(GL_ARRAY_BUFFER, id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteBuffers(1, &id)); }>;


////////////////////////////////////////////////////////////
using EBO = OpenGLRAII<[](auto& id) { glCheck(glGenBuffers(1, &id)); },
                       [](auto id) { glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteBuffers(1, &id)); }>;


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
                                  /*     stride */ sizeof(Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(Vertex, position)));

    // Hardcoded layout location `1u` for `sf_a_color`
    glCheck(glEnableVertexAttribArray(1u));
    glCheck(glVertexAttribPointer(/*      index */ 1u,
                                  /*       size */ 4,
                                  /*       type */ GL_UNSIGNED_BYTE,
                                  /* normalized */ GL_TRUE,
                                  /*     stride */ sizeof(Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(Vertex, color)));

    // Hardcoded layout location `2u` for `sf_a_texCoord`
    glCheck(glEnableVertexAttribArray(2u));
    glCheck(glVertexAttribPointer(/*      index */ 2u,
                                  /*       size */ 2,
                                  /*       type */ GL_FLOAT,
                                  /* normalized */ GL_FALSE,
                                  /*     stride */ sizeof(Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(Vertex, texCoords)));

#undef SFML_PRIV_OFFSETOF
}


////////////////////////////////////////////////////////////
struct RenderTarget::Impl
{
    explicit Impl(GraphicsContext& theGraphicsContext, const View& theView) :
    graphicsContext(&theGraphicsContext),
    view(theView),
    id(RenderTargetImpl::nextUniqueId.fetch_add(1u, std::memory_order_relaxed)),
    vao(theGraphicsContext),
    vbo(theGraphicsContext),
    ebo(theGraphicsContext)
    {
        bindGLObjects();
        cache.vaoBound = true;

        setupVertexAttribPointers();
    }

    GraphicsContext* graphicsContext; //!< The window context

    View view; //!< Current view

    StatesCache cache{}; //!< Render states cache

    RenderTargetImpl::IdType id{}; //!< Unique number that identifies the render target

    VAO vao; //!< Vertex array object associated with the render target
    VBO vbo; //!< Vertex buffer object associated with the render target
    EBO ebo; //!< Element index buffer object associated with the render target

    void bindGLObjects() const
    {
        vao.bind();
        vbo.bind();
        ebo.bind();
    }
};


////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(GraphicsContext& graphicsContext, const View& currentView) :
m_impl(graphicsContext, currentView)
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
    if (!RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) && !setActive(true))
    {
        priv::err() << "Failed to activate render target in `clearImpl`";
        return false;
    }

    // Unbind texture to fix RenderTexture preventing clear
    unapplyTexture(); // See https://en.sfml-dev.org/forums/index.php?topic=9350

    // Apply the view (scissor testing can affect clearing)
    if (!m_impl->cache.enable || m_impl->cache.viewChanged)
        applyView(m_impl->view);

    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(Color color)
{
    if (!clearImpl())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clearStencil(StencilValue stencilValue)
{
    if (!clearImpl())
        return;

    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(Color color, StencilValue stencilValue)
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

    m_impl->view              = view;
    m_impl->cache.viewChanged = true;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getView() const
{
    return m_impl->view;
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
Vector2f RenderTarget::mapPixelToCoords(Vector2i point) const
{
    return mapPixelToCoords(point, getView());
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(Vector2i point, const View& view) const
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
Vector2i RenderTarget::mapCoordsToPixel(Vector2f point) const
{
    return mapCoordsToPixel(point, getView());
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(Vector2f point, const View& view) const
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
void RenderTarget::draw(const Sprite& sprite, const Texture& texture, RenderStates states)
{
    states.texture        = &texture;
    states.coordinateType = CoordinateType::Pixels;

    Vertex buffer[4];
    priv::spriteToVertices(sprite, buffer);

    draw(buffer, PrimitiveType::TriangleStrip, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Shape& shape, const Texture* texture, const RenderStates& states)
{
    shape.drawOnto(*this, texture, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Vertex* vertices, base::SizeT vertexCount, PrimitiveType type, const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (vertices == nullptr || vertexCount == 0u ||
        (!RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) && !setActive(true)))
        return;

    setupDraw(states);

    RenderTargetImpl::streamToGPU(GL_ARRAY_BUFFER, vertices, sizeof(Vertex) * vertexCount);

    drawPrimitives(type, 0u, vertexCount);
    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::drawIndexedVertices(
    const Vertex*       vertices,
    base::SizeT         vertexCount,
    const unsigned int* indices,
    base::SizeT         indexCount,
    PrimitiveType       type,
    const RenderStates& states)
{
    // Nothing to draw or inactive target
    if (vertices == nullptr || vertexCount == 0u || indices == nullptr || indexCount == 0u ||
        (!RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) && !setActive(true)))
        return;

    setupDraw(states);

    RenderTargetImpl::streamToGPU(GL_ARRAY_BUFFER, vertices, sizeof(Vertex) * vertexCount);
    RenderTargetImpl::streamToGPU(GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(unsigned int) * indexCount);

    drawIndexedPrimitives(type, indexCount);
    cleanupDraw(states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const DrawableBatch& drawableBatch, const RenderStates& renderStates)
{
    drawIndexedVertices(drawableBatch.m_vertices.data(),
                        drawableBatch.m_vertices.size(),
                        drawableBatch.m_indices.data(),
                        drawableBatch.m_indices.size(),
                        PrimitiveType::Triangles,
                        renderStates);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const VertexBuffer& vertexBuffer, const RenderStates& states)
{
    draw(vertexBuffer, 0, vertexBuffer.getVertexCount(), states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const VertexBuffer& vertexBuffer, base::SizeT firstVertex, base::SizeT vertexCount, const RenderStates& states)
{
    // Sanity check
    if (firstVertex > vertexBuffer.getVertexCount())
        return;

    // Clamp vertexCount to something that makes sense
    vertexCount = base::min(vertexCount, vertexBuffer.getVertexCount() - firstVertex);

    // Nothing to draw or inactive target

    if (!vertexCount || !vertexBuffer.getNativeHandle() ||
        (!RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) && !setActive(true)))
        return;

    setupDraw(states);

    // Bind vertex buffer
    vertexBuffer.bind(*m_impl->graphicsContext);

    // Always enable texture coordinates (needed because different buffer is bound)
    setupVertexAttribPointers();

    drawPrimitives(vertexBuffer.getPrimitiveType(), firstVertex, vertexCount);

    // Unbind vertex buffer
    VertexBuffer::unbind(*vertexBuffer.m_graphicsContext);
    m_impl->vbo.bind();
    setupVertexAttribPointers(); // Needed to restore attrib pointers on regular VBO

    cleanupDraw(states);
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
    // If this RenderTarget is already active on the current GL context, do nothing
    if (const bool isAlreadyActive = RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id);
        (active && isAlreadyActive) || (!active && !isAlreadyActive))
        return true;

    // Mark this RenderTarget as active or no longer active in the tracking map
    const RenderTargetImpl::IdType contextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

    SFML_BASE_ASSERT(contextId < RenderTargetImpl::maxIdCount);
    std::atomic<RenderTargetImpl::IdType>& renderTargetId = RenderTargetImpl::contextRenderTargetMap[contextId];

    // Deactivation
    if (!active)
    {
        SFML_BASE_ASSERT(renderTargetId.load() != RenderTargetImpl::invalidId);
        renderTargetId.store(RenderTargetImpl::invalidId);

        m_impl->cache.enable = false;
        return true;
    }

    // First ever activation
    if (renderTargetId.load() == RenderTargetImpl::invalidId)
    {
        renderTargetId.store(m_impl->id);

        m_impl->cache.glStatesSet = false;
        m_impl->cache.enable      = false;
        return true;
    }

    // Activation on a different context
    SFML_BASE_ASSERT(renderTargetId.load() != RenderTargetImpl::invalidId);
    SFML_BASE_ASSERT(renderTargetId.load() != m_impl->id);
    renderTargetId.store(m_impl->id);

    m_impl->cache.enable = false;
    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::resetGLStates()
{
// Workaround for states not being properly reset on
// macOS unless a context switch really takes place
#if defined(SFML_SYSTEM_MACOS)
    if (!setActive(false))
        priv::err() << "Failed to set render target inactive";
#endif

    if (!RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) && !setActive(true))
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
    m_impl->cache.vaoBound       = false;

    m_impl->cache.glStatesSet = true;

    // Apply the default SFML states
    applyBlendMode(BlendAlpha);
    applyStencilMode(StencilMode());
    unapplyTexture();

    {
        Shader::unbind(*m_impl->graphicsContext);
        m_impl->cache.lastUsedProgramId = 0u;
    }

    VertexBuffer::unbind(*m_impl->graphicsContext);

    // Set the default view
    setView(getView());

    m_impl->cache.enable = true;
}


////////////////////////////////////////////////////////////
GraphicsContext& RenderTarget::getGraphicsContext()
{
    return *m_impl->graphicsContext;
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

    m_impl->cache.viewTransform = view.getTransform();
    m_impl->cache.viewChanged   = false;
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

    // Fast path if we have a default (disabled) stencil mode
    if (mode == StencilMode())
    {
        if (!m_impl->cache.enable || m_impl->cache.stencilEnabled)
        {
            glCheck(glDisable(GL_STENCIL_TEST));
            glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

            m_impl->cache.stencilEnabled = false;
        }
    }
    else
    {
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

    m_impl->cache.lastStencilMode = mode;
}


////////////////////////////////////////////////////////////
void RenderTarget::unapplyTexture()
{
    Texture::unbind(*m_impl->graphicsContext);

    m_impl->cache.lastTextureId      = 0ul;
    m_impl->cache.lastCoordinateType = CoordinateType::Pixels;
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDraw(const RenderStates& states)
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
        resetGLStates();

    // Bind GL objects
    if (!m_impl->cache.enable || !m_impl->cache.vaoBound)
    {
        m_impl->bindGLObjects();
        m_impl->cache.vaoBound = true;

        setupVertexAttribPointers();
    }

    // Select shader to be used
    const Shader& usedShader = states.shader != nullptr ? *states.shader : m_impl->graphicsContext->getBuiltInShader();

    // Update shader
    const auto usedNativeHandle = usedShader.getNativeHandle();
    const bool shaderChanged    = m_impl->cache.lastUsedProgramId != usedNativeHandle;

    if (shaderChanged)
    {
        usedShader.bind();
        m_impl->cache.lastUsedProgramId = usedNativeHandle;
    }

    // Apply the view
    if (!m_impl->cache.enable || m_impl->cache.viewChanged)
        applyView(m_impl->view);

    // Set the model-view-projection matrix
    setupDrawMVP(states, m_impl->cache.viewTransform, shaderChanged);

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
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDrawMVP(const RenderStates& states, const Transform& viewTransform, bool shaderChanged)
{
    // Compute the final draw transform
    const Transform trsfm = viewTransform * /* model-view matrix */ states.transform;

    // If there's no difference from the cached one, exit early
    if (!shaderChanged && (m_impl->cache.enable && trsfm == m_impl->cache.lastDrawTransform))
        return;

    // Update the cached transform
    m_impl->cache.lastDrawTransform = trsfm;

    // clang-format off
    const float transformMatrixBuffer[]{trsfm.m_a00, trsfm.m_a10, 0.f, 0.f,
                                        trsfm.m_a01, trsfm.m_a11, 0.f, 0.f,
                                        0.f,         0.f,         1.f, 0.f,
                                        trsfm.m_a02, trsfm.m_a12, 0.f, 1.f};
    // clang-format on

    // Upload uniform data to GPU (hardcoded layout location `0u` for `sf_u_modelViewProjectionMatrix`)
    glCheck(glUniformMatrix4fv(0u, 1, GL_FALSE, transformMatrixBuffer));
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDrawTexture(const RenderStates& states, bool shaderChanged)
{
    // Select texture to be used
    const Texture& usedTexture = states.texture != nullptr ? *states.texture
                                                           : getGraphicsContext().getBuiltInWhiteDotTexture();

    // If the texture is an FBO attachment, always rebind it in order to inform the OpenGL driver that we
    // want changes made to it in other contexts to be visible here as well. This saves us from having to
    // call `glFlush()` in `RenderTextureImplFBO` which can be quite costly
    //
    // See: https://www.khronos.org/opengl/wiki/Memory_Model

    // Should the texture be bound?
    const bool mustApplyTexture = !m_impl->cache.enable || usedTexture.m_fboAttachment ||
                                  usedTexture.m_cacheId != m_impl->cache.lastTextureId ||
                                  states.coordinateType != m_impl->cache.lastCoordinateType;

    // If not, exit early
    if (!mustApplyTexture)
        return;

    // Bind the texture
    usedTexture.bind(*m_impl->graphicsContext);

    // Update basic cache texture stuff
    m_impl->cache.lastTextureId      = usedTexture.m_cacheId;
    m_impl->cache.lastCoordinateType = states.coordinateType;

    // Retrieve the texture elements
    const auto elems = usedTexture.getMatrixElems(m_impl->cache.lastCoordinateType);

    // If texture uniform doesn't need an update, exit early
    if (!shaderChanged && (m_impl->cache.enable && m_impl->cache.lastTextureMatrixElems == elems))
        return;

    // Otherwise, update the cached matrix elements
    m_impl->cache.lastTextureMatrixElems = elems;

    // Create the matrix buffer with the elements
    // clang-format off
    const float textureMatrixBuffer[]{elems.a00, 0.f,       0.f, 0.f,
                                      0.f,       elems.a11, 0.f, 0.f,
                                      0.f,       0.f,       1.f, 0.f,
                                      0.f,       elems.a12, 0.f, 1.f};
    // clang-format on

    // Upload uniform data to GPU (hardcoded layout location `1u` for `sf_u_textureMatrix`)
    glCheck(glUniformMatrix4fv(1u, 1, GL_FALSE, textureMatrixBuffer));
}


////////////////////////////////////////////////////////////
void RenderTarget::drawPrimitives(PrimitiveType type, base::SizeT firstVertex, base::SizeT vertexCount)
{
    glCheck(glDrawArrays(/*     primitive type */ RenderTargetImpl::primitiveTypeToOpenGLMode(type),
                         /* first vertex index */ static_cast<GLint>(firstVertex),
                         /*       vertex count */ static_cast<GLsizei>(vertexCount)));
}


////////////////////////////////////////////////////////////
void RenderTarget::drawIndexedPrimitives(PrimitiveType type, base::SizeT indexCount)
{
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
