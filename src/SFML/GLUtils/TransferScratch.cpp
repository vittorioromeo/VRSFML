// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/GLUtils/TransferScratch.hpp"

#include "SFML/Window/WindowContext.hpp"

#include "SFML/GLUtils/GLCheck.hpp"
#include "SFML/GLUtils/GLUtils.hpp"
#include "SFML/GLUtils/Glad.hpp"
#include "SFML/GLUtils/TextureSaver.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/AnkerlUnorderedDense.hpp"
#include "SFML/Base/Assert.hpp"

#include <mutex>


namespace
{
////////////////////////////////////////////////////////////
struct ContextTransferScratch
{
    unsigned int readFramebuffer{};
    unsigned int drawFramebuffer{};

    unsigned int flipFramebuffer{};
    unsigned int flipTexture{};
    sf::Vec2u    flipTextureSize{};
    bool         flipTextureSrgb{};
};


////////////////////////////////////////////////////////////
class TransferScratchManager
{
private:
    std::mutex                                                         m_mutex;
    ankerl::unordered_dense::map<unsigned int, ContextTransferScratch> m_byContext;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getActiveContextId()
    {
        SFML_BASE_ASSERT(sf::WindowContext::hasActiveThreadLocalGlContext());
        return sf::WindowContext::getActiveThreadLocalGlContextId();
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] ContextTransferScratch& getOrCreateForActiveContext()
    {
        const auto [it, inserted] = m_byContext.try_emplace(getActiveContextId());
        return it->second;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int ensureFramebuffer(unsigned int& framebufferId, const char* const what)
    {
        if (framebufferId != 0u)
            return framebufferId;

        glCheck(glGenFramebuffers(1, &framebufferId));

        if (framebufferId == 0u)
            sf::priv::err() << "Failed to create transfer scratch " << what;

        return framebufferId;
    }

    ////////////////////////////////////////////////////////////
    static void deleteFramebuffer(unsigned int& framebufferId)
    {
        if (framebufferId == 0u)
            return;

        glCheck(glDeleteFramebuffers(1, &framebufferId));
        framebufferId = 0u;
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int ensureFlipTexture(ContextTransferScratch& scratch, const sf::Vec2u size, const bool sRgb)
    {
        if (scratch.flipTexture != 0u && scratch.flipTextureSize == size && scratch.flipTextureSrgb == sRgb)
            return scratch.flipTexture;

        if (scratch.flipTexture != 0u)
        {
            glCheck(glDeleteTextures(1, &scratch.flipTexture));
            scratch.flipTexture = 0u;
        }

        const sf::priv::TextureSaver textureSaver;

        glCheck(glGenTextures(1, &scratch.flipTexture));

        if (scratch.flipTexture == 0u)
        {
            sf::priv::err() << "Failed to create transfer scratch flip texture";
            return 0u;
        }

        sf::priv::bindAndInitializeTexture(scratch.flipTexture, sRgb, size, GL_CLAMP_TO_EDGE);

        scratch.flipTextureSize = size;
        scratch.flipTextureSrgb = sRgb;

        return scratch.flipTexture;
    }

    ////////////////////////////////////////////////////////////
    static void deleteTexture(unsigned int& textureId)
    {
        if (textureId == 0u)
            return;

        glCheck(glDeleteTextures(1, &textureId));
        textureId = 0u;
    }

public:
    ////////////////////////////////////////////////////////////
    TransferScratchManager() = default;

    ////////////////////////////////////////////////////////////
    ~TransferScratchManager()
    {
        // Verify that everything is already released for all contexts
        if (!m_byContext.empty())
        {
            sf::priv::err() << "TransferScratchManager destroyed with unreleased resources for " << m_byContext.size()
                            << " contexts";

            sf::base::abort();
        }
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getReadFramebuffer()
    {
        const std::lock_guard lock(m_mutex);

        auto& scratch = getOrCreateForActiveContext();
        return ensureFramebuffer(scratch.readFramebuffer, "read framebuffer");
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getDrawFramebuffer()
    {
        const std::lock_guard lock(m_mutex);

        auto& scratch = getOrCreateForActiveContext();
        return ensureFramebuffer(scratch.drawFramebuffer, "draw framebuffer");
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getFlipFramebuffer()
    {
        const std::lock_guard lock(m_mutex);

        auto& scratch = getOrCreateForActiveContext();
        return ensureFramebuffer(scratch.flipFramebuffer, "flip framebuffer");
    }

    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int ensureFlipTexture(const sf::Vec2u size, const bool sRgb)
    {
        const std::lock_guard lock(m_mutex);

        auto& scratch = getOrCreateForActiveContext();
        return ensureFlipTexture(scratch, size, sRgb);
    }

    ////////////////////////////////////////////////////////////
    void releaseForActiveContext()
    {
        if (!sf::WindowContext::isInstalled() || !sf::WindowContext::hasActiveThreadLocalGlContext())
            return;

        const std::lock_guard lock(m_mutex);

        const unsigned int contextId = getActiveContextId();
        auto*              it        = m_byContext.find(contextId);

        if (it == m_byContext.end())
            return;

        deleteFramebuffer(it->second.readFramebuffer);
        deleteFramebuffer(it->second.drawFramebuffer);

        deleteFramebuffer(it->second.flipFramebuffer);
        deleteTexture(it->second.flipTexture);

        m_byContext.erase(it);
    }
};


////////////////////////////////////////////////////////////
[[nodiscard]] TransferScratchManager& getTransferScratchManager()
{
    static TransferScratchManager result;
    return result;
}

} // namespace


// TODO P0: consider moving to WindowContext
namespace sf::priv
{
////////////////////////////////////////////////////////////
unsigned int getTransferScratchReadFramebuffer()
{
    return getTransferScratchManager().getReadFramebuffer();
}


////////////////////////////////////////////////////////////
unsigned int getTransferScratchDrawFramebuffer()
{
    return getTransferScratchManager().getDrawFramebuffer();
}


////////////////////////////////////////////////////////////
unsigned int getTransferScratchFlipFramebuffer()
{
    return getTransferScratchManager().getFlipFramebuffer();
}


////////////////////////////////////////////////////////////
unsigned int ensureTransferScratchFlipTexture(const Vec2u size, const bool sRgb)
{
    return getTransferScratchManager().ensureFlipTexture(size, sRgb);
}


////////////////////////////////////////////////////////////
void releaseTransferScratchForActiveContext()
{
    getTransferScratchManager().releaseForActiveContext();
}

} // namespace sf::priv
