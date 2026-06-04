#include "LoadIntoMemoryUtil.hpp"
#include "Zancle/System/IO.hpp"
#include "Zancle/System/Path.hpp"
#include "ZancleBase/Assert.hpp"


zb::Vector<char> loadIntoMemory(const char* path)
{
    zb::Vector<char> buffer;

    [[maybe_unused]] const bool ok = za::readFromFile(za::Path{path}, buffer);
    ZB_ASSERT(ok);

    return buffer;
}
