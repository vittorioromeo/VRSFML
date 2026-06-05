// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/IO/FileInputStream.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"
#include "Zancle/Vocabulary/UniquePtr.hpp"

#ifdef ZA_SYSTEM_ANDROID
    #include "Zancle/Window/Android/Activity.hpp"
    #include "Zancle/Window/Android/ResourceStream.hpp"
#endif

#include "Zancle/IO/FileUtils.hpp"
#include "Zancle/IO/Path.hpp"

#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Base/SizeT.hpp"

#include <cstdio>


namespace za
{
////////////////////////////////////////////////////////////
void FileInputStream::FileCloser::operator()(std::FILE* file)
{
    if (file != nullptr)
        std::fclose(file);
}


////////////////////////////////////////////////////////////
FileInputStream::~FileInputStream()                                     = default;
FileInputStream::FileInputStream(FileInputStream&&) noexcept            = default;
FileInputStream& FileInputStream::operator=(FileInputStream&&) noexcept = default;


////////////////////////////////////////////////////////////
za::Optional<FileInputStream> FileInputStream::open(const Path& filename)
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        auto androidFile = za::makeUnique<priv::ResourceStream>();
        if (!androidFile->open(filename))
            return za::nullOpt;

        return androidFile->tell().hasValue()
                   ? za::makeOptional<FileInputStream>(za::PassKey<FileInputStream>{}, ZA_MOVE(androidFile))
                   : za::nullOpt;
    }
#endif

    if (auto file = za::UniquePtr<std::FILE, FileCloser>(openFile(filename, "rb")))
        return za::makeOptional<FileInputStream>(za::PassKey<FileInputStream>{}, ZA_MOVE(file));

    return za::nullOpt;
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> FileInputStream::read(void* data, za::SizeT size)
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZA_ASSERT(m_androidFile != nullptr);
        return m_androidFile->read(data, size);
    }
#endif

    ZA_ASSERT(m_file != nullptr);
    return za::makeOptional(std::fread(data, 1, size, m_file.get()));
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> FileInputStream::seek(za::SizeT position)
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZA_ASSERT(m_androidFile != nullptr);
        return m_androidFile->seek(position);
    }
#endif

    ZA_ASSERT(m_file != nullptr);

    if (std::fseek(m_file.get(), static_cast<long>(position), SEEK_SET))
        return za::nullOpt;

    return tell();
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> FileInputStream::tell()
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZA_ASSERT(m_androidFile != nullptr);
        return m_androidFile->tell();
    }
#endif

    ZA_ASSERT(m_file != nullptr);

    const auto position = std::ftell(m_file.get());
    return position < 0 ? za::nullOpt : za::makeOptional(static_cast<za::SizeT>(position));
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> FileInputStream::getSize()
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZA_ASSERT(m_androidFile != nullptr);
        return m_androidFile->getSize();
    }
#endif

    ZA_ASSERT(m_file != nullptr);

    const auto position = tell().value();
    std::fseek(m_file.get(), 0, SEEK_END);

    za::Optional<za::SizeT> size = tell(); // Use a single local variable for NRVO

    if (!seek(position).hasValue())
    {
        size.reset();
        return size; // Empty optional
    }

    return size;
}


////////////////////////////////////////////////////////////
FileInputStream::FileInputStream(za::PassKey<FileInputStream>&&, za::UniquePtr<std::FILE, FileCloser>&& file) :
    m_file(ZA_MOVE(file))
{
}


////////////////////////////////////////////////////////////
#ifdef ZA_SYSTEM_ANDROID
FileInputStream::FileInputStream(za::PassKey<FileInputStream>&&, za::UniquePtr<priv::ResourceStream>&& androidFile) :
    m_androidFile(ZA_MOVE(androidFile))
{
}
#endif

} // namespace za
