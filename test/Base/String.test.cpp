#include "SFML/Base/String.hpp"

#include "StringifySfBaseStringUtil.hpp" // used
#include "StringifyStringViewUtil.hpp"   // used

#include "SFML/Base/Algorithm/Copy.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsMoveAssignable.hpp"
#include "SFML/Base/Trait/IsMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTrivial.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"

#include <Doctest.hpp>

#include <string> // For comparison and interoperability tests


TEST_CASE("[Base] Base/String.hpp")
{
    // A string guaranteed to be longer than the SSO buffer
    const char*    longStringLiteral = "This is a long string that will definitely not fit in SSO.";
    constexpr auto maxSsoSize        = sf::base::String{}.capacity();

    SECTION("Type traits")
    {
        // String has manual resource management, so it should not be trivial.
        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::base::String));
        STATIC_CHECK(!SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::base::String));

        // But it should be fully copyable and movable.
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::base::String));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::base::String));
        STATIC_CHECK(SFML_BASE_IS_MOVE_CONSTRUCTIBLE(sf::base::String));
        STATIC_CHECK(SFML_BASE_IS_MOVE_ASSIGNABLE(sf::base::String));

        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(sf::base::String));
    }

    SECTION("Constructors")
    {
        SUBCASE("Default constructor")
        {
            constexpr sf::base::String str;
            STATIC_CHECK(str.isSso());
            STATIC_CHECK(str.size() == 0);
            STATIC_CHECK(str.capacity() == maxSsoSize);
            STATIC_CHECK(str.empty());
            STATIC_CHECK(str[0] == '\0');
        }

        SUBCASE("From const char*")
        {
            sf::base::String ssoStr("Short");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 5);
            CHECK(ssoStr.capacity() == maxSsoSize);
            CHECK(ssoStr == "Short");

            sf::base::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr.size() == sf::base::StringView(longStringLiteral).size());
            CHECK(heapStr.capacity() >= heapStr.size());
            CHECK(heapStr == longStringLiteral);
        }

        SUBCASE("From const char* and count")
        {
            sf::base::String ssoStr("Short", 3);
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "Sho");

            sf::base::String heapStr(longStringLiteral, maxSsoSize + 5);
            CHECK(!heapStr.isSso());
            CHECK(heapStr.size() == maxSsoSize + 5);
            CHECK(sf::base::StringView(heapStr) == sf::base::StringView(longStringLiteral, maxSsoSize + 5));
        }

        SUBCASE("From StringView")
        {
            sf::base::StringView ssoView("Short");
            sf::base::String     ssoStr(ssoView);
            CHECK(ssoStr.size() == 5);
            CHECK(ssoStr == "Short");

            sf::base::StringView heapView(longStringLiteral);
            sf::base::String     heapStr(heapView);
            CHECK(heapStr.size() == heapView.size());
            CHECK(heapStr == longStringLiteral);
        }

        SUBCASE("Copy constructor")
        {
            sf::base::String ssoOrig("Short");
            CHECK(ssoOrig.isSso());
            CHECK(ssoOrig.size() == 5);
            CHECK(ssoOrig == "Short");

            sf::base::String ssoCopy(ssoOrig); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(ssoCopy.isSso());
            CHECK(ssoCopy.size() == 5);
            CHECK(ssoCopy == "Short");

            sf::base::String heapOrig(longStringLiteral);
            CHECK(!heapOrig.isSso());
            CHECK(heapOrig == longStringLiteral);

            sf::base::String heapCopy(heapOrig);
            CHECK(!heapCopy.isSso());
            CHECK(heapCopy == longStringLiteral);
            CHECK(heapCopy.size() == heapOrig.size());
            CHECK(heapCopy.data() != heapOrig.data()); // Must be a deep copy
        }

        SUBCASE("Move constructor")
        {
            sf::base::String ssoOrig("Short");
            CHECK(ssoOrig.isSso());
            CHECK(ssoOrig.size() == 5);
            CHECK(ssoOrig == "Short");

            sf::base::String ssoMoved(SFML_BASE_MOVE(ssoOrig));
            CHECK(ssoMoved.size() == 5);
            CHECK(ssoMoved == "Short");

            // Source must be sso to avoid double-free
            CHECK(ssoOrig.isSso());

            sf::base::String heapOrig(longStringLiteral);
            CHECK(!heapOrig.isSso());
            CHECK(heapOrig == longStringLiteral);

            const char* origPtr = heapOrig.data();

            sf::base::String heapMoved(SFML_BASE_MOVE(heapOrig));
            CHECK(!heapMoved.isSso());
            CHECK(heapMoved == longStringLiteral);
            CHECK(heapMoved.size() == sf::base::StringView(longStringLiteral).size());
            CHECK(heapMoved == longStringLiteral);
            CHECK(heapMoved.data() == origPtr); // Pointer must be stolen

            // Source will be in empty SSO state
            CHECK(heapOrig.isSso());
            CHECK(heapOrig.empty());
        }
    }

    SECTION("Assignment")
    {
        SUBCASE("Copy assignment")
        {
            sf::base::String ssoStr("SSO");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "SSO");

            sf::base::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            sf::base::String dest;
            CHECK(dest.isSso());
            CHECK(dest.empty());

            dest = ssoStr; // SSO -> Empty
            CHECK(dest.isSso());
            CHECK(!dest.empty());
            CHECK(dest == "SSO");
            CHECK(!ssoStr.empty());
            CHECK(ssoStr == "SSO");

            dest = heapStr; // Heap -> SSO
            CHECK(!dest.isSso());
            CHECK(!dest.empty());
            CHECK(dest == longStringLiteral);
            CHECK(dest.data() != heapStr.data());
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            heapStr = dest; // Heap -> Heap
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            heapStr = ssoStr; // SSO -> Heap
            CHECK(!heapStr.isSso());
            CHECK(!heapStr.empty());
            CHECK(heapStr == "SSO");
        }

        SUBCASE("Move assignment")
        {
            sf::base::String ssoStr("SSO");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "SSO");

            sf::base::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            auto* origPtr = heapStr.data();

            sf::base::String dest;
            CHECK(dest.isSso());
            CHECK(dest.empty());

            dest = SFML_BASE_MOVE(ssoStr); // SSO -> empty
            CHECK(dest.isSso());
            CHECK(dest == "SSO");
            CHECK(ssoStr.isSso()); // Source must be sso to avoid double-free

            dest = SFML_BASE_MOVE(heapStr); // Heap -> SSO
            CHECK(dest == longStringLiteral);
            CHECK(heapStr.isSso());
            CHECK(heapStr.empty());

            CHECK(!dest.isSso());
            CHECK(dest == longStringLiteral);
            CHECK(dest.size() == sf::base::StringView(longStringLiteral).size());
            CHECK(dest == longStringLiteral);
            CHECK(dest.data() == origPtr); // Pointer must be stolen
        }

        SUBCASE("C-string and StringView assignment")
        {
            sf::base::String str;
            str = "From C-string";
            CHECK(str == "From C-string");

            str = "From StringView";
            CHECK(str == "From StringView");

            str = longStringLiteral;
            CHECK(str == longStringLiteral);
        }
    }

    SECTION("Modifiers")
    {
        SUBCASE("clear")
        {
            sf::base::String ssoStr("sso");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "sso");

            ssoStr.clear();
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.empty());
            CHECK(ssoStr.size() == 0);
            CHECK(ssoStr.capacity() == maxSsoSize); // Capacity is preserved

            sf::base::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            const auto origHeapCap = heapStr.capacity();

            heapStr.clear();
            CHECK(!heapStr.isSso());
            CHECK(heapStr.empty());
            CHECK(heapStr.size() == 0);
            CHECK(heapStr.capacity() == origHeapCap); // Capacity is preserved
        }

        SUBCASE("pushBack")
        {
            sf::base::String str;
            CHECK(str.isSso());
            CHECK(str.empty());
            CHECK(str == "");

            str.pushBack('A');
            CHECK(str.isSso());
            CHECK(str.size() == 1);
            CHECK(str == "A");
            CHECK(sf::base::StringView(str) == "A");

            // Fill up to SSO limit
            for (sf::base::SizeT i = 1; i < maxSsoSize; ++i)
                str.pushBack('B');

            CHECK(str.isSso());
            CHECK(str.size() == maxSsoSize);
            CHECK(str.capacity() == maxSsoSize);

            // Trigger transition to heap
            str.pushBack('C');
            CHECK(!str.isSso());
            CHECK(str.size() == maxSsoSize + 1);
            CHECK(str.capacity() > maxSsoSize);
            CHECK(str[maxSsoSize] == 'C');

            // Append on heap
            str.pushBack('D');
            CHECK(!str.isSso());
            CHECK(str.size() == maxSsoSize + 2);
            CHECK(str[maxSsoSize + 1] == 'D');
        }

        SUBCASE("append")
        {
            sf::base::String str("Start");
            CHECK(str.isSso());

            str.append(sf::base::StringView("-Middle"));
            CHECK(str.isSso());
            CHECK(sf::base::StringView(str) == "Start-Middle");

            str.append("-End");
            CHECK(str.isSso());
            CHECK(sf::base::StringView(str) == "Start-Middle-End");

            sf::base::String finalPart(longStringLiteral);
            str.append(finalPart);
            CHECK(!str.isSso());
            CHECK(str.size() == 16 + finalPart.size());
            CHECK(sf::base::StringView(str).substrByPosLen(16) == longStringLiteral);
        }

        SUBCASE("append2")
        {
            sf::base::String str;
            CHECK(str.isSso());
            CHECK(str.empty());

            str += 'a';
            CHECK(str.isSso());
            CHECK(str == "a");

            str += "bc";
            CHECK(str.isSso());
            CHECK(str == "abc");

            str += longStringLiteral;
            CHECK(!str.isSso());
            CHECK(str.toStringView().substrByPosLen(3) == longStringLiteral);
        }
    }

    SECTION("Iterators")
    {
        sf::base::String       str("abc");
        const sf::base::String cStr("xyz");

        // Basic iteration
        std::string result;
        for (char c : str)
            result += c;

        CHECK(result == "abc");

        // Const iteration
        result.clear();
        for (char c : cStr)
            result += c;

        CHECK(result == "xyz");

        // Algorithm compatibility
        char buffer[4] = {};
        sf::base::copy(str.begin(), str.end(), buffer);
        CHECK(sf::base::StringView(buffer, 3) == "abc");
    }

    SECTION("Comparison")
    {
        sf::base::String sso1("abc");
        sf::base::String sso2("abc");
        sf::base::String sso3("def");
        sf::base::String heap1(longStringLiteral);
        sf::base::String heap2(longStringLiteral);
        sf::base::String heap3("A different long string for comparison");

        CHECK(sso1 == sso2);
        CHECK(sso1 != sso3);
        CHECK(heap1 == heap2);
        CHECK(heap1 != heap3);

        // Cross-type comparisons
        CHECK(sso1 == "abc");
        CHECK("abc" == sso1);
        CHECK(sso1 != "def");
        CHECK(heap1 == longStringLiteral);
        CHECK(sf::base::StringView("abc") == sso1);
    }

    SECTION("Swap")
    {
        SUBCASE("SSO with SSO")
        {
            sf::base::String a("A");
            sf::base::String b("B");
            swap(a, b);
            CHECK(sf::base::StringView(a) == "B");
            CHECK(sf::base::StringView(b) == "A");
        }

        SUBCASE("SSO with Heap")
        {
            sf::base::String a("Short");
            sf::base::String b(longStringLiteral);
            const char*      bPtr = b.data();

            swap(a, b);

            CHECK(sf::base::StringView(a) == longStringLiteral);
            CHECK(a.data() == bPtr); // Pointer must be swapped
            CHECK(sf::base::StringView(b) == "Short");
        }

        SUBCASE("Heap with Heap")
        {
            sf::base::String a(longStringLiteral);
            sf::base::String b("Another long string that is also on the heap");
            const char*      aPtr = a.data();
            const char*      bPtr = b.data();

            swap(a, b);

            CHECK(a.data() == bPtr);
            CHECK(b.data() == aPtr);
        }
    }

    SECTION("Move from heap string")
    {
        sf::base::String heapStr(longStringLiteral);
        CHECK(!heapStr.isSso());
        CHECK(heapStr == longStringLiteral);

        sf::base::String dest = SFML_BASE_MOVE(heapStr); // Heap -> empty
        CHECK(!dest.isSso());
        CHECK(dest == longStringLiteral);
        CHECK(heapStr.isSso());
    }
}

// TODO P0: self-append, self-assign
