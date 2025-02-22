#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/FileInputStream.hpp"

#include "SFML/Base/Optional.hpp"

#ifdef SFML_SYSTEM_ANDROID
#include "SFML/System/Android/Activity.hpp"
#include "SFML/System/Android/ResourceStream.hpp"
#endif

#include "SFML/System/FileUtils.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
void FileInputStream::FileCloser::operator()(std::FILE* file)
{
    if (file != nullptr)
        std::fclose(file);
}


////////////////////////////////////////////////////////////
FileInputStream::~FileInputStream() = default;


////////////////////////////////////////////////////////////
FileInputStream::FileInputStream(FileInputStream&&) noexcept = default;


////////////////////////////////////////////////////////////
FileInputStream& FileInputStream::operator=(FileInputStream&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<FileInputStream> FileInputStream::open(const Path& filename)
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        auto androidFile = base::makeUnique<priv::ResourceStream>(filename);
        return androidFile->tell().hasValue()
                   ? base::makeOptional<FileInputStream>(base::PassKey<FileInputStream>{}, SFML_BASE_MOVE(androidFile))
                   : base::nullOpt;
    }
#endif

    if (auto file = base::UniquePtr<std::FILE, FileCloser>(openFile(filename, "rb")))
        return base::makeOptional<FileInputStream>(base::PassKey<FileInputStream>{}, SFML_BASE_MOVE(file));

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::read(void* data, base::SizeT size)
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile != nullptr);
        return m_androidFile->read(data, size);
    }
#endif

    SFML_BASE_ASSERT(m_file != nullptr);
    return base::makeOptional(std::fread(data, 1, size, m_file.get()));
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::seek(base::SizeT position)
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile != nullptr);
        return m_androidFile->seek(position);
    }
#endif

    SFML_BASE_ASSERT(m_file != nullptr);

    if (std::fseek(m_file.get(), static_cast<long>(position), SEEK_SET))
        return base::nullOpt;

    return tell();
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::tell()
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile != nullptr);
        return m_androidFile->tell();
    }
#endif

    SFML_BASE_ASSERT(m_file != nullptr);

    const auto position = std::ftell(m_file.get());
    return position < 0 ? base::nullOpt : base::makeOptional(static_cast<base::SizeT>(position));
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> FileInputStream::getSize()
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile != nullptr);
        return m_androidFile->getSize();
    }
#endif

    SFML_BASE_ASSERT(m_file != nullptr);

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
m_file(SFML_BASE_MOVE(file))
{
}


////////////////////////////////////////////////////////////
#ifdef SFML_SYSTEM_ANDROID
FileInputStream::FileInputStream(base::PassKey<FileInputStream>&&, base::UniquePtr<priv::ResourceStream>&& androidFile) :
m_androidFile(SFML_BASE_MOVE(androidFile))
{
}
#endif

} // namespace sf
