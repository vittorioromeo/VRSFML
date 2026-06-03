#include "StringifySfBaseStringUtil.hpp"
#include "StringifyStringViewUtil.hpp"
#include "Tst/Tst.hpp"

#include "SFML/Base/StringView.hpp"

#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringViewSplits.hpp" // IWYU pragma: keep
#include "SFML/Base/Trait/IsAggregate.hpp"
#include "SFML/Base/Trait/IsStandardLayout.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Vector.hpp"


TEST_CASE("[Base] Base/StringView.hpp")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::base::StringView));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::base::StringView));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::base::StringView));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::base::StringView));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::StringView)); // because of member initializers
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::base::StringView));
        STATIC_CHECK(!SFML_BASE_IS_AGGREGATE(sf::base::StringView));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::StringView));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::base::StringView));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::base::StringView, sf::base::StringView));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::StringView));
    }

    //----------------------------------------------------------------------------

    SECTION("Structured bindings")
    {
        sf::base::StringView span{nullptr, 0u};

        auto [data, size] = span;

        CHECK(data == nullptr);
        CHECK(size == 0u);
    }

    //----------------------------------------------------------------------------

    SECTION("Default constructor")
    {
        sf::base::StringView view;

        REQUIRE(view.empty());
        REQUIRE(view.size() == 0);
        REQUIRE(view.data() == nullptr);
    }

    //----------------------------------------------------------------------------

    SECTION("Constructor with sf::base::String")
    {
        sf::base::String     str  = "Hello xyz";
        sf::base::StringView view = str;

        REQUIRE_FALSE(view.empty());
        REQUIRE(view.size() != 0);
        REQUIRE(view.data() == str.data());
    }

    //----------------------------------------------------------------------------

    SECTION("Constructor with const char*")
    {
        const char* emptyStr    = "";
        const char* nonEmptyStr = "Hello World";

        {
            sf::base::StringView view = emptyStr;
            REQUIRE(view.empty());
        }

        {
            sf::base::StringView view = nonEmptyStr;
            REQUIRE_FALSE(view.empty());
        }

        {
            sf::base::StringView view = emptyStr;
            REQUIRE(view.size() == 0);
        }

        {
            sf::base::StringView view = nonEmptyStr;
            REQUIRE(view.size() != 0);
        }

        {
            sf::base::StringView view = nonEmptyStr;
            REQUIRE(view.data() == nonEmptyStr);
        }
    }

    //----------------------------------------------------------------------------

    SECTION("Constructor with const char array")
    {
        sf::base::StringView empty = "";
        sf::base::StringView view  = "Hello xyz";

        REQUIRE(empty.empty());
        REQUIRE_FALSE(view.empty());
        REQUIRE(empty.size() == 0);
        REQUIRE(view.size() != 0);
    }

    //----------------------------------------------------------------------------
    // Capacity
    //----------------------------------------------------------------------------

    SECTION("Size method")
    {
        const char*          str  = "Hello World";
        sf::base::StringView view = str;

        SECTION("Non-zero size")
        {
            REQUIRE(view.size() == SFML_BASE_STRLEN(str));
        }

        SECTION("Zero size")
        {
            view = "";

            REQUIRE(view.size() == 0);
        }
    }

    //----------------------------------------------------------------------------

    SECTION("Empty method")
    {
        const char*          str  = "Hello World";
        sf::base::StringView view = str;

        SECTION("Non-empty string")
        {
            REQUIRE_FALSE(view.empty());
        }

        SECTION("Empty string")
        {
            view = "";

            REQUIRE(view.empty());
        }
    }

    //----------------------------------------------------------------------------
    // Element Access
    //----------------------------------------------------------------------------

    SECTION("Data method")
    {
        const char*          str  = "Hello World";
        sf::base::StringView view = str;

        REQUIRE(view.data() == str);
    }

    //----------------------------------------------------------------------------

    SECTION("Operator[] method")
    {
        const char*          str  = "Hello World";
        sf::base::StringView view = str;

        REQUIRE(view.data() == str);
    }

    SECTION("Substring method")
    {
        sf::base::StringView view = "Abcde Fghil";

        REQUIRE(view.substrByPosLen() == "Abcde Fghil");
        REQUIRE(view.substrByPosLen(6) == "Fghil");
        REQUIRE(view.substrByPosLen(6, 1) == "F");
        REQUIRE(view.substrByPosLen(6, 10) == "Fghil");
    }

    SECTION("Prefix removal method")
    {
        sf::base::StringView view = "AbcdeXFghil";
        view.removePrefix(6);
        REQUIRE(view == "Fghil");
    }

    SECTION("Suffix removal method")
    {
        sf::base::StringView view = "AbcdeXFghil";
        view.removeSuffix(6);
        REQUIRE(view == "Abcde");
    }

    //----------------------------------------------------------------------------

    SECTION("Find method")
    {
        SECTION("Empty view")
        {
            const auto sv = sf::base::StringView{""};

            SECTION("Empty argument")
            {
                const auto res = sv.find("");
                REQUIRE(res == 0u);
            }

            SECTION("Non-empty argument")
            {
                SECTION("Offset out of bounds")
                {
                    const auto res = sv.find("hello", 100);
                    REQUIRE(res == sf::base::StringView::nPos);
                }
            }
        }

        SECTION("Non-empty view")
        {
            const auto sv = sf::base::StringView{"hello hello xyz"};
            SECTION("Empty argument")
            {
                SECTION("Offset in bounds")
                {
                    const auto res = sv.find("", 5);
                    REQUIRE(res == 5u);
                }

                SECTION("Offset out of bounds")
                {
                    const auto res = sv.find("", 100);
                    REQUIRE(res == sf::base::StringView::nPos);
                }
            }

            SECTION("Argument in string, offset 0")
            {
                SECTION("Start of string")
                {
                    const auto res = sv.find("hello");
                    REQUIRE(res == 0u);
                }

                SECTION("End of string")
                {
                    const auto res = sv.find("xyz");
                    REQUIRE(res == 12u);
                }

                SECTION("Middle of string")
                {
                    const auto res = sv.find(" ");
                    REQUIRE(res == 5u);
                }
            }

            SECTION("Argument in string, offset in bounds")
            {
                const auto res = sv.find("hello", 5);
                REQUIRE(res == 6u);
            }

            SECTION("Argument is string (identity)")
            {
                const auto res = sv.find(sv);
                REQUIRE(res == 0u);
            }

            SECTION("Argument is string, offset in bounds, no match")
            {
                sf::base::StringView s1 = "01234567890ABCDEFGHIJ";
                sf::base::StringView s2 = "01";

                auto res = s1.find(s2);
                SECTION("Prefix find")
                {
                    REQUIRE(res != sf::base::StringView::nPos);
                    REQUIRE(s1.size() > 10);
                }

                auto res1 = s1.find(s2, 10);
                SECTION("Non-matching substring")
                {
                    REQUIRE(res1 == sf::base::StringView::nPos);
                }
            }
        }
    }

    SECTION("Rfind method")
    {
        SECTION("Empty view")
        {
            const auto sv = sf::base::StringView{""};
            SECTION("Empty argument")
            {
                const auto res = sv.rfind("");
                REQUIRE(res == 0u);
            }

            SECTION("Non-empty argument")
            {
                SECTION("Offset out of bounds")
                {
                    const auto res = sv.rfind("hello", 100);
                    REQUIRE(res == sf::base::StringView::nPos);
                }
            }
        }

        SECTION("Non-empty view")
        {
            const auto sv = sf::base::StringView{"hello xyz xyz"};
            SECTION("Empty argument")
            {
                SECTION("Offset in bounds")
                {
                    const auto res = sv.rfind("", 5);
                    REQUIRE(res == 5u);
                }

                SECTION("Offset out of bounds")
                {
                    const auto res = sv.rfind("", 100);
                    REQUIRE(res == sv.size());
                }
            }

            SECTION("Argument in string, offset at end")
            {
                SECTION("Start of string")
                {
                    const auto res = sv.rfind("hello");
                    REQUIRE(res == 0u);
                }

                SECTION("End of string")
                {
                    const auto res = sv.rfind("xyz");
                    REQUIRE(res == 10u);
                }

                SECTION("Middle of string")
                {
                    const auto res = sv.rfind(" ");
                    REQUIRE(res == 9u);
                }
            }

            SECTION("Argument in string, offset in bounds")
            {
                const auto res = sv.rfind("xyz", 8u);
                REQUIRE(res == 6u);
            }

            SECTION("Argument is string (identity)")
            {
                const auto res = sv.rfind(sv);
                REQUIRE(res == 0u);
            }

            SECTION("Other Tests")
            {
                {
                    const sf::base::StringView xsv{" () abc.def.x () "};
                    REQUIRE(xsv.rfind(".") == 11);
                }

                {
                    const sf::base::StringView xsv{
                        "auto sf::base::minipfr::priv::nameOfFieldImpl() [MsvcWorkaround = (anonymous namespace)::S1, "
                        "ptr = ClangWrapper<const int *>{&fakeObjectImpl.value.i}]"};

                    const sf::base::StringView untilRuntime{"."};
                    REQUIRE(xsv.rfind(untilRuntime) != sf::base::StringView::nPos);
                }
            }
        }

        SECTION("FindFirstOf method")
        {
            SECTION("Empty view")
            {
                const auto sv = sf::base::StringView{""};
                SECTION("Empty characters")
                {
                    SECTION("Out-of-bounds position")
                    {
                        const auto res = sv.findFirstOf("", 100);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }

                    SECTION("In-bounds position")
                    {
                        const auto res = sv.findFirstOf("", 0);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }
                }
            }

            SECTION("Non-empty view")
            {
                const auto sv = sf::base::StringView{"Hello xyz"};

                SECTION("Empty characters")
                {
                    REQUIRE(sv.findFirstOf("", 100) == sf::base::StringView::nPos);
                    REQUIRE(sv.findFirstOf("", 5) == sf::base::StringView::nPos);
                }

                SECTION("Non-empty characters")
                {
                    REQUIRE(sv.findFirstOf("y", 4) == 7u);
                    REQUIRE(sv.findFirstOf("zH!") == 0u);
                    REQUIRE(sv.findFirstOf("zd~") == (sv.size() - 1));
                    REQUIRE(sv.findFirstOf("5 2_") == 5u);
                    REQUIRE(sv.findFirstOf("l15") == 2u);

                    REQUIRE(sv.findFirstOf("12-09'") == sf::base::StringView::nPos);
                }
            }
        }

        SECTION("FindFirstNotOf method")
        {
            SECTION("Empty view")
            {
                const auto sv = sf::base::StringView{""};
                SECTION("Empty characters")
                {
                    SECTION("Out-of-bounds position")
                    {
                        const auto res = sv.findFirstNotOf("", 100);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }

                    SECTION("In-bounds position")
                    {
                        const auto res = sv.findFirstNotOf("", 0);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }
                }
            }

            SECTION("Non-empty view")
            {
                const auto sv = sf::base::StringView{"Hello xyz"};

                SECTION("Empty characters")
                {
                    REQUIRE(sv.findFirstOf("y", 5) == 7u);
                    REQUIRE(sv.findFirstNotOf("", 100) == sf::base::StringView::nPos);
                    REQUIRE(sv.findFirstNotOf("", 5) == 5u);
                }

                SECTION("Non-empty characters")
                {
                    SECTION("Characters in string")
                    {
                        REQUIRE(sv.findFirstNotOf("elo") == 0u);
                        REQUIRE(sv.findFirstNotOf("Helo wr") == 6u);
                        REQUIRE(sv.findFirstNotOf("Helowrd") == 5u);
                        REQUIRE(sv.findFirstNotOf("Heo") == 2u);
                    }

                    REQUIRE(sv.findFirstNotOf("123") == 0u);
                }
            }
        }

        SECTION("findLastOf method")
        {
            SECTION("Empty view")
            {
                const auto sv = sf::base::StringView{""};
                SECTION("Empty characters")
                {
                    SECTION("Out-of-bounds position")
                    {
                        const auto res = sv.findLastOf("", 100);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }

                    SECTION("In-bounds position")
                    {
                        const auto res = sv.findLastOf("", 0);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }
                }
            }

            SECTION("Non-empty view")
            {
                const auto sv = sf::base::StringView{"Hello xyz"};

                SECTION("Empty characters")
                {
                    SECTION("Out-of-bounds position")
                    {
                        const auto res = sv.findLastOf("", 100);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }

                    SECTION("In-bounds position")
                    {
                        const auto res = sv.findLastOf("", 5);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }
                }

                SECTION("Non-empty characters")
                {
                    SECTION("Characters in string")
                    {
                        SECTION("Offset by index")
                        {
                            const auto res = sv.findLastOf("l", 5);
                            REQUIRE(res == 3u);
                        }

                        SECTION("Match at beginning")
                        {
                            const auto res = sv.findLastOf("zH!");
                            REQUIRE(res == 8u);
                        }

                        SECTION("Match at end")
                        {
                            const auto res = sv.findLastOf("zd~");
                            REQUIRE(res == (sv.size() - 1));
                        }

                        SECTION("Match in middle")
                        {
                            const auto res = sv.findLastOf("5 2_");
                            REQUIRE(res == 5u);
                        }

                        SECTION("Duplicate match")
                        {
                            const auto res = sv.findLastOf("l15");
                            REQUIRE(res == 3u);
                        }
                    }

                    SECTION("Characters not in string")
                    {
                        const auto res = sv.findLastOf("12-09'");
                        REQUIRE(res == sf::base::StringView::nPos);
                    }
                }
            }
        }

        SECTION("FindLastNotOf method")
        {
            SECTION("Empty view")
            {
                const auto sv = sf::base::StringView{""};
                SECTION("Empty characters")
                {
                    SECTION("Out-of-bounds position")
                    {
                        const auto res = sv.findLastNotOf("", 100);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }

                    SECTION("In-bounds position")
                    {
                        const auto res = sv.findLastNotOf("", 0);
                        REQUIRE(res == sf::base::StringView::nPos);
                    }
                }
            }

            SECTION("Non-empty view")
            {
                const auto sv = sf::base::StringView{"Hello xyz"};

                SECTION("Empty characters")
                {
                    SECTION("Offset by index")
                    {
                        const auto res = sv.findLastNotOf("l", 5);
                        REQUIRE(res == 5u);
                    }

                    SECTION("Out-of-bounds position")
                    {
                        const auto res = sv.findLastNotOf("", 100);
                        REQUIRE(res == (sv.size() - 1));
                    }

                    SECTION("In-bounds position")
                    {
                        const auto res = sv.findLastNotOf("", 5);
                        REQUIRE(res == 5u);
                    }
                }

                SECTION("Non-empty characters")
                {
                    SECTION("Characters not in string")
                    {
                        const auto res = sv.findLastNotOf("123");
                        REQUIRE(res == (sv.size() - 1));
                    }
                }
            }
        }

        //----------------------------------------------------------------------------

        SECTION("Equality operator")
        {
            sf::base::StringView view = "Abcdef";

            SECTION("Equal cases")
            {
                {
                    sf::base::StringView view2 = "Abcdef";
                    REQUIRE(view == view2);
                }

                REQUIRE("Abcdef" == view);
                REQUIRE(view == "Abcdef");

                {
                    const char* str = "Abcdef";
                    REQUIRE(str == view);
                }

                {
                    const char* str = "Abcdef";
                    REQUIRE(view == str);
                }

                {
                    sf::base::String str = "Abcdef";
                    REQUIRE(str == view);
                }

                {
                    sf::base::String str = "Abcdef";
                    REQUIRE(view == str);
                }
            }

            SECTION("Not equal cases")
            {
                {
                    sf::base::StringView view2 = "Xyzw";
                    REQUIRE_FALSE(view == view2);
                }

                REQUIRE_FALSE("Xyzw" == view);
                REQUIRE_FALSE(view == "Xyzw");

                {
                    const char* str = "Xyzw";
                    REQUIRE_FALSE(str == view);
                }

                {
                    const char* str = "Xyzw";
                    REQUIRE_FALSE(view == str);
                }

                {
                    sf::base::String str = "Xyzw";
                    REQUIRE_FALSE(str == view);
                }

                {
                    sf::base::String str = "Xyzw";
                    REQUIRE_FALSE(view == str);
                }
            }
        }

        //----------------------------------------------------------------------------

        SECTION("Inequality operator")
        {
            sf::base::StringView view = "Abcdef";

            SECTION("Equal cases")
            {
                {
                    sf::base::StringView view2 = "Abcdef";
                    REQUIRE_FALSE(view != view2);
                }

                REQUIRE_FALSE("Abcdef" != view);
                REQUIRE_FALSE(view != "Abcdef");

                {
                    const char* str = "Abcdef";
                    REQUIRE_FALSE(str != view);
                }

                {
                    const char* str = "Abcdef";
                    REQUIRE_FALSE(view != str);
                }

                {
                    sf::base::String str = "Abcdef";
                    REQUIRE_FALSE(str != view);
                }

                {
                    sf::base::String str = "Abcdef";
                    REQUIRE_FALSE(view != str);
                }
            }

            SECTION("Not equal cases")
            {
                {
                    sf::base::StringView view2 = "Xyzw";
                    REQUIRE(view != view2);
                }

                REQUIRE("Xyzw" != view);
                REQUIRE(view != "Xyzw");

                {
                    const char* str = "Xyzw";
                    REQUIRE(str != view);
                }

                {
                    const char* str = "Xyzw";
                    REQUIRE(view != str);
                }

                {
                    sf::base::String str = "Xyzw";
                    REQUIRE(str != view);
                }

                {
                    sf::base::String str = "Xyzw";
                    REQUIRE(view != str);
                }
            }
        }
    }

    SECTION("startsWith method")
    {
        SECTION("Empty view")
        {
            const auto sv = sf::base::StringView{""};

            CHECK(sv.startsWith(sf::base::StringView{""}));
            CHECK_FALSE(sv.startsWith(sf::base::StringView{"a"}));
            CHECK_FALSE(sv.startsWith('a'));
            CHECK(sv.startsWith(""));
            CHECK_FALSE(sv.startsWith("hello"));
        }

        SECTION("Non-empty view")
        {
            const auto sv = sf::base::StringView{"Hello, world!"};

            SECTION("StringView prefix")
            {
                CHECK(sv.startsWith(sf::base::StringView{""}));
                CHECK(sv.startsWith(sf::base::StringView{"H"}));
                CHECK(sv.startsWith(sf::base::StringView{"Hello"}));
                CHECK(sv.startsWith(sf::base::StringView{"Hello, world!"}));
                CHECK_FALSE(sv.startsWith(sf::base::StringView{"hello"})); // case-sensitive
                CHECK_FALSE(sv.startsWith(sf::base::StringView{"world"}));
                CHECK_FALSE(sv.startsWith(sf::base::StringView{"Hello, world!!"})); // longer than view
            }

            SECTION("char prefix")
            {
                CHECK(sv.startsWith('H'));
                CHECK_FALSE(sv.startsWith('h'));
                CHECK_FALSE(sv.startsWith('!'));
            }

            SECTION("const char* prefix")
            {
                CHECK(sv.startsWith(""));
                CHECK(sv.startsWith("Hello"));
                CHECK_FALSE(sv.startsWith("Hello, world!!"));
                CHECK_FALSE(sv.startsWith("xyz"));
            }
        }
    }

    SECTION("endsWith method")
    {
        SECTION("Empty view")
        {
            const auto sv = sf::base::StringView{""};

            CHECK(sv.endsWith(sf::base::StringView{""}));
            CHECK_FALSE(sv.endsWith(sf::base::StringView{"a"}));
            CHECK_FALSE(sv.endsWith('a'));
            CHECK(sv.endsWith(""));
            CHECK_FALSE(sv.endsWith("hello"));
        }

        SECTION("Non-empty view")
        {
            const auto sv = sf::base::StringView{"Hello, world!"};

            SECTION("StringView suffix")
            {
                CHECK(sv.endsWith(sf::base::StringView{""}));
                CHECK(sv.endsWith(sf::base::StringView{"!"}));
                CHECK(sv.endsWith(sf::base::StringView{"world!"}));
                CHECK(sv.endsWith(sf::base::StringView{"Hello, world!"}));
                CHECK_FALSE(sv.endsWith(sf::base::StringView{"World!"}));         // case-sensitive
                CHECK_FALSE(sv.endsWith(sf::base::StringView{"Hello"}));          // not a suffix
                CHECK_FALSE(sv.endsWith(sf::base::StringView{"!Hello, world!"})); // longer than view
            }

            SECTION("char suffix")
            {
                CHECK(sv.endsWith('!'));
                CHECK_FALSE(sv.endsWith('?'));
                CHECK_FALSE(sv.endsWith('H'));
            }

            SECTION("const char* suffix")
            {
                CHECK(sv.endsWith(""));
                CHECK(sv.endsWith("world!"));
                CHECK_FALSE(sv.endsWith("World!"));
                CHECK_FALSE(sv.endsWith("xyz"));
            }
        }
    }

    SECTION("contains method")
    {
        SECTION("Empty view")
        {
            const auto sv = sf::base::StringView{""};

            CHECK(sv.contains(sf::base::StringView{""})); // empty needle in empty view
            CHECK_FALSE(sv.contains(sf::base::StringView{"a"}));
            CHECK_FALSE(sv.contains('a'));
            CHECK(sv.contains(""));
            CHECK_FALSE(sv.contains("hello"));
        }

        SECTION("Non-empty view")
        {
            const auto sv = sf::base::StringView{"Hello, world!"};

            SECTION("StringView needle")
            {
                CHECK(sv.contains(sf::base::StringView{""}));
                CHECK(sv.contains(sf::base::StringView{"Hello"}));         // start
                CHECK(sv.contains(sf::base::StringView{", "}));            // middle
                CHECK(sv.contains(sf::base::StringView{"world!"}));        // end
                CHECK(sv.contains(sf::base::StringView{"Hello, world!"})); // exact
                CHECK_FALSE(sv.contains(sf::base::StringView{"World"}));   // case-sensitive
                CHECK_FALSE(sv.contains(sf::base::StringView{"foo"}));
                CHECK_FALSE(sv.contains(sf::base::StringView{"Hello, world!!"})); // longer than view
            }

            SECTION("char needle")
            {
                CHECK(sv.contains('H'));
                CHECK(sv.contains(','));
                CHECK(sv.contains('!'));
                CHECK_FALSE(sv.contains('?'));
                CHECK_FALSE(sv.contains('Z'));
            }

            SECTION("const char* needle")
            {
                CHECK(sv.contains(""));
                CHECK(sv.contains("world"));
                CHECK_FALSE(sv.contains("World"));
                CHECK_FALSE(sv.contains("xyz"));
            }
        }

        SECTION("contains is consistent with find")
        {
            const auto sv = sf::base::StringView{"foobarbaz"};

            CHECK(sv.contains("bar"));
            CHECK(sv.find("bar") != sf::base::StringView::nPos);

            CHECK_FALSE(sv.contains("qux"));
            CHECK(sv.find("qux") == sf::base::StringView::nPos);
        }
    }

    SECTION("constexpr evaluation of startsWith / endsWith / contains")
    {
        // Ensure the new methods are usable in constant expressions.
        constexpr sf::base::StringView sv = "Hello";
        STATIC_CHECK(sv.startsWith("He"));
        STATIC_CHECK(sv.startsWith('H'));
        STATIC_CHECK(sv.endsWith("lo"));
        STATIC_CHECK(sv.endsWith('o'));
        STATIC_CHECK(sv.contains("ell"));
        STATIC_CHECK(sv.contains('l'));
        STATIC_CHECK(!sv.startsWith("xyz"));
        STATIC_CHECK(!sv.endsWith("xyz"));
        STATIC_CHECK(!sv.contains("xyz"));
    }

    SECTION("forLines method")
    {
        const auto collect = [](sf::base::StringView sv)
        {
            sf::base::Vector<sf::base::String> out;
            sv.forLines([&](sf::base::StringView line) { out.emplaceBack(line); });
            return out;
        };

        SECTION("Empty view yields no lines")
        {
            const auto lines = collect(sf::base::StringView{""});
            CHECK(lines.empty());
        }

        SECTION("Single line without trailing newline")
        {
            const auto lines = collect(sf::base::StringView{"hello"});
            REQUIRE(lines.size() == 1u);
            CHECK(lines[0] == "hello");
        }

        SECTION("Single line with trailing newline -- no extra empty line")
        {
            const auto lines = collect(sf::base::StringView{"hello\n"});
            REQUIRE(lines.size() == 1u);
            CHECK(lines[0] == "hello");
        }

        SECTION("Multiple lines without trailing newline")
        {
            const auto lines = collect(sf::base::StringView{"alpha\nbeta\ngamma"});
            REQUIRE(lines.size() == 3u);
            CHECK(lines[0] == "alpha");
            CHECK(lines[1] == "beta");
            CHECK(lines[2] == "gamma");
        }

        SECTION("Multiple lines with trailing newline")
        {
            const auto lines = collect(sf::base::StringView{"alpha\nbeta\ngamma\n"});
            REQUIRE(lines.size() == 3u);
            CHECK(lines[0] == "alpha");
            CHECK(lines[1] == "beta");
            CHECK(lines[2] == "gamma");
        }

        SECTION("Leading newline produces empty first line")
        {
            const auto lines = collect(sf::base::StringView{"\nbeta"});
            REQUIRE(lines.size() == 2u);
            CHECK(lines[0].empty());
            CHECK(lines[1] == "beta");
        }

        SECTION("Consecutive newlines preserve empty middle lines")
        {
            const auto lines = collect(sf::base::StringView{"alpha\n\ngamma"});
            REQUIRE(lines.size() == 3u);
            CHECK(lines[0] == "alpha");
            CHECK(lines[1].empty());
            CHECK(lines[2] == "gamma");
        }

        SECTION("Single newline yields one empty line")
        {
            const auto lines = collect(sf::base::StringView{"\n"});
            REQUIRE(lines.size() == 1u);
            CHECK(lines[0].empty());
        }

        SECTION("Two newlines yield two empty lines")
        {
            const auto lines = collect(sf::base::StringView{"\n\n"});
            REQUIRE(lines.size() == 2u);
            CHECK(lines[0].empty());
            CHECK(lines[1].empty());
        }

        SECTION("CRLF line endings keep the \\r in the line view")
        {
            // Documents the deliberate "split on \\n, do not strip \\r" choice.
            const auto lines = collect(sf::base::StringView{"alpha\r\nbeta"});
            REQUIRE(lines.size() == 2u);
            CHECK(lines[0] == "alpha\r");
            CHECK(lines[1] == "beta");
        }

        SECTION("Lines view into the original buffer (no allocation)")
        {
            const char                 source[] = "first\nsecond\nthird";
            const sf::base::StringView sv{source, sizeof(source) - 1};

            sf::base::SizeT count = 0u;
            sv.forLines([&](sf::base::StringView line)
            {
                // Each `line.data()` must be a pointer into `source`.
                CHECK(line.data() >= source);
                CHECK(line.data() <= source + sizeof(source));
                ++count;
            });
            CHECK(count == 3u);
        }

        SECTION("Forwards mutable lambda")
        {
            sf::base::SizeT total = 0u;
            sf::base::StringView{"a\nbb\nccc"}.forLines([&](sf::base::StringView line) mutable { total += line.size(); });
            CHECK(total == 6u);
        }
    }

    SECTION("forSplits method")
    {
        const auto collect = [](sf::base::StringView sv, sf::base::StringView splitter)
        {
            sf::base::Vector<sf::base::String> out;
            sv.forSplits(splitter, [&](sf::base::StringView seg) { out.emplaceBack(seg); });
            return out;
        };

        SECTION("Single-char splitter")
        {
            const auto parts = collect("a,b,c", ",");
            REQUIRE(parts.size() == 3u);
            CHECK(parts[0] == "a");
            CHECK(parts[1] == "b");
            CHECK(parts[2] == "c");
        }

        SECTION("Multi-char splitter")
        {
            const auto parts = collect("foo::bar::baz", "::");
            REQUIRE(parts.size() == 3u);
            CHECK(parts[0] == "foo");
            CHECK(parts[1] == "bar");
            CHECK(parts[2] == "baz");
        }

        SECTION("Empty input yields no segments")
        {
            const auto parts = collect("", ",");
            CHECK(parts.empty());
        }

        SECTION("No splitter occurrence yields the whole input")
        {
            const auto parts = collect("hello", ",");
            REQUIRE(parts.size() == 1u);
            CHECK(parts[0] == "hello");
        }

        SECTION("Trailing splitter does not produce an extra empty segment")
        {
            const auto parts = collect("a,b,", ",");
            REQUIRE(parts.size() == 2u);
            CHECK(parts[0] == "a");
            CHECK(parts[1] == "b");
        }

        SECTION("Leading splitter produces empty first segment")
        {
            const auto parts = collect(",a", ",");
            REQUIRE(parts.size() == 2u);
            CHECK(parts[0].empty());
            CHECK(parts[1] == "a");
        }

        SECTION("Consecutive splitters produce empty middle segments")
        {
            const auto parts = collect("a,,b", ",");
            REQUIRE(parts.size() == 3u);
            CHECK(parts[0] == "a");
            CHECK(parts[1].empty());
            CHECK(parts[2] == "b");
        }

        SECTION("Input is exactly the splitter -> single empty segment")
        {
            const auto parts = collect(",", ",");
            REQUIRE(parts.size() == 1u);
            CHECK(parts[0].empty());
        }

        SECTION("Multi-char splitter, non-overlapping convention")
        {
            // "aaaa" with splitter "aa" -> "" (before pos 0), "" (between pos 0-1
            // and 2-3). Standard non-overlapping: 2 splits -> 2 segments yielded
            // (the trailing one is dropped because it's the empty suffix after
            // the final splitter).
            const auto parts = collect("aaaa", "aa");
            REQUIRE(parts.size() == 2u);
            CHECK(parts[0].empty());
            CHECK(parts[1].empty());
        }

        SECTION("Multi-char splitter at end is consumed cleanly")
        {
            const auto parts = collect("foo::bar::", "::");
            REQUIRE(parts.size() == 2u);
            CHECK(parts[0] == "foo");
            CHECK(parts[1] == "bar");
        }

        SECTION("forLines is consistent with forSplits(\"\\n\")")
        {
            const sf::base::StringView input{"alpha\nbeta\n\ngamma"};

            sf::base::Vector<sf::base::String> viaLines;
            input.forLines([&](sf::base::StringView l) { viaLines.emplaceBack(l); });

            const auto viaSplits = collect(input, "\n");

            REQUIRE(viaLines.size() == viaSplits.size());
            for (sf::base::SizeT i = 0u; i < viaLines.size(); ++i)
                CHECK(viaLines[i] == viaSplits[i]);
        }

        SECTION("Segments view into the original buffer (no allocation)")
        {
            const char                 source[] = "alpha::beta::gamma";
            const sf::base::StringView sv{source, sizeof(source) - 1};

            sf::base::SizeT count = 0u;
            sv.forSplits("::",
                         [&](sf::base::StringView seg)
            {
                CHECK(seg.data() >= source);
                CHECK(seg.data() <= source + sizeof(source));
                ++count;
            });
            CHECK(count == 3u);
        }
    }

    SECTION("forSplits method (char overload)")
    {
        const auto collect = [](sf::base::StringView sv, char splitter)
        {
            sf::base::Vector<sf::base::String> out;
            sv.forSplits(splitter, [&](sf::base::StringView seg) { out.emplaceBack(seg); });
            return out;
        };

        SECTION("Basic split on comma")
        {
            const auto parts = collect("a,b,c", ',');
            REQUIRE(parts.size() == 3u);
            CHECK(parts[0] == "a");
            CHECK(parts[1] == "b");
            CHECK(parts[2] == "c");
        }

        SECTION("Empty input yields no segments")
        {
            const auto parts = collect("", '/');
            CHECK(parts.empty());
        }

        SECTION("No occurrence yields the whole input")
        {
            const auto parts = collect("hello", '|');
            REQUIRE(parts.size() == 1u);
            CHECK(parts[0] == "hello");
        }

        SECTION("Trailing splitter does not produce an extra empty segment")
        {
            const auto parts = collect("a,b,", ',');
            REQUIRE(parts.size() == 2u);
            CHECK(parts[0] == "a");
            CHECK(parts[1] == "b");
        }

        SECTION("Leading splitter produces empty first segment")
        {
            const auto parts = collect(",a", ',');
            REQUIRE(parts.size() == 2u);
            CHECK(parts[0].empty());
            CHECK(parts[1] == "a");
        }

        SECTION("Consecutive splitters produce empty middle segments")
        {
            const auto parts = collect("a,,b", ',');
            REQUIRE(parts.size() == 3u);
            CHECK(parts[0] == "a");
            CHECK(parts[1].empty());
            CHECK(parts[2] == "b");
        }

        SECTION("Input is exactly the splitter -> single empty segment")
        {
            const auto parts = collect(",", ',');
            REQUIRE(parts.size() == 1u);
            CHECK(parts[0].empty());
        }

        SECTION("char and StringView overloads agree on the same input")
        {
            // Cross-check: both overloads must produce identical output for
            // a single-character splitter.
            const sf::base::StringView input{"alpha,,beta,gamma,"};

            sf::base::Vector<sf::base::String> viaChar;
            input.forSplits(',', [&](sf::base::StringView s) { viaChar.emplaceBack(s); });

            sf::base::Vector<sf::base::String> viaView;
            input.forSplits(",", [&](sf::base::StringView s) { viaView.emplaceBack(s); });

            REQUIRE(viaChar.size() == viaView.size());
            for (sf::base::SizeT i = 0u; i < viaChar.size(); ++i)
                CHECK(viaChar[i] == viaView[i]);
        }

        SECTION("forLines uses the char overload internally")
        {
            // Just a smoke test that the char-based forLines path still matches
            // the StringView-splitter behavior on a typical input.
            const sf::base::StringView input{"alpha\nbeta\n\ngamma"};

            sf::base::Vector<sf::base::String> viaLines;
            input.forLines([&](sf::base::StringView l) { viaLines.emplaceBack(l); });

            const auto viaCharSplit = collect(input, '\n');

            REQUIRE(viaLines.size() == viaCharSplit.size());
            for (sf::base::SizeT i = 0u; i < viaLines.size(); ++i)
                CHECK(viaLines[i] == viaCharSplit[i]);
        }
    }

    SECTION("Embedded NUL bytes (memcmp semantics, not strncmp)")
    {
        // Two views that match up to a NUL but differ AFTER it. With the old
        // `strncmp`-based implementation these would compare equal because the
        // comparison stopped at the NUL; with `memcmp` they correctly differ.

        const char raw1[] = {'a', '\0', 'b'};
        const char raw2[] = {'a', '\0', 'c'};

        const sf::base::StringView v1{raw1, 3};
        const sf::base::StringView v2{raw2, 3};

        SECTION("operator== distinguishes bytes past NUL")
        {
            CHECK_FALSE(v1 == v2);
            CHECK(v1 != v2);
            CHECK(v1 == v1);
        }

        SECTION("compare distinguishes bytes past NUL")
        {
            // 'b' (98) vs 'c' (99) -> negative
            CHECK(v1.compare(v2) < 0);
            CHECK(v2.compare(v1) > 0);
            CHECK(v1.compare(v1) == 0);
        }

        SECTION("operator< honours bytes past NUL")
        {
            CHECK(v1 < v2);
            CHECK(v2 > v1);
        }

        SECTION("find finds the right offset across NULs")
        {
            const char                 haystack[] = {'x', 'a', '\0', 'b', 'y', 'a', '\0', 'c', 'z'};
            const sf::base::StringView hv{haystack, 9};

            const char needle1[] = {'a', '\0', 'b'};
            const char needle2[] = {'a', '\0', 'c'};

            CHECK(hv.find(sf::base::StringView{needle1, 3}) == 1u);
            CHECK(hv.find(sf::base::StringView{needle2, 3}) == 5u);
            // The wrong needle must NOT be found at the other position.
            CHECK(hv.find(sf::base::StringView{needle1, 3}, 2u) == sf::base::StringView::nPos);
        }

        SECTION("rfind honours bytes past NUL")
        {
            const char                 haystack[] = {'x', 'a', '\0', 'b', 'y', 'a', '\0', 'b', 'z'};
            const sf::base::StringView hv{haystack, 9};

            const char needle[] = {'a', '\0', 'b'};
            CHECK(hv.rfind(sf::base::StringView{needle, 3}) == 5u);
        }

        SECTION("startsWith honours bytes past NUL")
        {
            const char prefix1[] = {'a', '\0', 'b'};
            const char prefix2[] = {'a', '\0', 'c'};

            CHECK(v1.startsWith(sf::base::StringView{prefix1, 3}));
            CHECK_FALSE(v1.startsWith(sf::base::StringView{prefix2, 3}));
        }

        SECTION("endsWith honours bytes past NUL")
        {
            const char suffix1[] = {'\0', 'b'};
            const char suffix2[] = {'\0', 'c'};

            CHECK(v1.endsWith(sf::base::StringView{suffix1, 2}));
            CHECK_FALSE(v1.endsWith(sf::base::StringView{suffix2, 2}));
        }

        SECTION("contains honours bytes past NUL")
        {
            const char                 haystack[] = {'x', 'a', '\0', 'c', 'y'};
            const sf::base::StringView hv{haystack, 5};

            const char needle1[] = {'a', '\0', 'b'};
            const char needle2[] = {'a', '\0', 'c'};

            CHECK_FALSE(hv.contains(sf::base::StringView{needle1, 3}));
            CHECK(hv.contains(sf::base::StringView{needle2, 3}));
        }

        SECTION("compare empty operands does not deref null pointers")
        {
            // Default-constructed views have `theData == nullptr`; the
            // explicit `minSize == 0` short-circuit in `compare` keeps memcmp
            // from being called with null.
            const sf::base::StringView empty1;
            const sf::base::StringView empty2;
            CHECK(empty1.compare(empty2) == 0);
            CHECK(empty1 == empty2);

            const sf::base::StringView nonEmpty = "x";
            CHECK(empty1.compare(nonEmpty) < 0);
            CHECK(nonEmpty.compare(empty1) > 0);
        }
    }

    SECTION("swap method (D)")
    {
        SECTION("Swaps two non-empty views")
        {
            sf::base::StringView a = "alpha";
            sf::base::StringView b = "betacarotene";

            const auto* const aOldData = a.data();
            const auto* const bOldData = b.data();

            swap(a, b); // ADL-found friend

            CHECK(a == "betacarotene");
            CHECK(b == "alpha");

            // Pointers were swapped (not deep-copied).
            CHECK(a.data() == bOldData);
            CHECK(b.data() == aOldData);
        }

        SECTION("Swap with default-constructed view")
        {
            sf::base::StringView a = "hello";
            sf::base::StringView b;

            swap(a, b); // ADL-found friend

            CHECK(a.empty());
            CHECK(b == "hello");
        }

        SECTION("Self-swap is a no-op")
        {
            sf::base::StringView a = "stable";
            const auto* const    p = a.data();
            const auto           s = a.size();

            swap(a, a); // ADL-found friend

            CHECK(a.data() == p);
            CHECK(a.size() == s);
        }
    }
}
