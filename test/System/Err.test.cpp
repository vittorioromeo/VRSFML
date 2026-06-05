#include "Tst/Tst.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Base/SizeT.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"


namespace
{
////////////////////////////////////////////////////////////
struct CapturingErrSink
{
    za::String captured;

    static void append(void* ctx, const char* data, za::SizeT size)
    {
        static_cast<CapturingErrSink*>(ctx)->captured.append(data, size);
    }
};


////////////////////////////////////////////////////////////
struct ScopedErrSink
{
    explicit ScopedErrSink(CapturingErrSink& sink)
    {
        za::priv::setErrSink(&CapturingErrSink::append, &sink);
    }

    ~ScopedErrSink()
    {
        za::priv::setErrSink(nullptr, nullptr); // restore default
    }

    ScopedErrSink(const ScopedErrSink&)            = delete;
    ScopedErrSink& operator=(const ScopedErrSink&) = delete;
};
} // namespace


////////////////////////////////////////////////////////////
TEST_CASE("[System] priv::errMsg")
{
    SECTION("errMsg writes prefixed content + trailing newline through the installed sink")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        za::priv::errMsg("hello {}", 42);

        CHECK(sink.captured.find("[[ZANCLE ERROR]]: hello 42") != za::String::nPos);
        // Single-line emission ends with a newline.
        CHECK(sink.captured.find('\n') != za::String::nPos);
    }

    SECTION("errMsgMulti omits the trailing newline + stack trace")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        za::priv::errMsgMulti("partial message no trailing");

        CHECK(sink.captured == za::String{"[[ZANCLE ERROR]]: partial message no trailing"});
    }

    SECTION("ErrMsgScope composes multiple format calls into one emission")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        {
            za::priv::ErrMsgScope scope;
            scope.disableTrailing();
            scope.fmt("line A {}", 1);
            scope.append("\n");
            scope.fmt("line B {}", 2);
        }

        CHECK(sink.captured == za::String{"[[ZANCLE ERROR]]: line A 1\nline B 2"});
    }

    SECTION("Setting a null sink restores the default (writes to stderr; just verifies it doesn't crash)")
    {
        za::priv::setErrSink(nullptr, nullptr);
        // No observable assertion -- this section guarantees the default path exists.
    }

    SECTION("Output larger than the staging buffer goes through the heap path")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        za::String big;
        for (int i = 0; i < 100; ++i)
            big += za::String{"0123456789"}; // 1000 chars

        za::priv::errMsgMulti("{}", za::StringView{big.data(), big.size()});

        CHECK(sink.captured.find("[[ZANCLE ERROR]]: ") != za::String::nPos);
        CHECK(sink.captured.size() >= big.size());
    }
}
