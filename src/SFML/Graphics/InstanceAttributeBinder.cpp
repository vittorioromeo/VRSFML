// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/InstanceAttributeBinder.hpp"

#include "SFML/Graphics/GlDataType.hpp"
#include "SFML/Graphics/Priv/EnumToGlEnumConversions.hpp"
#include "SFML/Graphics/VBOHandle.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Unreachable.hpp"
#include "SFML/Base/SizeT.hpp"

#ifdef SFML_OPENGL_ES
    #include "SFML/System/Err.hpp"

    #include "SFML/Base/Abort.hpp"
#endif


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr bool isIntegerAttributeType(const sf::GlDataType type) noexcept
{
    switch (type)
    {
        case sf::GlDataType::Byte:
        case sf::GlDataType::UnsignedByte:
        case sf::GlDataType::Short:
        case sf::GlDataType::UnsignedShort:
        case sf::GlDataType::Int:
        case sf::GlDataType::UnsignedInt:
            return true;

        case sf::GlDataType::HalfFloat:
        case sf::GlDataType::Float:
        case sf::GlDataType::Double:
            return false;
    }

    SFML_BASE_UNREACHABLE();
}


////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline void setupVertexAttribPointer(
    const unsigned int   location,
    const unsigned int   size,
    const sf::GlDataType type,
    const bool           normalized,
    const GLsizei        stride,
    void* const          offset)
{
    const GLenum glType = sf::priv::dataTypeToOpenGLDataType(type);

    if (type == sf::GlDataType::Double)
    {
#ifdef SFML_OPENGL_ES
        sf::priv::err() << "FATAL ERROR: per-instance `double` attributes are unsupported on OpenGL ES";
        sf::base::abort();
#else
        glCheck(glVertexAttribLPointer(/*  index */ location,
                                       /*   size */ static_cast<GLint>(size),
                                       /*   type */ glType,
                                       /* stride */ stride,
                                       /* offset */ offset));
#endif

        return;
    }

    if (isIntegerAttributeType(type) && !normalized)
    {
        glCheck(glVertexAttribIPointer(/*  index */ location,
                                       /*   size */ static_cast<GLint>(size),
                                       /*   type */ glType,
                                       /* stride */ stride,
                                       /* offset */ offset));

        return;
    }

    glCheck(glVertexAttribPointer(/*      index */ location,
                                  /*       size */ static_cast<GLint>(size),
                                  /*       type */ glType,
                                  /* normalized */ normalized ? GL_TRUE : GL_FALSE,
                                  /*     stride */ stride,
                                  /*     offset */ offset));
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
InstanceAttributeBinder::InstanceAttributeBinder(const base::SizeT instanceCount) : m_instanceCount(instanceCount)
{
    SFML_BASE_ASSERT(instanceCount > 0u);
}

////////////////////////////////////////////////////////////
InstanceAttributeBinder::~InstanceAttributeBinder()
{
    if (m_drawSubmitted)
    {
        for (VBOHandle* const vboHandle : m_touchedVBOHandles)
            vboHandle->commitPendingUploads();

        return;
    }

    for (VBOHandle* const vboHandle : m_touchedVBOHandles)
        vboHandle->rollbackPendingUploads();
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::uploadData(VBOHandle& vboHandle, const void* const data, const base::SizeT stride)
{
    SFML_BASE_ASSERT(data != nullptr);
    SFML_BASE_ASSERT(stride > 0u);

    m_currentVBOHandle        = &vboHandle;
    m_currentUploadByteOffset = vboHandle.uploadStreamingData(data, stride * m_instanceCount);

    bool alreadyTouched = false;

    for (VBOHandle* const touchedVBOHandle : m_touchedVBOHandles)
    {
        if (touchedVBOHandle == &vboHandle)
        {
            alreadyTouched = true;
            break;
        }
    }

    if (!alreadyTouched)
        m_touchedVBOHandles.emplaceBack(&vboHandle);
}


////////////////////////////////////////////////////////////
void InstanceAttributeBinder::setup(
    const unsigned int location,
    const unsigned int size,
    const GlDataType   type,
    const bool         normalized,
    const base::SizeT  stride,
    const base::SizeT  fieldOffset)
{
    SFML_BASE_ASSERT(size >= 1u && size <= 4u);
    SFML_BASE_ASSERT(stride > 0u);
    SFML_BASE_ASSERT(fieldOffset < stride);
    SFML_BASE_ASSERT(m_currentVBOHandle != nullptr);
    m_deferredSetups.emplaceBack(DeferredSetup{
        .vboHandle  = m_currentVBOHandle,
        .location   = location,
        .size       = size,
        .type       = type,
        .normalized = normalized,
        .stride     = stride,
        .byteOffset = m_currentUploadByteOffset + fieldOffset,
    });
}

////////////////////////////////////////////////////////////
void InstanceAttributeBinder::applySetups() const
{
    VBOHandle* lastBoundVBOHandle = nullptr;

    for (const DeferredSetup& deferredSetup : m_deferredSetups)
    {
        if (deferredSetup.vboHandle != lastBoundVBOHandle)
        {
            deferredSetup.vboHandle->bind();
            lastBoundVBOHandle = deferredSetup.vboHandle;
        }

        glCheck(glEnableVertexAttribArray(deferredSetup.location));

        setupVertexAttribPointer(deferredSetup.location,
                                 deferredSetup.size,
                                 deferredSetup.type,
                                 deferredSetup.normalized,
                                 static_cast<GLsizei>(deferredSetup.stride),
                                 reinterpret_cast<void*>(deferredSetup.byteOffset));

        glCheck(glVertexAttribDivisor(deferredSetup.location, 1));
    }
}

////////////////////////////////////////////////////////////
void InstanceAttributeBinder::markDrawSubmitted() noexcept
{
    m_drawSubmitted = true;
}

} // namespace sf
