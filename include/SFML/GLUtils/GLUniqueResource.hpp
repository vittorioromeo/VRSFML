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
/// \brief Generic RAII base class for managing OpenGL resources.
/// \ingroup glutils
///
/// This template class provides basic RAII (Resource Acquisition Is Initialization)
/// functionality for OpenGL objects identified by an unsigned integer ID.
/// It requires a policy struct `TFuncs` that defines static `create`, `destroy`,
/// `bind`, and `get` functions specific to the type of OpenGL resource being managed.
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
    /// \brief Get the ID of the currently bound object of this type.
    ///
    /// \return The OpenGL ID of the currently bound resource.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static unsigned int getBoundId()
    {
        unsigned int out{};
        TFuncs::get(out);
        return out;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the OpenGL ID of this resource.
    ///
    /// \return The unique OpenGL identifier for this resource.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] unsigned int getId() const
    {
        return m_id;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Check if this specific resource instance is currently bound.
    ///
    /// \return True if `getId()` matches `getBoundId()`, false otherwise.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isBound() const
    {
        return getBoundId() == m_id;
    }

    ////////////////////////////////////////////////////////////
    /// \brief Bind this resource to the current OpenGL context.
    ///
    /// Asserts that the resource ID is valid and that the binding was successful.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(m_id != 0u);
        TFuncs::bind(m_id);

        SFML_BASE_ASSERT(isBound());
    }

    ////////////////////////////////////////////////////////////
    /// \brief Unbind this resource type from the current OpenGL context.
    ///
    /// Binds the default object (ID `0`) for this resource type.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unbind() const
    {
        TFuncs::bind(0u);
        SFML_BASE_ASSERT(!isBound());
    }

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int m_id;
};


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline, gnu::flatten]] static inline base::Optional<T> tryCreateGLUniqueResource()
{
    unsigned int id{};
    T::FuncsType::create(id);

    if (id == 0u)
        return base::nullOpt;

    return base::makeOptional<T>(id);
}

} // namespace sf
