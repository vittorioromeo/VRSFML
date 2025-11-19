#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtin/Memcmp.hpp"
#include "SFML/Base/Priv/VectorUtils.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief A string class with Small String Optimization (SSO)
///
/// This class provides a replacement for `std::string`, designed
/// to fit within the SFML/Base module. It avoids heap allocations
/// for small strings by storing them within the object itself.
///
////////////////////////////////////////////////////////////
class [[nodiscard]] String
{
public:
    ////////////////////////////////////////////////////////////
    enum : bool
    {
        enableTrivialRelocation = true
    };


    ////////////////////////////////////////////////////////////
    using iterator       = char*;
    using const_iterator = const char*;
    using value_type     = char;


    ////////////////////////////////////////////////////////////
    enum [[nodiscard]] : SizeT
    {
        nPos = static_cast<SizeT>(-1)
    };


private:
    ////////////////////////////////////////////////////////////
    enum [[nodiscard]] : SizeT
    {
        ssoSize    = sizeof(char*) + sizeof(SizeT) * 2,
        maxSsoSize = ssoSize - 1,
    };


    ////////////////////////////////////////////////////////////
    enum [[nodiscard]] : unsigned char
    {
        flagIsHeap = 0x01, // Use the LSB of the last byte to indicate non-SSO
    };


    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] HeapRep
    {
        char* data{nullptr};
        SizeT size{0u};
        SizeT capacityAndFlag{0u}; // The LSB is the SSO flag (0 for heap)
    };


    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] SsoRep
    {
        alignas(HeapRep) char buffer[ssoSize]{};
    };


    ////////////////////////////////////////////////////////////
    union [[nodiscard]] RepUnion
    {
        SsoRep  sso{};
        HeapRep heap;
    } m_rep;


public:
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool isSso() const noexcept
    {
        return (m_rep.sso.buffer[maxSsoSize] & flagIsHeap) == 0;
    }


private:
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void setSsoSize(const SizeT size) noexcept
    {
        m_rep.sso.buffer[maxSsoSize] = static_cast<char>((maxSsoSize - size) << 1);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT getSsoSize() const noexcept
    {
        return maxSsoSize - static_cast<SizeT>(m_rep.sso.buffer[maxSsoSize] >> 1);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void setHeap(char* const data, const SizeT size, const SizeT capacity) noexcept
    {
        m_rep.heap.data            = data;
        m_rep.heap.size            = size;
        m_rep.heap.capacityAndFlag = (capacity << 1);

        // Set the flag in the high byte (byte 23) by setting the high bit
        // On little-endian, this is bit 56 of the 64-bit value
        constexpr SizeT flagMask = static_cast<SizeT>(flagIsHeap) << ((sizeof(SizeT) * 8) - 8);
        m_rep.heap.capacityAndFlag |= flagMask;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT getHeapCapacity() const noexcept
    {
        // Mask out the flag bit from the high byte before shifting
        constexpr SizeT flagMask = static_cast<SizeT>(flagIsHeap) << ((sizeof(SizeT) * 8) - 8);
        return (m_rep.heap.capacityAndFlag & ~flagMask) >> 1;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void setSize(const SizeT newSize) noexcept
    {
        if (isSso())
            setSsoSize(newSize);
        else
            m_rep.heap.size = newSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void setSizeAndTerminate(const SizeT newSize) noexcept
    {
        setSize(newSize);
        data()[newSize] = '\0';
    }


    ////////////////////////////////////////////////////////////
    void createFrom(const char* cStr, SizeT count);
    void grow(SizeT minCapacity);


public:
    ////////////////////////////////////////////////////////////
    constexpr explicit String() noexcept : m_rep{}
    {
        setSizeAndTerminate(0);
    }


    ////////////////////////////////////////////////////////////
    /* implicit */ String(const char* cStr);
    explicit String(const char* cStr, SizeT count);
    explicit String(StringView view);


    ////////////////////////////////////////////////////////////
    template <typename AnsiStringLike>
        requires isSame<typename AnsiStringLike::value_type, char>
    explicit String(const AnsiStringLike& ansiString) : String{ansiString.data(), ansiString.size()}
    {
    }


    ////////////////////////////////////////////////////////////
    String(const String& other);


    ////////////////////////////////////////////////////////////
    String(String&& other) noexcept;


    ////////////////////////////////////////////////////////////
    constexpr ~String()
    {
        if (!isSso())
            priv::VectorUtils::deallocate(m_rep.heap.data, getHeapCapacity() + 1);
    }


    ////////////////////////////////////////////////////////////
    String& operator=(const String& other);
    String& operator=(String&& other) noexcept;
    String& operator=(const char* cStr);
    String& operator=(StringView view);


    ////////////////////////////////////////////////////////////
    String& operator+=(char c);
    String& operator+=(const String& other);
    String& operator+=(const char* cStr);
    String& operator+=(StringView view);


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr iterator begin() noexcept
    {
        return data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const_iterator begin() const noexcept
    {
        return data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const_iterator cbegin() const noexcept
    {
        return data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr iterator end() noexcept
    {
        return data() + size();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const_iterator end() const noexcept
    {
        return data() + size();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const_iterator cend() const noexcept
    {
        return data() + size();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr SizeT size() const noexcept
    {
        return isSso() ? getSsoSize() : m_rep.heap.size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr SizeT capacity() const noexcept
    {
        return isSso() ? maxSsoSize : getHeapCapacity();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr bool empty() const noexcept
    {
        return size() == 0;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const char* data() const noexcept
    {
        return isSso() ? m_rep.sso.buffer : m_rep.heap.data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char* data() noexcept
    {
        return isSso() ? m_rep.sso.buffer : m_rep.heap.data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const char* cStr() const noexcept
    {
        return data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char& operator[](const SizeT index) noexcept
    {
        SFML_BASE_ASSERT(index < size() + 1u);
        return data()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char operator[](const SizeT index) const noexcept
    {
        SFML_BASE_ASSERT(index < size() + 1u);
        return data()[index];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char& front() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return data()[0];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char front() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return data()[0];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char& back() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return data()[size() - 1u];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr char back() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return data()[size() - 1u];
    }


    ////////////////////////////////////////////////////////////
    void clear() noexcept;
    void pushBack(char ch);


    ////////////////////////////////////////////////////////////
    String& append(const String& str);
    String& append(StringView view);
    String& append(char c);
    String& append(const char* cStr);
    String& append(const char* cStr, SizeT count);


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] constexpr StringView toStringView() const noexcept
    {
        return StringView{data(), size()};
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] constexpr operator StringView() const noexcept
    {
        return toStringView();
    }


    ////////////////////////////////////////////////////////////
    void reserve(SizeT newCapacity);


    ////////////////////////////////////////////////////////////
    void erase(SizeT index, SizeT count = nPos);
    void assign(const char* cStr, SizeT count);
    void insert(SizeT pos, char c);
    void insert(SizeT pos, const char* cStr);


    ////////////////////////////////////////////////////////////
    friend void swap(String& lhs, String& rhs) noexcept;
};


////////////////////////////////////////////////////////////
[[nodiscard]] String operator+(char lhs, const String& rhs);
[[nodiscard]] String operator+(const String& lhs, char rhs);
[[nodiscard]] String operator+(const char* lhs, const String& rhs);
[[nodiscard]] String operator+(const String& lhs, const char* rhs);
[[nodiscard]] String operator+(StringView lhs, const String& rhs);
[[nodiscard]] String operator+(const String& lhs, StringView rhs);
[[nodiscard]] String operator+(const String& lhs, const String& rhs);


////////////////////////////////////////////////////////////
[[nodiscard, gnu::flatten, gnu::always_inline]] inline constexpr bool operator==(const String& lhs, const StringView rhs) noexcept
{
    if (lhs.size() != rhs.size())
        return false;

    return SFML_BASE_MEMCMP(lhs.data(), rhs.data(), lhs.size()) == 0;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator==(const StringView lhs, const String& rhs) noexcept
{
    return rhs == lhs;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator==(const String& lhs, const String& rhs) noexcept
{
    return lhs.toStringView() == rhs.toStringView();
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator==(const String& lhs, const char* const rhs) noexcept
{
    return lhs.toStringView() == StringView(rhs);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator==(const char* const lhs, const String& rhs) noexcept
{
    return StringView(lhs) == rhs.toStringView();
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator!=(const String& lhs, const String& rhs) noexcept
{
    return !(lhs == rhs);
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator<(const String& lhs, const String& rhs) noexcept
{
    return lhs.toStringView() < rhs.toStringView();
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator<=(const String& lhs, const String& rhs) noexcept
{
    return lhs.toStringView() <= rhs.toStringView();
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator>(const String& lhs, const String& rhs) noexcept
{
    return lhs.toStringView() > rhs.toStringView();
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline constexpr bool operator>=(const String& lhs, const String& rhs) noexcept
{
    return lhs.toStringView() >= rhs.toStringView();
}

} // namespace sf::base


namespace sf::base::literals
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline String operator""_s(const char* const cStr, const SizeT len) noexcept
{
    return String{cStr, len};
}

} // namespace sf::base::literals
