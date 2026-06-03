#include "Tst/Tst.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


namespace
{
////////////////////////////////////////////////////////////
struct CapturingErrSink
{
    sf::base::String captured;

    static void append(void* ctx, const char* data, sf::base::SizeT size)
    {
        static_cast<CapturingErrSink*>(ctx)->captured.append(data, size);
    }
};


////////////////////////////////////////////////////////////
struct ScopedErrSink
{
    explicit ScopedErrSink(CapturingErrSink& sink)
    {
        sf::priv::setErrSink(&CapturingErrSink::append, &sink);
    }

    ~ScopedErrSink()
    {
        sf::priv::setErrSink(nullptr, nullptr); // restore default
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

        sf::priv::errMsg("hello {}", 42);

        CHECK(sink.captured.find("[[SFML ERROR]]: hello 42") != sf::base::String::nPos);
        // Single-line emission ends with a newline.
        CHECK(sink.captured.find('\n') != sf::base::String::nPos);
    }

    SECTION("errMsgMulti omits the trailing newline + stack trace")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        sf::priv::errMsgMulti("partial message no trailing");

        CHECK(sink.captured == sf::base::String{"[[SFML ERROR]]: partial message no trailing"});
    }

    SECTION("ErrMsgScope composes multiple format calls into one emission")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        {
            sf::priv::ErrMsgScope scope;
            scope.disableTrailing();
            scope.fmt("line A {}", 1);
            scope.append("\n");
            scope.fmt("line B {}", 2);
        }

        CHECK(sink.captured == sf::base::String{"[[SFML ERROR]]: line A 1\nline B 2"});
    }

    SECTION("Setting a null sink restores the default (writes to stderr; just verifies it doesn't crash)")
    {
        sf::priv::setErrSink(nullptr, nullptr);
        // No observable assertion -- this section guarantees the default path exists.
    }

    SECTION("Output larger than the staging buffer goes through the heap path")
    {
        CapturingErrSink    sink;
        const ScopedErrSink scoped{sink};

        sf::base::String big;
        for (int i = 0; i < 100; ++i)
            big += sf::base::String{"0123456789"}; // 1000 chars

        sf::priv::errMsgMulti("{}", sf::base::StringView{big.data(), big.size()});

        CHECK(sink.captured.find("[[SFML ERROR]]: ") != sf::base::String::nPos);
        CHECK(sink.captured.size() >= big.size());
    }
}
