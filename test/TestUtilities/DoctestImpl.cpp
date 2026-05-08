// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

// Compile the doctest implementation in a single translation unit using the
// amalgamated single-header, instead of building the 33-file parts-based
// static library (doctest_with_main).

#undef DOCTEST_CONFIG_USE_STD_HEADERS
#undef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

//
#include "SFML/Config.hpp" // IWYU pragma: keep

#ifdef SFML_SYSTEM_EMSCRIPTEN
    #define SFML_PRIV_ENABLE_DOCTEST_VERBOSE_REPORTER
#endif

#ifdef SFML_PRIV_ENABLE_DOCTEST_VERBOSE_REPORTER
    #include <cstdio>
#endif


////////////////////////////////////////////////////////////
// Reporter that prints each test case's name + source location to stderr at
// start, so when a test aborts hard (e.g. `value()` on a disengaged optional,
// `SFML_BASE_ASSERT`, signal, etc.) the last-printed line identifies the
// failing test. Without this, traces in Emscripten only contain
// `DOCTEST_ANON_FUNC_<n>` with no name attached.
namespace
{
struct ProgressReporter : doctest::IReporter
{
    explicit ProgressReporter(const doctest::ContextOptions&)
    {
    }

    void test_case_start([[maybe_unused]] const doctest::TestCaseData& tc) override
    {
        // NOLINTNEXTLINE(modernize-use-std-print)

#ifdef SFML_PRIV_ENABLE_DOCTEST_VERBOSE_REPORTER
        std::fprintf(stderr, ">>> [doctest] starting: %s (%s:%d)\n", tc.m_name, tc.m_file.c_str(), tc.m_line);
#endif
    }

    void test_case_reenter([[maybe_unused]] const doctest::TestCaseData& tc) override
    {
        // NOLINTNEXTLINE(modernize-use-std-print)
#ifdef SFML_PRIV_ENABLE_DOCTEST_VERBOSE_REPORTER
        std::fprintf(stderr, ">>> [doctest] reentering: %s (%s:%d)\n", tc.m_name, tc.m_file.c_str(), tc.m_line);
#endif
    }

    void subcase_start([[maybe_unused]] const doctest::SubcaseSignature& sc) override
    {
        // NOLINTNEXTLINE(modernize-use-std-print)
#ifdef SFML_PRIV_ENABLE_DOCTEST_VERBOSE_REPORTER
        std::fprintf(stderr, ">>>   [doctest] subcase: %s (%s:%d)\n", sc.m_name.c_str(), sc.m_file, sc.m_line);
#endif
    }

    // clang-format off
    void report_query(const doctest::QueryData&) override { }
    void test_run_start() override { }
    void test_run_end(const doctest::TestRunStats&) override { }
    void test_case_end(const doctest::CurrentTestCaseStats&) override { }
    void test_case_exception(const doctest::TestCaseException&) override { }
    void subcase_end() override { }
    void log_assert(const doctest::AssertData&) override { }
    void log_message(const doctest::MessageData&) override { }
    void test_case_skipped(const doctest::TestCaseData&) override { }
    // clang-format on
};

} // namespace

DOCTEST_REGISTER_REPORTER("progress", 1, ProgressReporter);


////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    doctest::Context ctx(argc, argv);
    ctx.setOption("reporters", "console,progress");
    return ctx.run();
}
