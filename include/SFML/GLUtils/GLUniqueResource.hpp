#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename TFuncs>
class [[nodiscard]] GLUniqueResource
{
public:
    ////////////////////////////////////////////////////////////
    using FuncsType = TFuncs;

    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit GLUniqueResource()
    {
        TFuncs::create(m_id);
        SFML_BASE_ASSERT(m_id != 0u);
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit GLUniqueResource(const unsigned int id) : m_id{id}
    {
        SFML_BASE_ASSERT(m_id != 0u);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] ~GLUniqueResource()
    {
        if (m_id != 0u)
            TFuncs::destroy(m_id);
    }

    ////////////////////////////////////////////////////////////
    GLUniqueResource(const GLUniqueResource&)            = delete;
    GLUniqueResource& operator=(const GLUniqueResource&) = delete;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] GLUniqueResource(GLUniqueResource&& rhs) noexcept :
    m_id{base::exchange(rhs.m_id, 0u)}
    {
        SFML_BASE_ASSERT(m_id != 0u);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] GLUniqueResource& operator=(GLUniqueResource&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        if (m_id != 0u)
            TFuncs::destroy(m_id);

        m_id = base::exchange(rhs.m_id, 0u);
        SFML_BASE_ASSERT(m_id != 0u);

        return *this;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static unsigned int getBoundId()
    {
        unsigned int out{};
        TFuncs::get(out);
        return out;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] unsigned int getId() const
    {
        return m_id;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isBound() const
    {
        return getBoundId() == m_id;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(m_id != 0u);
        TFuncs::bind(m_id);

        SFML_BASE_ASSERT(isBound());
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unbind() const
    {
        TFuncs::bind(0u);
        SFML_BASE_ASSERT(!isBound());
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reallocate()
    {
        if (m_id != 0u)
            TFuncs::destroy(m_id);

        TFuncs::create(m_id);
        SFML_BASE_ASSERT(m_id != 0u);
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int m_id;
};


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten]] static base::Optional<T> tryCreateGLUniqueResource()
{
    unsigned int id{};
    T::FuncsType::create(id);

    if (id == 0u)
        return base::nullOpt;

    return base::makeOptional<T>(id);
}

} // namespace sf
