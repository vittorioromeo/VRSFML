#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>

#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>

#include <SFML/System/Err.hpp>

#include <utility>

#include <cstddef>
#include <cstring>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace VertexBufferImpl
{
GLenum usageToGlEnum(sf::VertexBuffer::Usage usage)
{
    switch (usage)
    {
        case sf::VertexBuffer::Usage::Static:
            return GLEXT_GL_STATIC_DRAW;
        case sf::VertexBuffer::Usage::Dynamic:
            return GLEXT_GL_DYNAMIC_DRAW;
        default:
            return GLEXT_GL_STREAM_DRAW;
    }
}
} // namespace VertexBufferImpl
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(GraphicsContext& graphicsContext) : m_graphicsContext(&graphicsContext)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(GraphicsContext& graphicsContext, PrimitiveType type) :
m_graphicsContext(&graphicsContext),
m_primitiveType(type)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(GraphicsContext& graphicsContext, Usage usage) :
m_graphicsContext(&graphicsContext),
m_usage(usage)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(GraphicsContext& graphicsContext, PrimitiveType type, Usage usage) :
m_graphicsContext(&graphicsContext),
m_primitiveType(type),
m_usage(usage)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(const VertexBuffer& rhs) :
m_graphicsContext(rhs.m_graphicsContext),
m_primitiveType(rhs.m_primitiveType),
m_usage(rhs.m_usage)
{
    if (rhs.m_buffer && rhs.m_size)
    {
        if (!create(rhs.m_size))
        {
            priv::err() << "Could not create vertex buffer for copying";
            return;
        }

        if (!update(rhs))
            priv::err() << "Could not copy vertex buffer";
    }
}


////////////////////////////////////////////////////////////
VertexBuffer::~VertexBuffer()
{
    if (m_buffer)
    {
        SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

        glCheck(GLEXT_glDeleteBuffers(1, &m_buffer));
    }
}


////////////////////////////////////////////////////////////
bool VertexBuffer::create(std::size_t vertexCount)
{
    if (!isAvailable(*m_graphicsContext))
        return false;

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    if (!m_buffer)
        glCheck(GLEXT_glGenBuffers(1, &m_buffer));

    if (!m_buffer)
    {
        priv::err() << "Could not create vertex buffer, generation failed";
        return false;
    }

    glCheck(GLEXT_glBindBuffer(GLEXT_GL_ARRAY_BUFFER, m_buffer));
    glCheck(GLEXT_glBufferData(GLEXT_GL_ARRAY_BUFFER,
                               static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexCount),
                               nullptr,
                               VertexBufferImpl::usageToGlEnum(m_usage)));
    glCheck(GLEXT_glBindBuffer(GLEXT_GL_ARRAY_BUFFER, 0));

    m_size = vertexCount;

    return true;
}


////////////////////////////////////////////////////////////
std::size_t VertexBuffer::getVertexCount() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
bool VertexBuffer::update(const Vertex* vertices)
{
    return update(vertices, m_size, 0);
}


////////////////////////////////////////////////////////////
bool VertexBuffer::update(const Vertex* vertices, std::size_t vertexCount, unsigned int offset)
{
    // Sanity checks
    if (!m_buffer)
        return false;

    if (!vertices)
        return false;

    if (offset && (offset + vertexCount > m_size))
        return false;

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    glCheck(GLEXT_glBindBuffer(GLEXT_GL_ARRAY_BUFFER, m_buffer));

    // Check if we need to resize or orphan the buffer
    if (vertexCount >= m_size)
    {
        glCheck(GLEXT_glBufferData(GLEXT_GL_ARRAY_BUFFER,
                                   static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexCount),
                                   nullptr,
                                   VertexBufferImpl::usageToGlEnum(m_usage)));

        m_size = vertexCount;
    }

    glCheck(GLEXT_glBufferSubData(GLEXT_GL_ARRAY_BUFFER,
                                  static_cast<GLintptrARB>(sizeof(Vertex) * offset),
                                  static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexCount),
                                  vertices));

    glCheck(GLEXT_glBindBuffer(GLEXT_GL_ARRAY_BUFFER, 0));

    return true;
}


////////////////////////////////////////////////////////////
bool VertexBuffer::update([[maybe_unused]] const VertexBuffer& vertexBuffer)
{
    if (!m_buffer || !vertexBuffer.m_buffer)
        return false;

    SFML_BASE_ASSERT(m_graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    if (GLEXT_copy_buffer)
    {
        glCheck(glBindBuffer(GL_COPY_READ_BUFFER, vertexBuffer.m_buffer));
        glCheck(glBindBuffer(GL_COPY_WRITE_BUFFER, m_buffer));

        glCheck(glCopyBufferSubData(GL_COPY_READ_BUFFER,
                                    GL_COPY_WRITE_BUFFER,
                                    0,
                                    0,
                                    static_cast<GLsizeiptr>(sizeof(Vertex) * vertexBuffer.m_size)));

        glCheck(glBindBuffer(GL_COPY_WRITE_BUFFER, 0));
        glCheck(glBindBuffer(GL_COPY_READ_BUFFER, 0));

        return true;
    }

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
    glCheck(glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexBuffer.m_size),
                         nullptr,
                         VertexBufferImpl::usageToGlEnum(m_usage)));

    void* destination = nullptr;
    glCheck(destination = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.m_buffer));

    void* source = nullptr;
    glCheck(source = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));

    std::memcpy(destination, source, sizeof(Vertex) * vertexBuffer.m_size);

    GLboolean sourceResult = GL_FALSE;
    glCheck(sourceResult = glUnmapBuffer(GL_ARRAY_BUFFER));

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));

    GLboolean destinationResult = GL_FALSE;
    glCheck(destinationResult = glUnmapBuffer(GL_ARRAY_BUFFER));

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

    return (sourceResult == GL_TRUE) && (destinationResult == GL_TRUE);
}


////////////////////////////////////////////////////////////
VertexBuffer& VertexBuffer::operator=(const VertexBuffer& rhs)
{
    VertexBuffer temp(rhs);

    swap(temp);

    return *this;
}


////////////////////////////////////////////////////////////
void VertexBuffer::swap(VertexBuffer& right) noexcept
{
    std::swap(m_size, right.m_size);
    std::swap(m_buffer, right.m_buffer);
    std::swap(m_primitiveType, right.m_primitiveType);
    std::swap(m_usage, right.m_usage);
}


////////////////////////////////////////////////////////////
unsigned int VertexBuffer::getNativeHandle() const
{
    return m_buffer;
}


////////////////////////////////////////////////////////////
void VertexBuffer::bind(GraphicsContext& graphicsContext, const VertexBuffer* vertexBuffer)
{
    if (!isAvailable(graphicsContext))
        return;

    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    glCheck(GLEXT_glBindBuffer(GLEXT_GL_ARRAY_BUFFER, vertexBuffer ? vertexBuffer->m_buffer : 0));
}


////////////////////////////////////////////////////////////
void VertexBuffer::setPrimitiveType(PrimitiveType type)
{
    m_primitiveType = type;
}


////////////////////////////////////////////////////////////
PrimitiveType VertexBuffer::getPrimitiveType() const
{
    return m_primitiveType;
}


////////////////////////////////////////////////////////////
void VertexBuffer::setUsage(Usage usage)
{
    m_usage = usage;
}


////////////////////////////////////////////////////////////
VertexBuffer::Usage VertexBuffer::getUsage() const
{
    return m_usage;
}


////////////////////////////////////////////////////////////
bool VertexBuffer::isAvailable([[maybe_unused]] GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
    return GLEXT_vertex_buffer_object != 0;
}


////////////////////////////////////////////////////////////
void VertexBuffer::draw(RenderTarget& target, RenderStates states) const
{
    if (m_buffer && m_size)
        target.draw(*this, 0, m_size, states);
}


////////////////////////////////////////////////////////////
void swap(VertexBuffer& left, VertexBuffer& right) noexcept
{
    left.swap(right);
}

} // namespace sf
