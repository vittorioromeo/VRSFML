#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Exchange.hpp"
#include "SFML/Base/Optional.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Generic RAII base class for managing OpenGL resources
/// \ingroup glutils
///
/// This template class provides basic RAII (Resource Acquisition Is
/// Initialization) functionality for OpenGL objects identified by an
/// unsigned integer ID. It requires a policy struct `TFuncs` that defines
/// the following static functions specific to the type of OpenGL resource
/// being managed:
///
/// - `create(unsigned int& id)`: allocate a new GL object and write its ID
/// - `destroy(unsigned int& id)`: release the GL object identified by `id`
/// - `bind(unsigned int id)`: bind the GL object (or `0` to unbind)
/// - `get(unsigned int& id)`: query the currently bound object's ID
///
/// Instances are non-copyable but movable. After move, the source is
/// left with a zeroed ID and is no longer usable. The class is marked
/// `[[nodiscard]]` to discourage accidental discards of newly created
/// resources.
///
/// \tparam TFuncs Policy class providing the four static functions
///                described above
///
/// \see `GLBufferObject`, `GLVertexArrayObject`, `GLRenderBufferObject`
///
////////////////////////////////////////////////////////////
template <typename TFuncs>
class [[nodiscard]] GLUniqueResource
{
public:
    ////////////////////////////////////////////////////////////
    using FuncsType = TFuncs;


    ////////////////////////////////////////////////////////////
    /// \brief Adopt an already-created OpenGL resource by ID
    ///
    /// Wraps an existing OpenGL object whose ID was obtained externally,
    /// taking ownership of its lifetime. The destructor will release the
    /// resource via `TFuncs::destroy`.
    ///
    /// \param id Non-zero OpenGL identifier to take ownership of
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit GLUniqueResource(const unsigned int id) : m_id{id}
    {
        SFML_BASE_ASSERT(m_id != 0u);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Destructor: release the underlying OpenGL resource
    ///
    /// Calls `TFuncs::destroy` if the ID is still valid (i.e. the
    /// instance has not been moved-from).
    ///
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
    /// \brief Move constructor
    ///
    /// Transfers ownership of the OpenGL resource from `rhs` to the new
    /// instance. After the move, `rhs` holds an ID of `0` and must not
    /// be used.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] GLUniqueResource(GLUniqueResource&& rhs) noexcept :
        m_id{base::exchange(rhs.m_id, 0u)}
    {
        SFML_BASE_ASSERT(m_id != 0u);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Move assignment operator
    ///
    /// Releases the resource currently owned by `*this` (if any) and
    /// then transfers ownership from `rhs`. After the move, `rhs` holds
    /// an ID of `0` and must not be used.
    ///
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
    /// \brief Get the ID of the currently bound object of this type
    ///
    /// Queries the GL state via `TFuncs::get` for the resource that is
    /// currently bound at the binding point managed by this policy
    /// (independent of any specific instance).
    ///
    /// \return The OpenGL ID of the currently bound resource, or `0`
    ///         if no resource is bound at this binding point
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static unsigned int getBoundId()
    {
        unsigned int out{};
        TFuncs::get(out);
        return out;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the OpenGL ID of this resource
    ///
    /// \return The unique OpenGL identifier owned by this instance,
    ///         or `0` if the instance has been moved-from
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] unsigned int getId() const
    {
        return m_id;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Check whether this specific resource instance is currently bound
    ///
    /// \return `true` if `getId()` matches `getBoundId()`, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] bool isBound() const
    {
        SFML_BASE_ASSERT(m_id != 0u); // error to call `isBound()` on a moved-from instance
        return getBoundId() == m_id;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Bind this resource to the current OpenGL context
    ///
    /// Calls `TFuncs::bind` with the owned ID. In debug builds it
    /// asserts both that the resource ID is valid and that the binding
    /// was successful (via `isBound()`).
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(m_id != 0u);
        TFuncs::bind(m_id);

        SFML_BASE_ASSERT(isBound());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Unbind this resource type from the current OpenGL context
    ///
    /// Binds the default object (ID `0`) for this resource type. Note
    /// that this affects the binding point globally, not just for this
    /// instance.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unbind() const
    {
        TFuncs::bind(0u);
        SFML_BASE_ASSERT(!isBound());
    }

private:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor: allocate a new OpenGL resource
    ///
    /// Calls `TFuncs::create` to allocate the underlying GL object and
    /// asserts that a non-zero ID was returned. If allocation may fail
    /// at run-time (e.g. due to a missing GL context), prefer
    /// `tryCreateGLUniqueResource` which returns an `Optional` instead.
    ///
    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline, gnu::flatten]] explicit GLUniqueResource()
    {
        TFuncs::create(m_id);
        SFML_BASE_ASSERT(m_id != 0u);
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    unsigned int m_id;
};


////////////////////////////////////////////////////////////
/// \brief Try to create a `GLUniqueResource`-derived object, returning an `Optional`
/// \ingroup glutils
///
/// Calls the policy's `create` function and, if it returns a valid
/// non-zero ID, wraps the result in a `T` instance and returns it
/// inside an `Optional`. Returns `base::nullOpt` if the GL allocation
/// failed (e.g. when no GL context is currently active).
///
/// Use this in place of the default constructor when you need to
/// gracefully handle resource creation failures rather than asserting.
///
/// \tparam T A type derived from `GLUniqueResource` that exposes a
///           `FuncsType` member alias to its policy class
///
/// \return An `Optional<T>` containing the freshly created resource on
///         success, or `nullOpt` on failure
///
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
