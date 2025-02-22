#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"

#include "SFML/Base/InPlacePImpl.hpp"

#include <iosfwd>


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_SYSTEM_API Path
{
public:
#if defined(SFML_SYSTEM_EMSCRIPTEN) || defined(SFML_SYSTEM_LINUX_OR_BSD)
    using value_type = char;
#else
    using value_type = wchar_t;
#endif

    [[nodiscard]] static Path tempDirectoryPath();

    [[nodiscard]] explicit Path();

    template <typename T>
    [[nodiscard]] Path(const T& source);

    template <typename T>
    [[nodiscard]] Path(const T* source);

    ~Path();

    Path(const Path&);
    Path& operator=(const Path&);

    Path(Path&&) noexcept;
    Path& operator=(Path&&) noexcept;

    [[nodiscard]] Path filename() const;
    [[nodiscard]] Path extension() const;
    [[nodiscard]] Path absolute() const;

    [[nodiscard]] const value_type* c_str() const;     // TODO P1: support other platforms
    [[nodiscard]] const char*       toCharPtr() const; // TODO P1: support other platforms

    template <typename T>
    T to() const;

    [[nodiscard]] bool remove() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool exists() const;

    Path& operator/=(const Path& rhs);

    friend Path          operator/(const Path& lhs, const Path& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    template <typename T>
    bool operator==(const T* str) const;

    template <typename T>
    bool operator!=(const T* str) const;

    template <typename T>
    bool operator==(const T& str) const;

    template <typename T>
    bool operator!=(const T& str) const;

private:
    [[nodiscard]] Path(int, const void* fsPath);

    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl;
};


} // namespace sf
