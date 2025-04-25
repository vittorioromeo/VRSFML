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
/// \brief TODO P1: docs
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
