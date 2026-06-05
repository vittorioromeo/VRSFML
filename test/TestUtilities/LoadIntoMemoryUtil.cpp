#include "LoadIntoMemoryUtil.hpp"

#include "Zancle/IO/IO.hpp"
#include "Zancle/IO/Path.hpp"

#include "Zancle/Diagnostic/Assert.hpp"


za::Vector<char> loadIntoMemory(const char* path)
{
    za::Vector<char> buffer;

    [[maybe_unused]] const bool ok = za::readFromFile(za::Path{path}, buffer);
    ZA_ASSERT(ok);

    return buffer;
}
