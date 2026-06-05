#include "Zancle/Graphics/VertexBuffer.hpp"

#include "Zancle/Graphics/GraphicsContext.hpp"

// Other 1st party headers
#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"

#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/Vertex.hpp"

#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsMoveAssignable.hpp"
#include "Zancle/Trait/IsMoveConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"
#include "Zancle/Trait/IsNothrowSwappable.hpp"


// Skip these tests with [.display] because they produce flakey failures in CI when using xvfb-run
TEST_CASE("[Graphics] za::VertexBuffer", "[.display]")
{
    auto graphicsContext = za::GraphicsContext::create().value();

    SECTION("Type traits")
    {
        STATIC_CHECK(ZA_IS_COPY_CONSTRUCTIBLE(za::VertexBuffer));
        STATIC_CHECK(ZA_IS_COPY_ASSIGNABLE(za::VertexBuffer));
        STATIC_CHECK(ZA_IS_MOVE_CONSTRUCTIBLE(za::VertexBuffer));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::VertexBuffer));
        STATIC_CHECK(ZA_IS_MOVE_ASSIGNABLE(za::VertexBuffer));
        STATIC_CHECK(!ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::VertexBuffer));
        STATIC_CHECK(ZA_IS_NOTHROW_SWAPPABLE(za::VertexBuffer));
    }

    SECTION("Construction")
    {
        SECTION("Default constructor")
        {
            const za::VertexBuffer vertexBuffer;
            CHECK(vertexBuffer.getVertexCount() == 0);
            CHECK(vertexBuffer.getNativeHandle() == 0);
            CHECK(vertexBuffer.getPrimitiveType() == za::PrimitiveType::Points);
            CHECK(vertexBuffer.getUsage() == za::VertexBuffer::Usage::Stream);
        }

        SECTION("Primitive type constructor")
        {
            const za::VertexBuffer vertexBuffer(za::PrimitiveType::Triangles);
            CHECK(vertexBuffer.getVertexCount() == 0);
            CHECK(vertexBuffer.getNativeHandle() == 0);
            CHECK(vertexBuffer.getPrimitiveType() == za::PrimitiveType::Triangles);
            CHECK(vertexBuffer.getUsage() == za::VertexBuffer::Usage::Stream);
        }

        SECTION("Usage constructor")
        {
            const za::VertexBuffer vertexBuffer(za::VertexBuffer::Usage::Static);
            CHECK(vertexBuffer.getVertexCount() == 0);
            CHECK(vertexBuffer.getNativeHandle() == 0);
            CHECK(vertexBuffer.getPrimitiveType() == za::PrimitiveType::Points);
            CHECK(vertexBuffer.getUsage() == za::VertexBuffer::Usage::Static);
        }

        SECTION("Primitive type and usage constructor")
        {
            const za::VertexBuffer vertexBuffer(za::PrimitiveType::LineStrip, za::VertexBuffer::Usage::Dynamic);
            CHECK(vertexBuffer.getVertexCount() == 0);
            CHECK(vertexBuffer.getNativeHandle() == 0);
            CHECK(vertexBuffer.getPrimitiveType() == za::PrimitiveType::LineStrip);
            CHECK(vertexBuffer.getUsage() == za::VertexBuffer::Usage::Dynamic);
        }
    }

    SECTION("Copy semantics")
    {
        const za::VertexBuffer vertexBuffer(za::PrimitiveType::LineStrip, za::VertexBuffer::Usage::Dynamic);

        SECTION("Construction")
        {
            const za::VertexBuffer vertexBufferCopy(vertexBuffer); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(vertexBufferCopy.getVertexCount() == 0);
            CHECK(vertexBufferCopy.getNativeHandle() == 0);
            CHECK(vertexBufferCopy.getPrimitiveType() == za::PrimitiveType::LineStrip);
            CHECK(vertexBufferCopy.getUsage() == za::VertexBuffer::Usage::Dynamic);
        }

        SECTION("Assignment")
        {
            za::VertexBuffer vertexBufferCopy;
            vertexBufferCopy = vertexBuffer;
            CHECK(vertexBufferCopy.getVertexCount() == 0);
            CHECK(vertexBufferCopy.getNativeHandle() == 0);
            CHECK(vertexBufferCopy.getPrimitiveType() == za::PrimitiveType::LineStrip);
            CHECK(vertexBufferCopy.getUsage() == za::VertexBuffer::Usage::Dynamic);
        }
    }

    SECTION("create()")
    {
        za::VertexBuffer vertexBuffer;
        CHECK(vertexBuffer.create(100));
        CHECK(vertexBuffer.getVertexCount() == 100);
    }

    SECTION("update()")
    {
        za::VertexBuffer vertexBuffer;
        za::Vertex       vertices[128]{};

        SECTION("Vertices")
        {
            SECTION("Uninitialized buffer")
            {
                CHECK(!vertexBuffer.update(vertices));
            }

            CHECK(vertexBuffer.create(128));

            SECTION("Null vertices")
            {
                CHECK(!vertexBuffer.update(nullptr));
            }

            CHECK(vertexBuffer.update(vertices));
            CHECK(vertexBuffer.getVertexCount() == 128);
            CHECK(vertexBuffer.getNativeHandle() != 0);
        }

        SECTION("Vertices, count, and offset")
        {
            CHECK(vertexBuffer.create(128));

            SECTION("Count + offset too large")
            {
                CHECK(!vertexBuffer.update(vertices, 100, 100));
            }

            CHECK(vertexBuffer.update(vertices, 128, 0));
            CHECK(vertexBuffer.getVertexCount() == 128);
        }

        SECTION("Another buffer")
        {
            za::VertexBuffer otherVertexBuffer;

            CHECK(!vertexBuffer.update(otherVertexBuffer));
            CHECK(otherVertexBuffer.create(42));
            CHECK(!vertexBuffer.update(otherVertexBuffer));
        }
    }

    SECTION("swap()")
    {
        za::VertexBuffer vertexBuffer1(za::PrimitiveType::LineStrip, za::VertexBuffer::Usage::Dynamic);
        CHECK(vertexBuffer1.create(50));

        za::VertexBuffer vertexBuffer2(za::PrimitiveType::TriangleStrip, za::VertexBuffer::Usage::Stream);
        CHECK(vertexBuffer2.create(60));

        za::swap(vertexBuffer1, vertexBuffer2);

        CHECK(vertexBuffer1.getVertexCount() == 60);
        CHECK(vertexBuffer1.getNativeHandle() != 0);
        CHECK(vertexBuffer1.getPrimitiveType() == za::PrimitiveType::TriangleStrip);
        CHECK(vertexBuffer1.getUsage() == za::VertexBuffer::Usage::Stream);

        CHECK(vertexBuffer2.getVertexCount() == 50);
        CHECK(vertexBuffer2.getNativeHandle() != 0);
        CHECK(vertexBuffer2.getPrimitiveType() == za::PrimitiveType::LineStrip);
        CHECK(vertexBuffer2.getUsage() == za::VertexBuffer::Usage::Dynamic);
    }

    SECTION("Set/get primitive type")
    {
        za::VertexBuffer vertexBuffer;
        vertexBuffer.setPrimitiveType(za::PrimitiveType::TriangleFan);
        CHECK(vertexBuffer.getPrimitiveType() == za::PrimitiveType::TriangleFan);
    }

    SECTION("Set/get usage")
    {
        za::VertexBuffer vertexBuffer;
        vertexBuffer.setUsage(za::VertexBuffer::Usage::Dynamic);
        CHECK(vertexBuffer.getUsage() == za::VertexBuffer::Usage::Dynamic);
    }
}
