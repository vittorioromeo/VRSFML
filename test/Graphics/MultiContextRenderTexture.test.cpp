#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/RectangleShapeData.hpp"
#include "SFML/Graphics/RenderTexture.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Vertex.hpp"

#include "SFML/Window/WindowContext.hpp"
#include "SFML/Window/WindowSettings.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>


namespace
{
#if defined(_WIN32)
    #define SFML_TEST_GL_API_PTR __stdcall
#else
    #define SFML_TEST_GL_API_PTR
#endif

using GLenum    = unsigned int;
using GLuint    = unsigned int;
using GLsizei   = int;
using GLboolean = unsigned char;

using PFNGLCHECKFRAMEBUFFERSTATUSPROC = GLenum(SFML_TEST_GL_API_PTR*)(GLenum target);
using PFNGLGENFRAMEBUFFERSPROC        = void(SFML_TEST_GL_API_PTR*)(GLsizei n, GLuint* framebuffers);
using PFNGLISFRAMEBUFFERPROC          = GLboolean(SFML_TEST_GL_API_PTR*)(GLuint framebuffer);

constexpr GLenum GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6u;

extern "C"
{
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus;
extern PFNGLGENFRAMEBUFFERSPROC        glad_glGenFramebuffers;
extern PFNGLISFRAMEBUFFERPROC          glad_glIsFramebuffer;
}


struct ScopedFramebufferHooks
{
    static inline PFNGLCHECKFRAMEBUFFERSTATUSPROC originalCheckFramebufferStatus{};
    static inline PFNGLGENFRAMEBUFFERSPROC        originalGenFramebuffers{};
    static inline sf::base::Vector<unsigned int>* generatedFramebuffers{};
    static inline unsigned int                    checkFramebufferStatusCallCount{};
    static inline bool                            failOnSecondFramebufferStatusCheck{};

    sf::base::Vector<unsigned int> ids;

    ////////////////////////////////////////////////////////////
    static GLenum SFML_TEST_GL_API_PTR checkFramebufferStatusHook(const GLenum target)
    {
        ++checkFramebufferStatusCallCount;

        if (failOnSecondFramebufferStatusCheck && checkFramebufferStatusCallCount == 2u)
            return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;

        return originalCheckFramebufferStatus(target);
    }

    ////////////////////////////////////////////////////////////
    static void SFML_TEST_GL_API_PTR genFramebuffersHook(const GLsizei n, GLuint* const framebuffers)
    {
        originalGenFramebuffers(n, framebuffers);

        if (generatedFramebuffers == nullptr)
            return;

        for (GLsizei i = 0; i < n; ++i)
            generatedFramebuffers->emplaceBack(framebuffers[i]);
    }

    ////////////////////////////////////////////////////////////
    explicit ScopedFramebufferHooks(const bool failSecondFramebufferStatusCheck)
    {
        originalCheckFramebufferStatus         = glad_glCheckFramebufferStatus;
        originalGenFramebuffers                = glad_glGenFramebuffers;
        generatedFramebuffers                  = &ids;
        checkFramebufferStatusCallCount        = 0u;
        failOnSecondFramebufferStatusCheck     = failSecondFramebufferStatusCheck;
        glad_glCheckFramebufferStatus          = &checkFramebufferStatusHook;
        glad_glGenFramebuffers                 = &genFramebuffersHook;
    }

    ////////////////////////////////////////////////////////////
    void stopFailingFramebufferStatusChecks() const
    {
        failOnSecondFramebufferStatusCheck = false;
    }

    ////////////////////////////////////////////////////////////
    ~ScopedFramebufferHooks()
    {
        glad_glCheckFramebufferStatus      = originalCheckFramebufferStatus;
        glad_glGenFramebuffers             = originalGenFramebuffers;
        generatedFramebuffers              = nullptr;
        checkFramebufferStatusCallCount    = 0u;
        failOnSecondFramebufferStatusCheck = false;
    }
};

#undef SFML_TEST_GL_API_PTR
} // namespace


TEST_CASE("[Graphics] MultiContext" * doctest::skip(skipDisplayTests))
{
    sf::Vertex   vertices[]{{.position = {0.f, 0.f}}};
    unsigned int indices[] = {0};

    auto graphicsContext = sf::GraphicsContext::create().value();

    SECTION("Test")
    {
        auto wnd = sf::RenderWindow::create({.size = {1024u, 1024u}, .title = "Window A"}).value();
        wnd.drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
        });

        wnd.display();

        auto rt1 = sf::RenderTexture::create({1024u, 1024u}).value();
        wnd.drawIndexedVertices({
            .vertexData    = vertices,
            .vertexCount   = 1u,
            .indexData     = indices,
            .indexCount    = 1u,
            .primitiveType = sf::PrimitiveType::Points,
        });

        rt1.display();
    }

    SECTION("Test2")
    {
        sf::base::Optional<sf::RenderWindow>  optWnd;
        sf::base::Optional<sf::RenderTexture> optRT0;

        for (int i = 0; i < 2; ++i)
        {
            optRT0.emplace(sf::RenderTexture::create({1024u, 1024u}).value());
            optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0.reset();
            optRT0.emplace(sf::RenderTexture::create({1024u, 1024u}).value());
            optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0->drawIndexedVertices(
                {
                    .vertexData    = vertices,
                    .vertexCount   = 1u,
                    .indexData     = indices,
                    .indexCount    = 1u,
                    .primitiveType = sf::PrimitiveType::Points,
                },
                {.view = optWnd->computeView()});

            optRT0->display();
        }
    }

    SECTION("Test3")
    {
        sf::base::Optional<sf::RenderWindow> optWnd;

        auto rt = sf::RenderTexture::create({1024u, 1024u});

        optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices(
            {
                .vertexData    = vertices,
                .vertexCount   = 1u,
                .indexData     = indices,
                .indexCount    = 1u,
                .primitiveType = sf::PrimitiveType::Points,
            },
            {.view = rt->computeView()});

        rt->display();

        optWnd->drawIndexedVertices(
            {
                .vertexData    = vertices,
                .vertexCount   = 1u,
                .indexData     = indices,
                .indexCount    = 1u,
                .primitiveType = sf::PrimitiveType::Points,
            },
            {.view = optWnd->computeView()});

        optWnd = sf::RenderWindow::create(sf::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices(
            {
                .vertexData    = vertices,
                .vertexCount   = 1u,
                .indexData     = indices,
                .indexCount    = 1u,
                .primitiveType = sf::PrimitiveType::Points,
            },
            {.view = rt->computeView()});

        rt->display();
    }

    SECTION("Test4")
    {
        auto window = sf::RenderWindow::create({.size{1024u, 768u}, .title = "Window C"}).value();

        auto baseRenderTexture = sf::RenderTexture::create({100u, 100u});
        auto leftInnerRT       = sf::RenderTexture::create({100u, 100u});

        leftInnerRT->draw(sf::RectangleShapeData{}, {.view = leftInnerRT->computeView()});
        leftInnerRT->display();
    }

    SECTION("RenderTexture retries replace leaked same-context framebuffer")
    {
        auto renderTexture = sf::RenderTexture::create({64u, 64u});
        REQUIRE(renderTexture.hasValue());

        auto window = sf::RenderWindow::create({.size{64u, 64u}, .title = "Window D"}).value();
        REQUIRE(window.setActive(true));

        ScopedFramebufferHooks framebufferHooks(/* failSecondFramebufferStatusCheck */ true);

        CHECK_FALSE(renderTexture->setActive(true));

        framebufferHooks.stopFailingFramebufferStatusChecks();

        CHECK(renderTexture->setActive(true));
        CHECK(renderTexture->setActive(false));

        REQUIRE(framebufferHooks.ids.size() >= 4u);
        const unsigned int retriedMainFramebuffer = framebufferHooks.ids[2];

        renderTexture.reset();

        REQUIRE(glad_glIsFramebuffer != nullptr);
        CHECK(glad_glIsFramebuffer(retriedMainFramebuffer) == 0u);
    }
}
