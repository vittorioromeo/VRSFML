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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/System/InPlacePImpl.hpp>

#include <iosfwd>
#include <string>
#include <string_view>


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] Path
{
public:
    [[nodiscard]] static Path tempDirectoryPath();

    [[nodiscard]] Path();
    [[nodiscard]] Path(const char* path);
    [[nodiscard]] Path(const std::string& path);
    [[nodiscard]] Path(std::string_view path);

    ~Path();

    Path(const Path&);
    Path& operator=(const Path&);

    Path(Path&&) noexcept;
    Path& operator=(Path&&) noexcept;

    [[nodiscard]] Path filename() const;
    [[nodiscard]] Path extension() const;

    [[nodiscard]] const wchar_t* c_str() const; // TODO: support other platforms
    [[nodiscard]] std::string    string() const;

    [[nodiscard]] operator std::string() const;

    [[nodiscard]] bool remove() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool exists() const;

    Path& operator/=(const Path& rhs);

    friend Path          operator/(const Path& lhs, const Path& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    friend bool operator==(const Path& path, const char* str);
    friend bool operator!=(const Path& path, const char* str);

    friend bool operator==(const Path& path, std::string_view str);
    friend bool operator!=(const Path& path, std::string_view str);

    friend bool operator==(const Path& path, const std::string& str);
    friend bool operator!=(const Path& path, const std::string& str);

private:
    [[nodiscard]] Path(int, const void* fsPath);

    struct Impl;
    priv::InPlacePImpl<Impl, 64> m_impl;
};


} // namespace sf
