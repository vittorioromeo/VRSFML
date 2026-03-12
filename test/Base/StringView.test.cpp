#include "SFML/Base/StringView.hpp"

#include "SFML/Base/Builtin/Strlen.hpp"
#include "SFML/Base/String.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <StringifySfBaseStringUtil.hpp>
#include <StringifyStringViewUtil.hpp>


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
}
