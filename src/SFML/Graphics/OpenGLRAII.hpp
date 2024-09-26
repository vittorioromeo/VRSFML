#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
class GraphicsContext;


////////////////////////////////////////////////////////////
using OpenGLRAIIFnPtrRef = void (*)(unsigned int& id);
using OpenGLRAIIFnPtr    = void (*)(unsigned int id);


////////////////////////////////////////////////////////////
class [[nodiscard]] OpenGLRAIIID
{
public:
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit OpenGLRAIIID(unsigned int id) : m_id{id}
    {
    }

    OpenGLRAIIID(const OpenGLRAIIID&)            = delete;
    OpenGLRAIIID& operator=(const OpenGLRAIIID&) = delete;

    [[gnu::always_inline, gnu::flatten]] OpenGLRAIIID(OpenGLRAIIID&& rhs) noexcept : m_id(base::exchange(rhs.m_id, 0u))
    {
    }

    [[gnu::always_inline, gnu::flatten]] OpenGLRAIIID& operator=(OpenGLRAIIID&& rhs) noexcept
    {
        if (&rhs != this)
            m_id = base::exchange(rhs.m_id, 0u);

        return *this;
    }

    [[nodiscard, gnu::always_inline]] unsigned int& get() noexcept
    {
        return m_id;
    }

    [[nodiscard, gnu::always_inline]] const unsigned int& get() const noexcept
    {
        return m_id;
    }

private:
    unsigned int m_id;
};


////////////////////////////////////////////////////////////
class [[nodiscard]] OpenGLRAII
{
public:
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit OpenGLRAII(
        GraphicsContext&,
        OpenGLRAIIFnPtrRef fnGen,
        OpenGLRAIIFnPtr    fnBind,
        OpenGLRAIIFnPtrRef fnGet,
        OpenGLRAIIFnPtrRef fnDelete) :
    m_fnGen{fnGen},
    m_fnBind{fnBind},
    m_fnGet{fnGet},
    m_fnDelete{fnDelete},
    m_id{0u}
    {
        SFML_BASE_ASSERT(m_id.get() == 0u);
        m_fnGen(m_id.get());
        SFML_BASE_ASSERT(m_id.get() != 0u);
    }

    [[gnu::always_inline, gnu::flatten]] ~OpenGLRAII()
    {
        if (m_id.get() != 0u)
            m_fnDelete(m_id.get());
    }

    OpenGLRAII(const OpenGLRAII&)            = delete;
    OpenGLRAII& operator=(const OpenGLRAII&) = delete;

    [[gnu::always_inline, gnu::flatten]] OpenGLRAII(OpenGLRAII&& rhs) noexcept            = default;
    [[gnu::always_inline, gnu::flatten]] OpenGLRAII& operator=(OpenGLRAII&& rhs) noexcept = default;

    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isBound() const
    {
        unsigned int out{};
        m_fnGet(out);
        return out != 0u;
    }

    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(m_id.get() != 0u);
        m_fnBind(m_id.get());

        SFML_BASE_ASSERT(isBound());
    }

    [[gnu::always_inline, gnu::flatten]] void unbind() const
    {
        m_fnBind(0u);
        SFML_BASE_ASSERT(isBound());
    }

    [[gnu::always_inline, gnu::flatten]] void reallocate()
    {
        if (m_id.get() != 0u)
            m_fnDelete(m_id.get());

        m_fnGen(m_id.get());
        SFML_BASE_ASSERT(m_id.get() != 0u);
    }

private:
    OpenGLRAIIFnPtrRef m_fnGen;
    OpenGLRAIIFnPtr    m_fnBind;
    OpenGLRAIIFnPtrRef m_fnGet;
    OpenGLRAIIFnPtrRef m_fnDelete;

    OpenGLRAIIID m_id;
};

} // namespace sf
