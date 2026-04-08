#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLBufferObject.hpp"
#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUniqueResource.hpp"
#include "SFML/GLUtils/Glad.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Manages a group consisting of a VAO, VBO, and EBO
/// \ingroup glutils
///
/// Helper struct that encapsulates the common pattern of pairing a
/// Vertex Array Object (VAO) with a Vertex Buffer Object (VBO) and an
/// Element Buffer Object (EBO/IBO).
///
/// Because VAOs are not shareable between OpenGL contexts, this struct
/// stores **one VAO per context** in a hash map keyed by GL context ID.
/// The VBO and EBO, on the other hand, are created on the shared
/// context and are reused across all contexts. A VAO for the currently
/// active context is created and configured lazily on the first call
/// to `bind()` from that context.
///
////////////////////////////////////////////////////////////
struct GLVAOGroup
{
    ////////////////////////////////////////////////////////////
    mutable ankerl::unordered_dense::map<unsigned int, unsigned int> perContextVAOIds; //!< Map from GL context ID to its lazily created VAO ID


    ////////////////////////////////////////////////////////////
    GLVertexBufferObject  vbo; //!< Vertex buffer object (created on the shared context)
    GLElementBufferObject ebo; //!< Element index buffer object (created on the shared context)


    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates the underlying VBO and EBO on the shared GL context.
    /// VAOs are not allocated here — they are created on demand by
    /// `bind()` on the first call from each context.
    ///
    ////////////////////////////////////////////////////////////
    explicit GLVAOGroup() :
        vbo{tryCreateGLUniqueResource<GLVertexBufferObject>().value()},
        ebo{tryCreateGLUniqueResource<GLElementBufferObject>().value()}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Unregisters this group's VAO for the currently active context
    /// from `WindowContext`. The actual `glDeleteVertexArrays` call is
    /// performed by `WindowContext` when the corresponding GL context
    /// is destroyed, since unshared VAOs can only be deleted while
    /// their owning context is current.
    ///
    /// \warning Requires a thread-local GL context to be active when
    ///          the destructor runs.
    ///
    ////////////////////////////////////////////////////////////
    ~GLVAOGroup()
    {
        SFML_BASE_ASSERT(WindowContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = WindowContext::getActiveThreadLocalGlContextId();

        auto* it = perContextVAOIds.find(glContextId);

        if (it != perContextVAOIds.end())
            WindowContext::unregisterUnsharedVAO(glContextId, it->second);
    }


    ////////////////////////////////////////////////////////////
    GLVAOGroup(const GLVAOGroup&)            = delete;
    GLVAOGroup& operator=(const GLVAOGroup&) = delete;


    ////////////////////////////////////////////////////////////
    GLVAOGroup(GLVAOGroup&&) noexcept            = default;
    GLVAOGroup& operator=(GLVAOGroup&&) noexcept = default;


    ////////////////////////////////////////////////////////////
    /// \brief Bind the associated VAO, VBO, and EBO for the current context
    ///
    /// If no VAO exists yet for the currently active OpenGL context,
    /// one is created via `glGenVertexArrays`, registered with
    /// `WindowContext` (so that it gets cleaned up alongside the
    /// context), bound, and stored in `perContextVAOIds`. Otherwise,
    /// the cached VAO ID is bound directly. In either case, the shared
    /// VBO and EBO are bound after the VAO so that subsequent draw
    /// calls see the correct vertex layout.
    ///
    /// \warning Requires a thread-local GL context to be active.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(WindowContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = WindowContext::getActiveThreadLocalGlContextId();

        const auto* it = perContextVAOIds.find(glContextId);

        if (it != perContextVAOIds.end())
        {
            glCheck(glBindVertexArray(it->second));
        }
        else
        {
            unsigned int id{};
            glCheck(glGenVertexArrays(1, &id));

            if (id == 0u)
            {
                priv::err() << "Failed to create a vertex array object";
                base::abort();
            }

            auto res = perContextVAOIds.try_emplace(glContextId, id);

            const unsigned int vaoId = res.first->second;

            glCheck(glBindVertexArray(vaoId));
            WindowContext::registerUnsharedVAO(glContextId, vaoId);
        }

        vbo.bind();
        ebo.bind();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the OpenGL ID of the VAO for the current context
    ///
    /// Looks up the cached VAO for the active GL context without
    /// creating one. To force lazy creation use `bind()`.
    ///
    /// \return The VAO ID for the active context, or `0` if no VAO has
    ///         been created for this context yet
    ///
    /// \warning Requires a thread-local GL context to be active.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] unsigned int getId() const
    {
        SFML_BASE_ASSERT(WindowContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = WindowContext::getActiveThreadLocalGlContextId();

        const auto* it = perContextVAOIds.find(glContextId);

        if (it != perContextVAOIds.end())
            return it->second;

        return 0u;
    }
};

} // namespace sf
