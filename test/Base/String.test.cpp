#include "StringifyZbStringUtil.hpp" // IWYU pragma: keep
#include "StringifyStringViewUtil.hpp"   // IWYU pragma: keep
#include "Tst/Tst.hpp"
#include "ZancleBase/Algorithm/Copy.hpp"
#include "ZancleBase/Fmt/FmtAppendMixin.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp" // IWYU pragma: keep -- enables int/float `fmtArg`
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/StringViewSplits.hpp" // IWYU pragma: keep
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsMoveAssignable.hpp"
#include "ZancleBase/Trait/IsMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyRelocatable.hpp"


namespace
{

zb::String makeLongString(const zb::StringView& base)
{
    zb::String result;
    result.reserve(base.size() * 2 + 30);
    result.append(base);
    return result;
}

} // namespace


TEST_CASE("[Base] Base/String.hpp")
{
    // A string guaranteed to be longer than the SSO buffer
    const char*    longStringLiteral = "This is a long string that will definitely not fit in SSO.";
    constexpr auto maxSsoSize        = zb::String::maxSsoSize;

    SECTION("Type traits")
    {
        // String has manual resource management, so it should not be trivial.
        STATIC_CHECK(!ZB_IS_TRIVIAL(zb::String));
        STATIC_CHECK(!ZB_IS_TRIVIALLY_COPYABLE(zb::String));

        // But it should be fully copyable and movable.
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(zb::String));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(zb::String));
        STATIC_CHECK(ZB_IS_MOVE_CONSTRUCTIBLE(zb::String));
        STATIC_CHECK(ZB_IS_MOVE_ASSIGNABLE(zb::String));

        STATIC_CHECK(ZB_IS_TRIVIALLY_RELOCATABLE(zb::String));
    }

    SECTION("Constexpr")
    {
        constexpr zb::String emptyStr;
    }

    SECTION("Constructors")
    {
        SUBCASE("Default constructor")
        {
            constexpr zb::String str;
            STATIC_CHECK(str.isSso());
            STATIC_CHECK(str.size() == 0);
            STATIC_CHECK(str.capacity() == maxSsoSize);
            STATIC_CHECK(str.empty());
            STATIC_CHECK(str[0] == '\0');

            // Default constructor is non-explicit: must work in copy-list-init contexts.
            constexpr zb::String copyListInit = {};
            STATIC_CHECK(copyListInit.isSso());
            STATIC_CHECK(copyListInit.empty());

            const auto returnsEmpty = []() -> zb::String { return {}; };
            CHECK(returnsEmpty().empty());
        }

        SUBCASE("From const char*")
        {
            zb::String ssoStr("Short");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 5);
            CHECK(ssoStr.capacity() == maxSsoSize);
            CHECK(ssoStr == "Short");

            zb::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr.size() == zb::StringView(longStringLiteral).size());
            CHECK(heapStr.capacity() >= heapStr.size());
            CHECK(heapStr == longStringLiteral);
        }

        SUBCASE("From const char* and count")
        {
            zb::String ssoStr("Short", 3);
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "Sho");

            zb::String heapStr(longStringLiteral, maxSsoSize + 5);
            CHECK(!heapStr.isSso());
            CHECK(heapStr.size() == maxSsoSize + 5);
            CHECK(zb::StringView(heapStr) == zb::StringView(longStringLiteral, maxSsoSize + 5));
        }

        SUBCASE("From StringView")
        {
            zb::StringView ssoView("Short");
            zb::String     ssoStr(ssoView);
            CHECK(ssoStr.size() == 5);
            CHECK(ssoStr == "Short");

            zb::StringView heapView(longStringLiteral);
            zb::String     heapStr(heapView);
            CHECK(heapStr.size() == heapView.size());
            CHECK(heapStr == longStringLiteral);
        }

        SUBCASE("Copy constructor")
        {
            zb::String ssoOrig("Short");
            CHECK(ssoOrig.isSso());
            CHECK(ssoOrig.size() == 5);
            CHECK(ssoOrig == "Short");

            zb::String ssoCopy(ssoOrig); // NOLINT(performance-unnecessary-copy-initialization)
            CHECK(ssoCopy.isSso());
            CHECK(ssoCopy.size() == 5);
            CHECK(ssoCopy == "Short");

            zb::String heapOrig(longStringLiteral);
            CHECK(!heapOrig.isSso());
            CHECK(heapOrig == longStringLiteral);

            zb::String heapCopy(heapOrig);
            CHECK(!heapCopy.isSso());
            CHECK(heapCopy == longStringLiteral);
            CHECK(heapCopy.size() == heapOrig.size());
            CHECK(heapCopy.data() != heapOrig.data()); // Must be a deep copy
        }

        SUBCASE("Move constructor")
        {
            zb::String ssoOrig("Short");
            CHECK(ssoOrig.isSso());
            CHECK(ssoOrig.size() == 5);
            CHECK(ssoOrig == "Short");

            zb::String ssoMoved(ZB_MOVE(ssoOrig));
            CHECK(ssoMoved.size() == 5);
            CHECK(ssoMoved == "Short");

            // Source must be sso to avoid double-free
            CHECK(ssoOrig.isSso());

            zb::String heapOrig(longStringLiteral);
            CHECK(!heapOrig.isSso());
            CHECK(heapOrig == longStringLiteral);

            const char* origPtr = heapOrig.data();

            zb::String heapMoved(ZB_MOVE(heapOrig));
            CHECK(!heapMoved.isSso());
            CHECK(heapMoved == longStringLiteral);
            CHECK(heapMoved.size() == zb::StringView(longStringLiteral).size());
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
            zb::String ssoStr("SSO");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "SSO");

            zb::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            zb::String dest;
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
            zb::String ssoStr("SSO");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "SSO");

            zb::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());
            CHECK(heapStr == longStringLiteral);

            auto* origPtr = heapStr.data();

            zb::String dest;
            CHECK(dest.isSso());
            CHECK(dest.empty());

            dest = ZB_MOVE(ssoStr); // SSO -> empty
            CHECK(dest.isSso());
            CHECK(dest == "SSO");
            CHECK(ssoStr.isSso()); // Source must be sso to avoid double-free

            dest = ZB_MOVE(heapStr); // Heap -> SSO
            CHECK(dest == longStringLiteral);
            CHECK(heapStr.isSso());
            CHECK(heapStr.empty());

            CHECK(!dest.isSso());
            CHECK(dest == longStringLiteral);
            CHECK(dest.size() == zb::StringView(longStringLiteral).size());
            CHECK(dest == longStringLiteral);
            CHECK(dest.data() == origPtr); // Pointer must be stolen
        }

        SUBCASE("C-string and StringView assignment")
        {
            zb::String str;
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
            zb::String ssoStr("sso");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);
            CHECK(ssoStr == "sso");

            ssoStr.clear();
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.empty());
            CHECK(ssoStr.size() == 0);
            CHECK(ssoStr.capacity() == maxSsoSize); // Capacity is preserved

            zb::String heapStr(longStringLiteral);
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
            zb::String str;
            CHECK(str.isSso());
            CHECK(str.empty());
            CHECK(str == "");

            str.pushBack('A');
            CHECK(str.isSso());
            CHECK(str.size() == 1);
            CHECK(str == "A");
            CHECK(zb::StringView(str) == "A");

            // Fill up to SSO limit
            for (zb::SizeT i = 1; i < maxSsoSize; ++i)
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

        SUBCASE("popBack")
        {
            // Pop on SSO string
            zb::String ssoStr("abc");
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 3);

            ssoStr.popBack();
            CHECK(ssoStr.isSso());
            CHECK(ssoStr.size() == 2);
            CHECK(ssoStr == "ab");
            CHECK(ssoStr.cStr()[2] == '\0'); // Null terminator written

            ssoStr.popBack();
            CHECK(ssoStr == "a");
            CHECK(ssoStr.cStr()[1] == '\0');

            ssoStr.popBack();
            CHECK(ssoStr.empty());
            CHECK(ssoStr.size() == 0);
            CHECK(ssoStr == "");
            CHECK(ssoStr.cStr()[0] == '\0');

            // Pop on heap string preserves capacity and heap-ness
            zb::String heapStr(longStringLiteral);
            CHECK(!heapStr.isSso());

            const auto origCap  = heapStr.capacity();
            const auto origSize = heapStr.size();

            heapStr.popBack();
            CHECK(!heapStr.isSso());
            CHECK(heapStr.size() == origSize - 1);
            CHECK(heapStr.capacity() == origCap);
            CHECK(heapStr.cStr()[heapStr.size()] == '\0');
            CHECK(zb::StringView(heapStr) == zb::StringView(longStringLiteral, origSize - 1));

            // Pop down to one character on heap (stays heap, capacity unchanged)
            while (heapStr.size() > 1)
                heapStr.popBack();

            CHECK(!heapStr.isSso());
            CHECK(heapStr.size() == 1);
            CHECK(heapStr.capacity() == origCap);
            CHECK(heapStr[0] == longStringLiteral[0]);

            heapStr.popBack();
            CHECK(!heapStr.isSso()); // Still on heap, popBack does not deallocate
            CHECK(heapStr.empty());
            CHECK(heapStr.capacity() == origCap);

            // pushBack/popBack round-trip on a freshly-grown heap string
            zb::String roundTrip;
            for (zb::SizeT i = 0; i < maxSsoSize + 5; ++i)
                roundTrip.pushBack(static_cast<char>('a' + (i % 26)));

            CHECK(!roundTrip.isSso());
            const auto rtSize = roundTrip.size();

            roundTrip.pushBack('Z');
            CHECK(roundTrip.size() == rtSize + 1);
            CHECK(roundTrip.back() == 'Z');

            roundTrip.popBack();
            CHECK(roundTrip.size() == rtSize);
            CHECK(roundTrip.back() == static_cast<char>('a' + ((rtSize - 1) % 26)));
            CHECK(roundTrip.cStr()[roundTrip.size()] == '\0');
        }

        SUBCASE("append")
        {
            // Build a string via two appends that stays within SSO
            const zb::String partA(longStringLiteral, maxSsoSize / 3);
            const zb::String partB(longStringLiteral, maxSsoSize / 3);
            const zb::String partC(longStringLiteral, maxSsoSize / 3);

            zb::String str(partA);
            CHECK(str.isSso());

            str.append(zb::StringView(partB));
            CHECK(str.isSso());

            str.append(zb::StringView(partC));
            CHECK(str.isSso());

            const auto prefixSize = str.size();

            zb::String finalPart(longStringLiteral);
            str.append(finalPart);
            CHECK(!str.isSso());
            CHECK(str.size() == prefixSize + finalPart.size());
            CHECK(zb::StringView(str).substrByPosLen(prefixSize) == longStringLiteral);
        }

        SUBCASE("append2")
        {
            zb::String str;
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
            CHECK(str.substrByPosLen(3u) == longStringLiteral);
        }
    }

    SECTION("Iterators")
    {
        zb::String       str("abc");
        const zb::String cStr("xyz");

        // Basic iteration
        zb::String result;
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
        zb::copy(str.begin(), str.end(), buffer);
        CHECK(zb::StringView(buffer, 3) == "abc");
    }

    SECTION("Comparison")
    {
        zb::String sso1("abc");
        zb::String sso2("abc");
        zb::String sso3("def");
        zb::String heap1(longStringLiteral);
        zb::String heap2(longStringLiteral);
        zb::String heap3("A different long string for comparison");

        CHECK(sso1 == sso2);
        CHECK(sso1 != sso3);
        CHECK(heap1 == heap2);
        CHECK(heap1 != heap3);

        // Cross-type comparisons
        CHECK(sso1 == "abc");
        CHECK("abc" == sso1);
        CHECK(sso1 != "def");
        CHECK(heap1 == longStringLiteral);
        CHECK(zb::StringView("abc") == sso1);
    }

    SECTION("Swap")
    {
        SUBCASE("SSO with SSO")
        {
            zb::String a("A");
            zb::String b("B");
            swap(a, b);
            CHECK(zb::StringView(a) == "B");
            CHECK(zb::StringView(b) == "A");
        }

        SUBCASE("SSO with Heap")
        {
            zb::String  a("Short");
            zb::String  b(longStringLiteral);
            const char* bPtr = b.data();

            swap(a, b);

            CHECK(zb::StringView(a) == longStringLiteral);
            CHECK(a.data() == bPtr); // Pointer must be swapped
            CHECK(zb::StringView(b) == "Short");
        }

        SUBCASE("Heap with Heap")
        {
            zb::String  a(longStringLiteral);
            zb::String  b("Another long string that is also on the heap");
            const char* aPtr = a.data();
            const char* bPtr = b.data();

            swap(a, b);

            CHECK(a.data() == bPtr);
            CHECK(b.data() == aPtr);
        }
    }

    SECTION("Move from heap string")
    {
        zb::String heapStr(longStringLiteral);
        CHECK(!heapStr.isSso());
        CHECK(heapStr == longStringLiteral);

        zb::String dest = ZB_MOVE(heapStr); // Heap -> empty
        CHECK(!dest.isSso());
        CHECK(dest == longStringLiteral);
        CHECK(heapStr.isSso());
    }

    SECTION("Self-append")
    {
        zb::String str("Hello");

        str.append(str);
        CHECK(str == "HelloHello");

        str.append(str);
        CHECK(str == "HelloHelloHelloHello");

        str.append(str);
        CHECK(str == "HelloHelloHelloHelloHelloHelloHelloHello");

        str.append(str);
        CHECK(str == "HelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHelloHello");
    }

    SECTION("SsoSelfAppend")
    {
        // Small string: should be SSO and appending itself should produce "abcabc"
        zb::String s("abc");
        CHECK(s.size() == 3u);
        s.append(s); // append itself
        CHECK(s.size() == 6u);
        CHECK(s == "abcabc");
    }


    SECTION("InPlaceSelfAppend_NoRealloc_UsesMemmovePath")
    {
        const auto makePattern = [](const zb::SizeT n)
        {
            zb::String s;
            s.reserve(n);

            for (zb::SizeT i = 0u; i < n; ++i)
                s.pushBack(char('a' + (i % 26)));

            return s;
        };

        // Prepare a reasonably large string so it uses heap storage
        const zb::SizeT initialSize = 128;
        const auto      pattern     = makePattern(initialSize);

        // Construct our zb::String on the heap
        zb::String s(pattern.cStr(), static_cast<zb::SizeT>(pattern.size()));
        CHECK(s.size() == static_cast<zb::SizeT>(initialSize));

        // Ensure capacity is large enough to fit the result of appending itself
        const auto needed = static_cast<zb::SizeT>(initialSize * 2);
        s.reserve(needed);

        // Sanity: capacity must be >= needed (so append won't reallocate)
        CHECK(s.capacity() == needed);

        // Capture data pointer before append -- if append reallocates, this will change.
        const char* dataBefore = s.data();

        // Perform self-append (source overlaps destination)
        s.append(s);

        // Data pointer should be unchanged (no reallocation) -> in-place path executed
        const char* dataAfter = s.data();
        CHECK(dataBefore == dataAfter);

        // Size doubled
        CHECK(s.size() == needed);

        // First half equals original
        CHECK(zb::StringView{s.cStr(), initialSize} == zb::StringView{pattern.cStr(), initialSize});

        // Second half equals original
        CHECK(zb::StringView{s.cStr() + initialSize, initialSize} == zb::StringView{pattern.cStr(), initialSize});
    }

    SECTION("Self-Assignment")
    {
        SECTION("s = s with SSO string")
        {
            zb::String s = "hello sso";
            CHECK(s.isSso());
            const char* originalData = s.data();

            auto& self = s;
            s          = self; // Self-assignment (avoid warning)

            CHECK(s == "hello sso");
            CHECK(s.data() == originalData); // Should not have reallocated
        }

        SECTION("s = s with Heap string")
        {
            zb::String s = makeLongString("this is a long string definitely on the heap");
            CHECK(!s.isSso());
            const char* originalData = s.data();
            const auto  originalCap  = s.capacity();

            auto& self = s;
            s          = self; // Self-assignment (avoid warning)

            CHECK(s == "this is a long string definitely on the heap");
            CHECK(s.data() == originalData); // Should not have reallocated
            CHECK(s.capacity() == originalCap);
        }

        SECTION("s = s.toStringView() with SSO string")
        {
            zb::String  s            = "sso view";
            const char* originalData = s.data();

            s = s.toStringView();

            CHECK(s == "sso view");
            CHECK(s.data() == originalData);
        }

        SECTION("s = s.toStringView() with Heap string")
        {
            zb::String  s            = makeLongString("heap view assignment");
            const char* originalData = s.data();

            s = s.toStringView();

            CHECK(s == "heap view assignment");
            CHECK(s.data() == originalData);
        }

        SECTION("Assigning an overlapping substring (tests memmove)")
        {
            // Case 1: Initial string is SSO
            zb::String s1 = "abcdefgh";
            CHECK(s1.isSso());

            // Assign a substring of itself: s1 should become "cdef"
            s1 = zb::StringView(s1.data() + 2, 4);

            CHECK(s1 == "cdef");

            // Case 2: Initial string is on the heap
            zb::String s2 = makeLongString("This is a long string for testing overlap");
            CHECK(!s2.isSso());

            // Assign a substring of itself: s2 should become "long"
            s2 = zb::StringView(s2.data() + 10, 4);
            CHECK(s2 == "long");
        }
    }

    SECTION("Self-Append")
    {
        SECTION("s += s with SSO string (transitioning to heap)")
        {
            // Choose a size that fits SSO but when doubled exceeds maxSsoSize.
            const auto len = (maxSsoSize / 2) + 1;
            zb::String s(longStringLiteral, len);
            CHECK(s.size() == len);
            CHECK(s.isSso());

            const zb::String expected = s + zb::StringView(s);
            s += s; // Self-append, should trigger grow() and use-after-free bug

            CHECK(s.size() == len * 2);
            CHECK(s == expected);
            CHECK(!s.isSso());
        }

        SECTION("s += s with Heap string (forcing reallocation)")
        {
            // Create a heap string with little to no spare capacity
            zb::String original = "a long string that will force a reallocation upon self-append";
            zb::String s        = original;
            CHECK(!s.isSso());
            CHECK(s.capacity() - s.size() < s.size()); // Ensure it *must* reallocate

            s += s; // Self-append that reallocates heap buffer

            zb::String expected = original;
            expected.append(original);

            CHECK(s == expected);
            CHECK(!s.isSso());
        }

        SECTION("s += s with Heap string (with enough capacity)")
        {
            zb::String s = "pre-reserved";
            // Reserve enough space so no reallocation occurs
            s.reserve(s.size() * 2 + 10);
            CHECK(!s.isSso());
            const char* originalData = s.data();

            s += s; // Self-append in-place

            CHECK(s == "pre-reservedpre-reserved");
            CHECK(s.data() == originalData); // Must not reallocate
        }

        SECTION("Appending a substring of itself")
        {
            zb::String s = "12345";
            CHECK(s.isSso());

            // Append a view of its own beginning
            s.append(zb::StringView(s.data(), 3)); // Append "123"

            CHECK(s == "12345123");
        }
    }

    SECTION("Self-Insert")
    {
        SECTION("insert with overlapping source and no reallocation")
        {
            zb::String s("abcdef");
            s.reserve(12);

            const char* const originalData = s.data();
            s.insert(2, s.cStr() + 1);

            CHECK(s.data() == originalData);
            CHECK(s == "abbcdefcdef");
        }

        SECTION("insert with overlapping source and reallocation")
        {
            // Build a string that fits SSO but after insert(3, cStr()+2)
            // produces a result of length (2*len - 2) that exceeds maxSsoSize.
            const auto len = (maxSsoSize / 2) + 2; // guarantees 2*len - 2 > maxSsoSize
            zb::String s(longStringLiteral, len);
            CHECK(s.isSso());

            // Build the expected result: s[0..3] + s[2..] + s[3..]
            zb::String expected;
            expected.append(zb::StringView(s).substrByPosLen(0, 3));
            expected.append(zb::StringView(s).substrByPosLen(2));
            expected.append(zb::StringView(s).substrByPosLen(3));

            s.insert(3, s.cStr() + 2);

            CHECK(!s.isSso());
            CHECK(s == expected);
        }
    }

    SECTION("Resize")
    {
        zb::String s = "Hello";
        CHECK(s.isSso());
        CHECK(s.size() == 5);

        s.resize(10, 'X');
        CHECK(s.size() == 10);
        CHECK(s == "HelloXXXXX");
        CHECK(s.isSso());

        s.resize(3);
        CHECK(s.size() == 3);
        CHECK(s == "Hel");
        CHECK(s.isSso());

        s.resize(50);
        CHECK(s.size() == 50);
        CHECK(s[0] == 'H');
        CHECK(s[1] == 'e');
        CHECK(s[2] == 'l');

        for (zb::SizeT i = 3; i < 50; ++i)
            CHECK(s[i] == '\0');

        CHECK(!s.isSso());
    }

    SECTION("ResizeAndOverwrite")
    {
        SECTION("Grow from SSO into heap, op fills the whole buffer")
        {
            zb::String s = "abc";
            CHECK(s.isSso());
            CHECK(s.size() == 3);

            constexpr zb::SizeT requested = 100u; // > maxSsoSize
            s.resizeAndOverwrite(requested,
                                 [](char* buf, zb::SizeT n) -> zb::SizeT
            {
                for (zb::SizeT i = 0; i < n; ++i)
                    buf[i] = static_cast<char>('a' + (i % 26));
                return n;
            });

            CHECK(s.size() == requested);
            CHECK(!s.isSso());
            CHECK(s[0] == 'a');
            CHECK(s[1] == 'b');
            CHECK(s[25] == 'z');
            CHECK(s[26] == 'a');
            // Must always be null-terminated for cStr() callers.
            CHECK(s.cStr()[requested] == '\0');
        }

        SECTION("Returning a smaller size truncates")
        {
            zb::String s;
            s.resizeAndOverwrite(20u,
                                 [](char* buf, zb::SizeT) -> zb::SizeT
            {
                buf[0] = 'h';
                buf[1] = 'i';
                return 2u;
            });

            CHECK(s.size() == 2u);
            CHECK(s == "hi");
            CHECK(s.cStr()[2] == '\0');
        }

        SECTION("Returning zero clears the string")
        {
            zb::String s = "previous";
            s.resizeAndOverwrite(50u, [](char*, zb::SizeT) -> zb::SizeT { return 0u; });

            CHECK(s.empty());
            CHECK(s.size() == 0u);
            CHECK(s.cStr()[0] == '\0');
        }

        SECTION("Existing prefix is preserved up to min(size(), newSize)")
        {
            // Matches std::string::resize_and_overwrite semantics: bytes up to
            // the lesser of old and new size keep their values when op runs.
            zb::String s = "Hello";
            CHECK(s.isSso());

            s.resizeAndOverwrite(8u,
                                 [](char* buf, zb::SizeT n) -> zb::SizeT
            {
                // First 5 bytes are still "Hello"; only fill the new tail.
                CHECK(buf[0] == 'H');
                CHECK(buf[1] == 'e');
                CHECK(buf[2] == 'l');
                CHECK(buf[3] == 'l');
                CHECK(buf[4] == 'o');
                buf[5] = '!';
                buf[6] = '!';
                buf[7] = '!';
                return n;
            });

            CHECK(s == "Hello!!!");
        }

        SECTION("Shrinking via newSize < size() keeps prefix and truncates")
        {
            zb::String s = "abcdefghij";
            s.resizeAndOverwrite(3u,
                                 [](char* buf, zb::SizeT n) -> zb::SizeT
            {
                CHECK(buf[0] == 'a');
                CHECK(buf[1] == 'b');
                CHECK(buf[2] == 'c');
                return n;
            });

            CHECK(s.size() == 3u);
            CHECK(s == "abc");
        }

        SECTION("newSize == 0 with empty string is a no-op")
        {
            zb::String s;
            s.resizeAndOverwrite(0u, [](char*, zb::SizeT n) -> zb::SizeT { return n; });

            CHECK(s.empty());
            CHECK(s.cStr()[0] == '\0');
        }
    }

    SECTION("Replace")
    {
        SECTION("Same-length replacement (no shift)")
        {
            zb::String s = "Hello, world!";
            s.replace(7, 5, "WORLD");
            CHECK(s == "Hello, WORLD!");
            CHECK(s.size() == 13u);
        }

        SECTION("Shorter replacement (tail shifts left)")
        {
            zb::String s = "Hello, world!";
            s.replace(7, 5, "you");
            CHECK(s == "Hello, you!");
            CHECK(s.size() == 11u);
        }

        SECTION("Longer replacement (tail shifts right)")
        {
            zb::String s = "Hello, world!";
            s.replace(7, 5, "everybody");
            CHECK(s == "Hello, everybody!");
            CHECK(s.size() == 17u);
        }

        SECTION("Empty replacement acts as erase")
        {
            zb::String s = "Hello, world!";
            s.replace(5, 7, "");
            CHECK(s == "Hello!");
            CHECK(s.size() == 6u);
        }

        SECTION("Zero count acts as insert")
        {
            zb::String s = "Hello!";
            s.replace(5, 0, ", world");
            CHECK(s == "Hello, world!");
            CHECK(s.size() == 13u);
        }

        SECTION("Replace at beginning")
        {
            zb::String s = "abcdef";
            s.replace(0, 3, "XYZW");
            CHECK(s == "XYZWdef");
        }

        SECTION("Replace at end (pos == size)")
        {
            zb::String s = "abc";
            s.replace(3, 0, "def");
            CHECK(s == "abcdef");
        }

        SECTION("count == nPos clamps to end")
        {
            zb::String s = "Hello, world!";
            s.replace(7, zb::String::nPos, "EVERYONE");
            CHECK(s == "Hello, EVERYONE");
        }

        SECTION("count past end clamps to end")
        {
            zb::String s = "abcdef";
            s.replace(2, 100u, "XYZ");
            CHECK(s == "abXYZ");
        }

        SECTION("Replace whole string")
        {
            zb::String s = "abc";
            s.replace(0, s.size(), "abcdefghij");
            CHECK(s == "abcdefghij");
        }

        SECTION("Replace then content fits in original capacity (no realloc)")
        {
            zb::String s = "Hello, world!";
            s.reserve(64);
            const char* const originalData = s.data();

            s.replace(7, 5, "everybody");

            CHECK(s == "Hello, everybody!");
            CHECK(s.data() == originalData); // no reallocation
        }

        SECTION("Replace forces growth out of SSO")
        {
            zb::String s = "abc";
            CHECK(s.isSso());

            s.replace(0, s.size(), longStringLiteral);

            CHECK(!s.isSso());
            CHECK(s == longStringLiteral);
        }

        SECTION("Self-aliasing replacement (substring of itself)")
        {
            // Replace [7..12) ("world") with the substring "Hello" already
            // sitting at [0..5). The implementation must copy the source first
            // before shifting the tail, otherwise the read would race the move.
            zb::String s = "Hello, world!";
            s.replace(7, 5, zb::StringView{s}.substrByPosLen(0, 5));

            CHECK(s == "Hello, Hello!");
        }

        SECTION("Self-aliasing replacement that overlaps the replaced range")
        {
            // Source overlaps the destination: replace [3..6) with the
            // substring at [4..7). Self-aliasing path must copy first.
            zb::String s = "abcdefghij";
            s.replace(3, 3, zb::StringView{s}.substrByPosLen(4, 3));

            CHECK(s == "abcefgghij");
        }

        SECTION("Heap string self-aliasing forcing reallocation")
        {
            // Long string that's already on the heap; replace with a tail of
            // itself that, after substitution, exceeds current capacity.
            zb::String s(longStringLiteral);
            CHECK(!s.isSso());

            const auto       tailView = zb::StringView{s}.substrByPosLen(s.size() / 2);
            const zb::String expected = zb::String{longStringLiteral} + zb::String{tailView};

            // Replace zero chars at end with our own tail -> doubles the data.
            s.replace(s.size(), 0, tailView);

            CHECK(s == expected);
        }

        SECTION("Replace with empty replacement on empty string is a no-op")
        {
            zb::String s;
            s.replace(0, 0, "");
            CHECK(s.empty());
        }

        SECTION("Replace preserves null terminator")
        {
            zb::String s = "Hello, world!";
            s.replace(7, 5, "you");
            // `cStr()` must still produce a valid C-string at the new size.
            CHECK(s.cStr()[s.size()] == '\0');
        }

        SECTION("Replace accepts String, StringView, const char* via implicit conversions")
        {
            zb::String s1 = "abc";
            zb::String s2 = "abc";
            zb::String s3 = "abc";

            const zb::String     replString = "XX";
            const zb::StringView replView   = "YY";

            s1.replace(1, 1, replString);
            s2.replace(1, 1, replView);
            s3.replace(1, 1, "ZZ");

            CHECK(s1 == "aXXc");
            CHECK(s2 == "aYYc");
            CHECK(s3 == "aZZc");
        }
    }

    SECTION("ReplaceFirstOccurrence")
    {
        SECTION("Found")
        {
            zb::String s = "Hello, world!";
            CHECK(s.replaceFirstOccurrence("world", "everybody"));
            CHECK(s == "Hello, everybody!");
        }

        SECTION("Not found leaves string unchanged")
        {
            zb::String s = "Hello, world!";
            CHECK_FALSE(s.replaceFirstOccurrence("xyz", "ABC"));
            CHECK(s == "Hello, world!");
        }

        SECTION("Only first match is replaced")
        {
            zb::String s = "abc abc abc";
            CHECK(s.replaceFirstOccurrence("abc", "XYZ"));
            CHECK(s == "XYZ abc abc");
        }

        SECTION("Empty target returns false and leaves string unchanged")
        {
            zb::String s = "Hello";
            CHECK_FALSE(s.replaceFirstOccurrence("", "X"));
            CHECK(s == "Hello");
        }

        SECTION("Empty replacement acts as erase")
        {
            zb::String s = "Hello, world!";
            CHECK(s.replaceFirstOccurrence(", world", ""));
            CHECK(s == "Hello!");
        }

        SECTION("Replace at very start")
        {
            zb::String s = "abcabc";
            CHECK(s.replaceFirstOccurrence("abc", "XX"));
            CHECK(s == "XXabc");
        }

        SECTION("Replace at very end")
        {
            zb::String s = "abcabc";
            CHECK(s.replaceFirstOccurrence("bc", "XYZ"));
            CHECK(s == "aXYZabc"); // first "bc" at index 1
        }

        SECTION("Empty target on empty string returns false")
        {
            zb::String s;
            CHECK_FALSE(s.replaceFirstOccurrence("", "X"));
            CHECK(s.empty());
        }

        SECTION("Self-aliasing target view (substring of self)")
        {
            zb::String s   = "abcdef";
            const auto sub = zb::StringView{s}.substrByPosLen(2, 2); // "cd"
            CHECK(s.replaceFirstOccurrence(sub, "XYZ"));
            CHECK(s == "abXYZef");
        }
    }

    SECTION("ReplaceAllOccurrences")
    {
        SECTION("Multiple matches")
        {
            zb::String s = "abc abc abc";
            CHECK(s.replaceAllOccurrences("abc", "X") == 3u);
            CHECK(s == "X X X");
        }

        SECTION("No matches leaves string unchanged and returns 0")
        {
            zb::String s = "Hello, world!";
            CHECK(s.replaceAllOccurrences("xyz", "ABC") == 0u);
            CHECK(s == "Hello, world!");
        }

        SECTION("Single match returns 1")
        {
            zb::String s = "Hello, world!";
            CHECK(s.replaceAllOccurrences("world", "everybody") == 1u);
            CHECK(s == "Hello, everybody!");
        }

        SECTION("Empty target returns 0 and leaves string unchanged")
        {
            zb::String s = "Hello";
            CHECK(s.replaceAllOccurrences("", "X") == 0u);
            CHECK(s == "Hello");
        }

        SECTION("Empty replacement removes all occurrences")
        {
            zb::String s = "abXYZabXYZab";
            CHECK(s.replaceAllOccurrences("XYZ", "") == 2u);
            CHECK(s == "ababab");
        }

        SECTION("Adjacent matches all replaced")
        {
            zb::String s = "aaaa";
            // "aa" "aa" matches at 0 and 2; both get replaced.
            CHECK(s.replaceAllOccurrences("aa", "X") == 2u);
            CHECK(s == "XX");
        }

        SECTION("Replacement contains target -- does not infinite-loop")
        {
            zb::String s = "abc";
            // "a" -> "aa": after replace, advance past it so we don't re-match.
            CHECK(s.replaceAllOccurrences("a", "aa") == 1u);
            CHECK(s == "aabc");
        }

        SECTION("Replacement equals target is a no-op (count is reported)")
        {
            zb::String s = "abcabc";
            CHECK(s.replaceAllOccurrences("abc", "abc") == 2u);
            CHECK(s == "abcabc");
        }

        SECTION("Overlapping target avoids re-matching its own output")
        {
            // Replacement "a" at every 2-char "aa": "aaaa" -> "aaa" -> "aa".
            // We use the non-overlapping convention: jump past each replacement,
            // so positions are 0 and 2 (after first replace -> "aaa"), then 1
            // (after second replace -> "aa"). Two replacements total.
            zb::String s = "aaaa";
            CHECK(s.replaceAllOccurrences("aa", "a") == 2u);
            CHECK(s == "aa");
        }

        SECTION("Replacement causes growth out of SSO")
        {
            zb::String s = "aXa";
            CHECK(s.isSso());

            // Replace each 'X' with the long literal -> result definitely heap.
            const auto count = s.replaceAllOccurrences("X", longStringLiteral);
            CHECK(count == 1u);
            CHECK(!s.isSso());

            zb::String expected = "a";
            expected.append(longStringLiteral);
            expected.append("a");
            CHECK(s == expected);
        }

        SECTION("Self-aliasing target view (substring of self)")
        {
            zb::String s = "abcabcabc";
            // Take a view of the first "abc" and replace all occurrences of
            // it with "X". The aliasing-detection path must copy the target
            // before the buffer reallocates.
            const auto needle = zb::StringView{s}.substrByPosLen(0, 3);
            CHECK(s.replaceAllOccurrences(needle, "X") == 3u);
            CHECK(s == "XXX");
        }

        SECTION("Self-aliasing replacement view (substring of self)")
        {
            zb::String s = "ab_cd";
            // Replace "_" with everything from index 0 to 2 ("ab"). View is
            // taken before mutation; aliasing-detection path must copy it.
            const auto repl = zb::StringView{s}.substrByPosLen(0, 2);
            CHECK(s.replaceAllOccurrences("_", repl) == 1u);
            CHECK(s ==
                  "abab"
                  "cd"); // "ab" + "ab" + "cd"
        }

        SECTION("Heap string with many matches forces multiple shifts")
        {
            // Build a long string of repeated tokens, then replace them all.
            zb::String s;
            for (int i = 0; i < 100; ++i)
                s.append("foo,");

            const auto count = s.replaceAllOccurrences("foo", "bar");
            CHECK(count == 100u);

            zb::String expected;
            for (int i = 0; i < 100; ++i)
                expected.append("bar,");
            CHECK(s == expected);
        }

        SECTION("Empty replacement on empty string is a no-op")
        {
            zb::String s;
            CHECK(s.replaceAllOccurrences("x", "") == 0u);
            CHECK(s.empty());
        }
    }

    SECTION("StringView bridge methods")
    {
        SECTION("forSplits is bridged from StringView (B)")
        {
            zb::String s = "alpha,beta,gamma";

            zb::SizeT count = 0u;
            s.forSplits(',',
                        [&](zb::StringView seg)
            {
                ++count;
                if (count == 1u)
                    CHECK(seg == "alpha");
                if (count == 2u)
                    CHECK(seg == "beta");
                if (count == 3u)
                    CHECK(seg == "gamma");
            });
            CHECK(count == 3u);
        }

        SECTION("forSplits with StringView splitter is bridged from StringView (B)")
        {
            zb::String s = "foo::bar::baz";

            zb::SizeT count = 0u;
            s.forSplits("::",
                        [&](zb::StringView seg)
            {
                ++count;
                if (count == 1u)
                    CHECK(seg == "foo");
                if (count == 2u)
                    CHECK(seg == "bar");
                if (count == 3u)
                    CHECK(seg == "baz");
            });
            CHECK(count == 3u);
        }

        SECTION("forLines side-effects are not elided (no gnu::pure on bridge) (C)")
        {
            // The bridge macro previously had `gnu::pure`, which would have
            // allowed the optimizer to drop calls whose return value isn't used.
            // `forLines` returns void and runs side-effecting user code, so
            // any elision would be observable as a missed counter increment.
            zb::String s    = "a\nb\nc\nd";
            zb::SizeT  hits = 0u;
            s.forLines([&](zb::StringView) { ++hits; });
            CHECK(hits == 4u);
        }
    }

    SECTION("erase at end-of-string is a no-op (E)")
    {
        // `std::string::erase(size())` is allowed and is a no-op (count
        // clamps to 0). Previously this asserted; loosened to `<=`.
        zb::String s = "Hello";

        s.erase(s.size());
        CHECK(s == "Hello");

        s.erase(s.size(), 0u);
        CHECK(s == "Hello");

        s.erase(s.size(), zb::String::nPos);
        CHECK(s == "Hello");

        // Erasing the entire string still works at index 0.
        s.erase(0u);
        CHECK(s.empty());

        // Erasing at the (now zero) size of an empty string is also valid.
        s.erase(0u);
        CHECK(s.empty());
    }

    SECTION("operator== distinguishes embedded NUL bytes (A)")
    {
        // String comparison goes through StringView's `==` (memcmp), so it
        // must distinguish strings that differ AFTER an embedded NUL.
        const char raw1[] = {'a', '\0', 'b'};
        const char raw2[] = {'a', '\0', 'c'};

        const zb::String s1{raw1, 3};
        const zb::String s2{raw2, 3};

        CHECK_FALSE(s1 == s2);
        CHECK(s1 != s2);
        CHECK(s1 == s1);
    }

    SECTION("operator+ lvalue overloads")
    {
        const zb::String a = "foo";
        const zb::String b = "bar";

        CHECK(a + b == "foobar");
        CHECK(a + 'x' == "foox");
        CHECK('x' + a == "xfoo");
        CHECK(a + "bar" == "foobar");
        CHECK("bar" + a == "barfoo");
        CHECK(a + zb::StringView{"bar"} == "foobar");
        CHECK(zb::StringView{"bar"} + a == "barfoo");

        // Source operands unaffected.
        CHECK(a == "foo");
        CHECK(b == "bar");
    }

    SECTION("operator+ rvalue-lhs reuses the buffer (no realloc)")
    {
        // Force the lhs onto the heap with capacity to spare so the
        // append does not reallocate. The result should share the same
        // heap pointer, proving that lhs was moved into the result and
        // appended in-place rather than the impl allocating a fresh buffer.
        zb::String lhs;
        lhs.reserve(64u);
        lhs = "abcdefghijklmnopqrstuvwxyz"; // > SSO max, on heap
        REQUIRE(lhs.capacity() >= 28u);

        const char* const lhsPtrBefore = lhs.data();

        const zb::String result = static_cast<zb::String&&>(lhs) + zb::String{"!!"};
        CHECK(result == "abcdefghijklmnopqrstuvwxyz!!");
        CHECK(result.data() == lhsPtrBefore);
    }

    SECTION("operator+ rvalue-lhs overloads -- all argument types")
    {
        auto makeHeap = []
        {
            zb::String s;
            s.reserve(64u);
            s = "abcdefghijklmnopqrstuvwxyz";
            return s;
        };

        CHECK(makeHeap() + zb::String{"!"} == "abcdefghijklmnopqrstuvwxyz!");
        CHECK(makeHeap() + '!' == "abcdefghijklmnopqrstuvwxyz!");
        CHECK(makeHeap() + "!!" == "abcdefghijklmnopqrstuvwxyz!!");
        CHECK(makeHeap() + zb::StringView{"!!!"} == "abcdefghijklmnopqrstuvwxyz!!!");
    }

    SECTION("operator+ chained expressions feed the rvalue overload")
    {
        // `a + b + c + d` -- after the first `+`, subsequent operands see
        // an rvalue lhs and reuse the buffer instead of reallocating.
        // The buffer-reuse property is verified in the prior section;
        // here we only check the result.
        const zb::String a = "Hello, ";
        const zb::String b = "beautiful ";
        const zb::String c = "world";
        const zb::String d = "!";

        CHECK(a + b + c + d == "Hello, beautiful world!");
    }

    SECTION("appendFmt (FmtAppendMixin) on empty string")
    {
        zb::String s;
        s.appendFmt("hello");
        CHECK(s == "hello");
    }

    SECTION("appendFmt with single placeholder")
    {
        zb::String s;
        s.appendFmt("answer = {}", 42);
        CHECK(s == "answer = 42");
    }

    SECTION("appendFmt with multiple heterogeneous placeholders")
    {
        zb::String s;
        s.appendFmt("{}-{}-{}", 1, 'x', zb::StringView{"end"});
        CHECK(s == "1-x-end");
    }

    SECTION("appendFmt with width / precision / type-tag spec")
    {
        zb::String s;
        s.appendFmt("[{:5}][{:x}][{:.3}]", 7, 255, 3.14159);
        CHECK(s == "[    7][ff][3.142]");
    }

    SECTION("appendFmt is genuinely appending (not replacing)")
    {
        zb::String s = "prefix:";
        s.appendFmt(" v={}", 7);
        CHECK(s == "prefix: v=7");

        // Second appendFmt continues onto the existing buffer.
        s.appendFmt(",{}", 11);
        CHECK(s == "prefix: v=7,11");
    }

    SECTION("appendFmt grows an SSO buffer onto the heap when needed")
    {
        zb::String s = "abc";
        // Long format expansion forces growth beyond SSO.
        s.appendFmt("-{}-{}-{}-{}-{}-{}-{}-{}", "alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta");
        CHECK(s == "abc-alpha-beta-gamma-delta-epsilon-zeta-eta-theta");
    }

    SECTION("appendArg appends a single value with no spec")
    {
        zb::String s = "n=";
        s.appendArg(42);
        CHECK(s == "n=42");

        s.appendArg(',' /*char*/);
        CHECK(s == "n=42,");

        s.appendArg(zb::StringView{"tail"});
        CHECK(s == "n=42,tail");
    }

    SECTION("appendArg with float renders default precision")
    {
        zb::String s;
        s.appendArg(3.14159);
        // `defaultFloatPrecision == 6` ({:.6f}).
        CHECK(s == "3.141590");
    }
}
