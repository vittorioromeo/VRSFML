// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/VertexBuffer.hpp"

#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"


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
            return GL_STATIC_DRAW;
        case sf::VertexBuffer::Usage::Dynamic:
            return GL_DYNAMIC_DRAW;
        default:
            return GL_STREAM_DRAW;
    }
}
} // namespace VertexBufferImpl
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer() = default;


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(PrimitiveType type) : m_primitiveType(type)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(Usage usage) : m_usage(usage)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(PrimitiveType type, Usage usage) : m_primitiveType(type), m_usage(usage)
{
}


////////////////////////////////////////////////////////////
VertexBuffer::VertexBuffer(const VertexBuffer& rhs) : m_primitiveType(rhs.m_primitiveType), m_usage(rhs.m_usage)
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
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
        SFML_BASE_ASSERT(GraphicsContext::isInstalled());

        glCheck(glDeleteBuffers(1, &m_buffer));
    }
}


////////////////////////////////////////////////////////////
bool VertexBuffer::create(base::SizeT vertexCount)
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(GraphicsContext::isInstalled());

    if (!m_buffer)
        glCheck(glGenBuffers(1, &m_buffer));

    if (!m_buffer)
    {
        priv::err() << "Could not create vertex buffer, generation failed";
        return false;
    }

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
    glCheck(glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexCount),
                         nullptr,
                         VertexBufferImpl::usageToGlEnum(m_usage)));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

    m_size = vertexCount;

    return true;
}


////////////////////////////////////////////////////////////
base::SizeT VertexBuffer::getVertexCount() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
bool VertexBuffer::update(const Vertex* vertices)
{
    return update(vertices, m_size, 0);
}


////////////////////////////////////////////////////////////
bool VertexBuffer::update(const Vertex* vertices, base::SizeT vertexCount, unsigned int offset)
{
    // Sanity checks
    if (!m_buffer)
        return false;

    if (!vertices)
        return false;

    if (offset && (offset + vertexCount > m_size))
        return false;

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(GraphicsContext::isInstalled());

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));

    // Check if we need to resize or orphan the buffer
    if (vertexCount >= m_size)
    {
        glCheck(glBufferData(GL_ARRAY_BUFFER,
                             static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexCount),
                             nullptr,
                             VertexBufferImpl::usageToGlEnum(m_usage)));

        m_size = vertexCount;
    }

    glCheck(glBufferSubData(GL_ARRAY_BUFFER,
                            static_cast<GLintptrARB>(sizeof(Vertex) * offset),
                            static_cast<GLsizeiptrARB>(sizeof(Vertex) * vertexCount),
                            vertices));

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

    return true;
}


////////////////////////////////////////////////////////////
bool VertexBuffer::update(const VertexBuffer& vertexBuffer) const
{
    if (!m_buffer || !vertexBuffer.m_buffer)
        return false;

    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(GraphicsContext::isInstalled());

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


////////////////////////////////////////////////////////////
VertexBuffer& VertexBuffer::operator=(const VertexBuffer& rhs)
{
    VertexBuffer temp(rhs);

    swap(temp);

    return *this;
}


////////////////////////////////////////////////////////////
void VertexBuffer::swap(VertexBuffer& rhs) noexcept
{
    base::swap(m_size, rhs.m_size);
    base::swap(m_buffer, rhs.m_buffer);
    base::swap(m_primitiveType, rhs.m_primitiveType);
    base::swap(m_usage, rhs.m_usage);
}


////////////////////////////////////////////////////////////
unsigned int VertexBuffer::getNativeHandle() const
{
    return m_buffer;
}


////////////////////////////////////////////////////////////
void VertexBuffer::bind() const
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(GraphicsContext::isInstalled());
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_buffer));
}


////////////////////////////////////////////////////////////
void VertexBuffer::unbind()
{
    SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
    SFML_BASE_ASSERT(GraphicsContext::isInstalled());
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0u));
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
void VertexBuffer::draw(RenderTarget& target, RenderStates states) const
{
    if (m_buffer && m_size)
        target.draw(*this, 0, m_size, states);
}


////////////////////////////////////////////////////////////
void swap(VertexBuffer& lhs, VertexBuffer& rhs) noexcept
{
    lhs.swap(rhs);
}

} // namespace sf
