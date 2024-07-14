////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/FileInputStream.hpp>

#include <SFML/Base/Optional.hpp>

#ifdef SFML_SYSTEM_ANDROID
#include <SFML/System/Android/Activity.hpp>
#include <SFML/System/Android/ResourceStream.hpp>
#endif

#include <SFML/System/Path.hpp>

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>

#include <cstddef>

namespace sf
{
////////////////////////////////////////////////////////////
void FileInputStream::FileCloser::operator()(std::FILE* file)
{
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
        if (androidFile->tell().hasValue())
            return sf::base::makeOptional<FileInputStream>(base::PassKey<FileInputStream>{}, SFML_BASE_MOVE(androidFile));
        return base::nullOpt;
    }
#endif

#ifdef SFML_SYSTEM_WINDOWS
    if (auto file = base::UniquePtr<std::FILE, FileCloser>(_wfopen(filename.c_str(), L"rb")))
#else
    if (auto file = base::UniquePtr<std::FILE, FileCloser>(std::fopen(filename.c_str(), "rb")))
#endif
        return sf::base::makeOptional<FileInputStream>(base::PassKey<FileInputStream>{}, SFML_BASE_MOVE(file));

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> FileInputStream::read(void* data, std::size_t size)
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile);
        return m_androidFile->read(data, size);
    }
#endif

    SFML_BASE_ASSERT(m_file);
    return sf::base::makeOptional(std::fread(data, 1, size, m_file.get()));
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> FileInputStream::seek(std::size_t position)
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile);
        return m_androidFile->seek(position);
    }
#endif

    SFML_BASE_ASSERT(m_file);

    if (std::fseek(m_file.get(), static_cast<long>(position), SEEK_SET))
        return base::nullOpt;

    return tell();
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> FileInputStream::tell()
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile);
        return m_androidFile->tell();
    }
#endif

    SFML_BASE_ASSERT(m_file);

    const auto position = std::ftell(m_file.get());
    return position < 0 ? base::nullOpt : sf::base::makeOptional(static_cast<std::size_t>(position));
}


////////////////////////////////////////////////////////////
base::Optional<std::size_t> FileInputStream::getSize()
{
#ifdef SFML_SYSTEM_ANDROID
    if (priv::getActivityStatesPtr() != nullptr)
    {
        SFML_BASE_ASSERT(m_androidFile);
        return m_androidFile->getSize();
    }
#endif

    SFML_BASE_ASSERT(m_file);

    const auto position = tell().value();
    std::fseek(m_file.get(), 0, SEEK_END);

    base::Optional<std::size_t> size = tell(); // Use a single local variable for NRVO

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
