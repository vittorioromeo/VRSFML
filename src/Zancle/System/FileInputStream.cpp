// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/FileInputStream.hpp"

#include "ZancleBase/Optional.hpp"
#include "ZancleBase/PassKey.hpp"
#include "ZancleBase/UniquePtr.hpp"

#ifdef ZA_SYSTEM_ANDROID
    #include "Zancle/System/Android/Activity.hpp"
    #include "Zancle/System/Android/ResourceStream.hpp"
#endif

#include "Zancle/System/FileUtils.hpp"
#include "Zancle/System/Path.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/SizeT.hpp"

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
base::Optional<FileInputStream> FileInputStream::open(const Path& filename)
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        auto androidFile = base::makeUnique<priv::ResourceStream>();
        if (!androidFile->open(filename))
            return base::nullOpt;

        return androidFile->tell().hasValue()
                   ? base::makeOptional<FileInputStream>(base::PassKey<FileInputStream>{}, ZB_MOVE(androidFile))
                   : base::nullOpt;
    }
#endif

    if (auto file = base::UniquePtr<std::FILE, FileCloser>(openFile(filename, "rb")))
        return base::makeOptional<FileInputStream>(base::PassKey<FileInputStream>{}, ZB_MOVE(file));

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::read(void* data, base::SizeT size)
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZB_ASSERT(m_androidFile != nullptr);
        return m_androidFile->read(data, size);
    }
#endif

    ZB_ASSERT(m_file != nullptr);
    return base::makeOptional(std::fread(data, 1, size, m_file.get()));
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::seek(base::SizeT position)
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZB_ASSERT(m_androidFile != nullptr);
        return m_androidFile->seek(position);
    }
#endif

    ZB_ASSERT(m_file != nullptr);

    if (std::fseek(m_file.get(), static_cast<long>(position), SEEK_SET))
        return base::nullOpt;

    return tell();
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::tell()
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZB_ASSERT(m_androidFile != nullptr);
        return m_androidFile->tell();
    }
#endif

    ZB_ASSERT(m_file != nullptr);

    const auto position = std::ftell(m_file.get());
    return position < 0 ? base::nullOpt : base::makeOptional(static_cast<base::SizeT>(position));
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::getSize()
{
#ifdef ZA_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        ZB_ASSERT(m_androidFile != nullptr);
        return m_androidFile->getSize();
    }
#endif

    ZB_ASSERT(m_file != nullptr);

    const auto position = tell().value();
    std::fseek(m_file.get(), 0, SEEK_END);

    base::Optional<base::SizeT> size = tell(); // Use a single local variable for NRVO

    if (!seek(position).hasValue())
    {
        size.reset();
        return size; // Empty optional
    }

    return size;
}


////////////////////////////////////////////////////////////
FileInputStream::FileInputStream(base::PassKey<FileInputStream>&&, base::UniquePtr<std::FILE, FileCloser>&& file) :
    m_file(ZB_MOVE(file))
{
}


////////////////////////////////////////////////////////////
#ifdef ZA_SYSTEM_ANDROID
FileInputStream::FileInputStream(base::PassKey<FileInputStream>&&, base::UniquePtr<priv::ResourceStream>&& androidFile) :
    m_androidFile(ZB_MOVE(androidFile))
{
}
#endif

} // namespace za
