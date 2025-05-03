#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"

#include "SFML/GLUtils/GLBufferObject.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/AnkerlUnorderedDense.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Manages a group consisting of a VAO, VBO, and EBO.
/// \ingroup glutils
///
/// This helper struct encapsulates the common pattern of using a
/// Vertex Array Object (VAO) together with a Vertex Buffer Object (VBO)
/// and an Element Buffer Object (EBO/IBO).
///
/// The VAO itself is managed per-context (as VAOs are not shareable
/// between contexts), while the VBO and EBO are assumed to be shared.
/// The VAO for a context is created lazily on the first call to `bind()`
/// within that context.
///
////////////////////////////////////////////////////////////
struct GLVAOGroup
{
    ////////////////////////////////////////////////////////////
    mutable ankerl::unordered_dense::map<unsigned int, unsigned int> perContextVAOIds; //!< Vertex array object (unshared, per context)

    ////////////////////////////////////////////////////////////
    GLVertexBufferObject  vbo; //!< Vertex buffer object (shared context)
    GLElementBufferObject ebo; //!< Element index buffer object (shared context)

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor.
    ///
    /// Creates the underlying VBO and EBO.
    ///
    ////////////////////////////////////////////////////////////
    explicit GLVAOGroup() = default;

    ////////////////////////////////////////////////////////////
    ~GLVAOGroup()
    {
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();

        auto* it = perContextVAOIds.find(glContextId);

        if (it != perContextVAOIds.end())
            GraphicsContext::unregisterUnsharedVAO(glContextId, it->second);
    }

    ////////////////////////////////////////////////////////////
    GLVAOGroup(const GLVAOGroup&)            = delete;
    GLVAOGroup& operator=(const GLVAOGroup&) = delete;

    ////////////////////////////////////////////////////////////
    GLVAOGroup(GLVAOGroup&&) noexcept            = default;
    GLVAOGroup& operator=(GLVAOGroup&&) noexcept = default;

    ////////////////////////////////////////////////////////////
    /// \brief Bind the associated VAO, VBO, and EBO for the current context.
    ///
    /// If a VAO doesn't exist for the currently active OpenGL context,
    /// it is created and configured here. Then, the VAO is bound,
    /// followed by the VBO and EBO.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void bind() const
    {
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();

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
                return;
            }

            auto res = perContextVAOIds.try_emplace(glContextId, id);

            const unsigned int vaoId = res.first->second;

            glCheck(glBindVertexArray(vaoId));
            GraphicsContext::registerUnsharedVAO(glContextId, vaoId);
        }

        vbo.bind();
        ebo.bind();
    }

    ////////////////////////////////////////////////////////////
    /// \brief Get the OpenGL ID of the VAO for the current context.
    ///
    /// \return The VAO ID for the active context, or `0` if no VAO has been
    ///         created for this context yet (e.g., if `bind()` hasn't been called).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] unsigned int getId() const
    {
        SFML_BASE_ASSERT(GraphicsContext::hasActiveThreadLocalGlContext());
        const unsigned int glContextId = GraphicsContext::getActiveThreadLocalGlContextId();

        const auto* it = perContextVAOIds.find(glContextId);

        if (it != perContextVAOIds.end())
            return it->second;

        return 0u;
    }
};

} // namespace sf
