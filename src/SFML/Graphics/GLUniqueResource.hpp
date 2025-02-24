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
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] GLUniqueResourceID
{
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GLUniqueResourceID() = default;

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit GLUniqueResourceID(unsigned int theValue) : value{theValue}
    {
    }

    ////////////////////////////////////////////////////////////
    GLUniqueResourceID(const GLUniqueResourceID&)            = delete;
    GLUniqueResourceID& operator=(const GLUniqueResourceID&) = delete;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] GLUniqueResourceID(GLUniqueResourceID&& rhs) noexcept :
    value(base::exchange(rhs.value, 0u))
    {
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] GLUniqueResourceID& operator=(GLUniqueResourceID&& rhs) noexcept
    {
        if (&rhs != this)
            value = base::exchange(rhs.value, 0u);

        return *this;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int value;
};

////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
template <typename TFuncs>
class [[nodiscard]] GLUniqueResource
{
public:
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit GLUniqueResource()
    {
        TFuncs::create(m_id.value);
        SFML_BASE_ASSERT(m_id.value != 0u);
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] ~GLUniqueResource()
    {
        if (m_id.value != 0u)
            TFuncs::destroy(m_id.value);
    }

    ////////////////////////////////////////////////////////////
    GLUniqueResource(const GLUniqueResource&)            = delete;
    GLUniqueResource& operator=(const GLUniqueResource&) = delete;

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] GLUniqueResource(GLUniqueResource&& rhs) noexcept            = default;
    [[gnu::always_inline, gnu::flatten]] GLUniqueResource& operator=(GLUniqueResource&& rhs) noexcept = default;

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
        return m_id.value;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isBound() const
    {
        return getBoundId() == m_id.value;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(m_id.value != 0u);
        TFuncs::bind(m_id.value);

        SFML_BASE_ASSERT(isBound());
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unbind() const
    {
        TFuncs::bind(0u);
        SFML_BASE_ASSERT(isBound());
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reallocate()
    {
        if (m_id.value != 0u)
            TFuncs::destroy(m_id.value);

        TFuncs::create(m_id.value);
        SFML_BASE_ASSERT(m_id.value != 0u);
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    GLUniqueResourceID m_id;
};

} // namespace sf
