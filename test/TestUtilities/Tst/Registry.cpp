// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Detail/Registry.hpp"
#include "Tst/Detail/State.hpp"

#include "Zancle/Base/SizeT.hpp"
#include "Zancle/Container/Vector.hpp"


////////////////////////////////////////////////////////////
// Test case registration and storage. All static initializers funnel
// through `registerTestCase` / `registerTemplatedTestCase`; the actual
// storage is a function-local `Vector` so the lifetime is well-defined
// across translation units.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
za::Vector<TestCaseInfo>& registeredTestCases() noexcept
{
    static za::Vector<TestCaseInfo> instance;
    return instance;
}


////////////////////////////////////////////////////////////
int registerTestCase(TestFn fn, const char* file, int line, const char* name) noexcept
{
    auto& list = registeredTestCases();
    list.emplaceBack(fn, file, line, name, nullptr, false);
    return static_cast<int>(list.size()) - 1;
}


////////////////////////////////////////////////////////////
int registerTemplatedTestCase(TestFn fn, const char* file, int line, const char* name, const char* typeStr) noexcept
{
    auto& list = registeredTestCases();
    list.emplaceBack(fn, file, line, name, typeStr, false);
    return static_cast<int>(list.size()) - 1;
}


////////////////////////////////////////////////////////////
void setTestCaseName(int index, const char* name) noexcept
{
    auto& list = registeredTestCases();

    if (index < 0 || static_cast<za::SizeT>(index) >= list.size())
        return;

    list.data()[static_cast<za::SizeT>(index)].name = name;
}


////////////////////////////////////////////////////////////
void setTestCaseSkip(int index, bool skip) noexcept
{
    auto& list = registeredTestCases();

    if (index < 0 || static_cast<za::SizeT>(index) >= list.size())
        return;

    list.data()[static_cast<za::SizeT>(index)].skipped = skip;
}

} // namespace tst::detail
