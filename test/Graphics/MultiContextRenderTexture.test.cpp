#include "GraphicsUtil.hpp"
#include "StringifyOptionalUtil.hpp"
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Graphics/DrawIndexedVerticesSettings.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/RectangleShapeData.hpp"
#include "Zancle/Graphics/RenderTexture.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/Window/WindowContext.hpp"
#include "Zancle/Window/WindowSettings.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Vector.hpp"


#if defined(_WIN32)
    #define ZA_TEST_GL_API_PTR __stdcall
#else
    #define ZA_TEST_GL_API_PTR
#endif

using GLenum    = unsigned int;
using GLuint    = unsigned int;
using GLsizei   = int;
using GLboolean = unsigned char;

using PFNGLCHECKFRAMEBUFFERSTATUSPROC = GLenum(ZA_TEST_GL_API_PTR*)(GLenum target);
using PFNGLGENFRAMEBUFFERSPROC        = void(ZA_TEST_GL_API_PTR*)(GLsizei n, GLuint* framebuffers);
using PFNGLISFRAMEBUFFERPROC          = GLboolean(ZA_TEST_GL_API_PTR*)(GLuint framebuffer);

extern "C"
{
    extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus; // NOLINT(readability-identifier-naming)
    extern PFNGLGENFRAMEBUFFERSPROC        glad_glGenFramebuffers;        // NOLINT(readability-identifier-naming)
    extern PFNGLISFRAMEBUFFERPROC          glad_glIsFramebuffer;          // NOLINT(readability-identifier-naming)
}


namespace
{
constexpr GLenum kGlFramebufferIncompleteAttachment = 0x8C'D6u; // NOLINT(readability-identifier-naming)


struct ScopedFramebufferHooks
{
    static inline PFNGLCHECKFRAMEBUFFERSTATUSPROC originalCheckFramebufferStatus{};
    static inline PFNGLGENFRAMEBUFFERSPROC        originalGenFramebuffers{};
    static inline zb::Vector<unsigned int>*       generatedFramebuffers{};
    static inline unsigned int                    checkFramebufferStatusCallCount{};
    static inline bool                            failOnSecondFramebufferStatusCheck{};

    zb::Vector<unsigned int> ids;

    ////////////////////////////////////////////////////////////
    static GLenum ZA_TEST_GL_API_PTR checkFramebufferStatusHook(const GLenum target)
    {
        ++checkFramebufferStatusCallCount;

        if (failOnSecondFramebufferStatusCheck && checkFramebufferStatusCallCount == 2u)
            return kGlFramebufferIncompleteAttachment;

        return originalCheckFramebufferStatus(target);
    }

    ////////////////////////////////////////////////////////////
    static void ZA_TEST_GL_API_PTR genFramebuffersHook(const GLsizei n, GLuint* const framebuffers)
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
        originalCheckFramebufferStatus     = glad_glCheckFramebufferStatus;
        originalGenFramebuffers            = glad_glGenFramebuffers;
        generatedFramebuffers              = &ids;
        checkFramebufferStatusCallCount    = 0u;
        failOnSecondFramebufferStatusCheck = failSecondFramebufferStatusCheck;
        glad_glCheckFramebufferStatus      = &checkFramebufferStatusHook;
        glad_glGenFramebuffers             = &genFramebuffersHook;
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

#undef ZA_TEST_GL_API_PTR
} // namespace


// Emscripten/WebGL does not support multiple GL contexts, which this entire
// test case exercises.
#ifndef ZA_SYSTEM_EMSCRIPTEN
TEST_CASE("[Graphics] MultiContext" * tst::skip(skipDisplayTests))
{
    za::Vertex   vertices[]{{.position = {0.f, 0.f}}};
    unsigned int indices[] = {0};

    auto graphicsContext = za::GraphicsContext::create().value();

    SECTION("Test")
    {
        auto wnd = za::RenderWindow::create({.size = {1024u, 1024u}, .title = "Window A"}).value();
        wnd.drawIndexedVertices(za::DrawIndexedVerticesSettings{
            .vertexSpan    = vertices,
            .indexSpan     = indices,
            .primitiveType = za::PrimitiveType::Points,
        });

        wnd.display();

        auto rt1 = za::RenderTexture::create({1024u, 1024u}).value();
        wnd.drawIndexedVertices(za::DrawIndexedVerticesSettings{
            .vertexSpan    = vertices,
            .indexSpan     = indices,
            .primitiveType = za::PrimitiveType::Points,
        });

        rt1.display();
    }

    SECTION("Test2")
    {
        zb::Optional<za::RenderWindow>  optWnd;
        zb::Optional<za::RenderTexture> optRT0;

        for (int i = 0; i < 2; ++i)
        {
            optRT0.emplace(za::RenderTexture::create({1024u, 1024u}).value());
            optWnd = za::RenderWindow::create(za::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0.reset();
            optRT0.emplace(za::RenderTexture::create({1024u, 1024u}).value());
            optWnd = za::RenderWindow::create(za::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

            optRT0->drawIndexedVertices(
                za::DrawIndexedVerticesSettings{
                    .vertexSpan    = vertices,
                    .indexSpan     = indices,
                    .primitiveType = za::PrimitiveType::Points,
                },
                {.view = optWnd->computeView()});

            optRT0->display();
        }
    }

    SECTION("Test3")
    {
        zb::Optional<za::RenderWindow> optWnd;

        auto rt = za::RenderTexture::create({1024u, 1024u});

        optWnd = za::RenderWindow::create(za::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices(
            za::DrawIndexedVerticesSettings{
                .vertexSpan    = vertices,
                .indexSpan     = indices,
                .primitiveType = za::PrimitiveType::Points,
            },
            {.view = rt->computeView()});

        rt->display();

        optWnd->drawIndexedVertices(
            za::DrawIndexedVerticesSettings{
                .vertexSpan    = vertices,
                .indexSpan     = indices,
                .primitiveType = za::PrimitiveType::Points,
            },
            {.view = optWnd->computeView()});

        optWnd = za::RenderWindow::create(za::WindowSettings{.size = {1024u, 1024u}, .title = "Window B"});

        rt->drawIndexedVertices(
            za::DrawIndexedVerticesSettings{
                .vertexSpan    = vertices,
                .indexSpan     = indices,
                .primitiveType = za::PrimitiveType::Points,
            },
            {.view = rt->computeView()});

        rt->display();
    }

    SECTION("Test4")
    {
        auto window = za::RenderWindow::create({.size{1024u, 768u}, .title = "Window C"}).value();

        auto baseRenderTexture = za::RenderTexture::create({100u, 100u});
        auto leftInnerRT       = za::RenderTexture::create({100u, 100u});

        leftInnerRT->draw(za::RectangleShapeData{}, {.view = leftInnerRT->computeView()});
        leftInnerRT->display();
    }

    SECTION("RenderTexture retries replace leaked same-context framebuffer")
    {
        auto renderTexture = za::RenderTexture::create({64u, 64u});
        REQUIRE(renderTexture.hasValue());

        auto window = za::RenderWindow::create({.size{64u, 64u}, .title = "Window D"}).value();
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
#endif
