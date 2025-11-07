// LICENSE AND COPYRIGHT (C) INFORMATION
// (Assume your license here)

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/String.hpp"

#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"

#include <string>


namespace sf::base
{
////////////////////////////////////////////////////////////
void String::grow(const SizeT minCapacity)
{
    const SizeT oldCapacity = capacity();
    // Geometric growth, plus a little to avoid growing by 0 or 1.
    const SizeT newCapacity   = oldCapacity + oldCapacity / 2 + 8;
    const SizeT finalCapacity = newCapacity > minCapacity ? newCapacity : minCapacity;

    char* newData = priv::VectorUtils::allocate<char>(finalCapacity + 1); // +1 for null terminator

    const SizeT currentSize = size();
    SFML_BASE_MEMCPY(newData, data(), currentSize);

    if (!isSso())
        priv::VectorUtils::deallocate(m_rep.heap.data, oldCapacity + 1);

    setHeap(newData, currentSize, finalCapacity);
    setNullTerminator();
}


////////////////////////////////////////////////////////////
void String::createFrom(const char* const cStr, const SizeT count)
{
    SFML_BASE_ASSERT(cStr != nullptr);

    if (count <= maxSsoSize)
    {
        SFML_BASE_MEMCPY(m_rep.sso.buffer, cStr, count);
        setSsoSize(count);
        setNullTerminator();
    }
    else
    {
        char* newData = priv::VectorUtils::allocate<char>(count + 1);
        SFML_BASE_MEMCPY(newData, cStr, count);
        setHeap(newData, count, count);
        setNullTerminator();
    }
}


////////////////////////////////////////////////////////////
String::String(const char* cStr) : String{cStr, SFML_BASE_STRLEN(cStr)}
{
    SFML_BASE_ASSERT(cStr != nullptr);
}


////////////////////////////////////////////////////////////
String::String(const char* cStr, const SizeT count) : m_rep{}
{
    createFrom(cStr, count);
}


////////////////////////////////////////////////////////////
String::String(StringView view) : String{view.data(), view.size()}
{
}


////////////////////////////////////////////////////////////
template <typename AnsiStringLike>
    requires isSame<typename AnsiStringLike::value_type, char>
String::String(const AnsiStringLike& ansiString) : String{ansiString.data(), ansiString.size()}
{
}


////////////////////////////////////////////////////////////
template String::String(const std::string&);


////////////////////////////////////////////////////////////
String::String(const String& other) : String{other.data(), other.size()}
{
}


////////////////////////////////////////////////////////////
String::String(String&& other) noexcept : m_rep{}
{
    SFML_BASE_MEMCPY(&m_rep, &other.m_rep, sizeof(m_rep));

    if (!other.isSso())
    {
        // Reset the other string to a valid empty state
        other.setSsoSize(0);
        other.setNullTerminator();
    }
}


////////////////////////////////////////////////////////////
String& String::operator=(const String& other)
{
    if (this == &other)
        return *this;

    return *this = other.toStringView();
}


////////////////////////////////////////////////////////////
String& String::operator=(String&& other) noexcept
{
    if (this == &other)
        return *this;

    // Free our own resources before taking others
    if (!isSso())
        priv::VectorUtils::deallocate(m_rep.heap.data, getHeapCapacity() + 1u);

    SFML_BASE_MEMCPY(&m_rep, &other.m_rep, sizeof(m_rep));

    if (!other.isSso())
    {
        // Reset the other string to a valid empty state
        other.setSsoSize(0);
        other.setNullTerminator();
    }

    return *this;
}


////////////////////////////////////////////////////////////
String& String::operator=(const StringView view)
{
    if (!isSso() && view.size() <= getHeapCapacity())
    {
        SFML_BASE_MEMCPY(m_rep.heap.data, view.data(), view.size());
        m_rep.heap.size = view.size();
        setNullTerminator();
        return *this;
    }

    if (!isSso())
        priv::VectorUtils::deallocate(m_rep.heap.data, getHeapCapacity() + 1u);

    createFrom(view.data(), view.size());
    return *this;
}


////////////////////////////////////////////////////////////
String& String::operator=(const char* cStr)
{
    return *this = StringView{cStr};
}


////////////////////////////////////////////////////////////
String& String::operator+=(const char c)
{
    return append(c);
}


////////////////////////////////////////////////////////////
String& String::operator+=(const String& other)
{
    return append(other);
}


////////////////////////////////////////////////////////////
String& String::operator+=(const char* const cStr)
{
    SFML_BASE_ASSERT(cStr != nullptr);
    return append(StringView{cStr});
}


////////////////////////////////////////////////////////////
String& String::operator+=(const StringView view)
{
    return append(view);
}


////////////////////////////////////////////////////////////
void String::clear() noexcept
{
    if (isSso())
        setSsoSize(0u);
    else
        m_rep.heap.size = 0u;

    setNullTerminator();
}


////////////////////////////////////////////////////////////
void String::pushBack(const char c)
{
    const SizeT currentSize = size();
    if (currentSize == capacity())
        grow(currentSize + 1);

    char* const d  = data();
    d[currentSize] = c;

    if (isSso())
        setSsoSize(currentSize + 1);
    else
        m_rep.heap.size = currentSize + 1;

    setNullTerminator();
}


////////////////////////////////////////////////////////////
String& String::append(const char c)
{
    pushBack(c);
    return *this;
}


////////////////////////////////////////////////////////////
String& String::append(const String& str)
{
    return append(str.toStringView());
}


////////////////////////////////////////////////////////////
String& String::append(const StringView view)
{
    const SizeT otherSize = view.size();

    if (otherSize == 0u)
        return *this;

    const SizeT currentSize = size();

    if (currentSize + otherSize > capacity())
        grow(currentSize + otherSize);

    SFML_BASE_MEMCPY(data() + currentSize, view.data(), otherSize);

    if (isSso())
        setSsoSize(currentSize + otherSize);
    else
        m_rep.heap.size = currentSize + otherSize;

    setNullTerminator();
    return *this;
}


////////////////////////////////////////////////////////////
String& String::append(const char* const cStr)
{
    SFML_BASE_ASSERT(cStr != nullptr);
    return append(StringView{cStr});
}


////////////////////////////////////////////////////////////
String& String::append(const char* const cStr, const SizeT count)
{
    SFML_BASE_ASSERT(cStr != nullptr);
    return append(StringView{cStr, count});
}


////////////////////////////////////////////////////////////
void String::reserve(const SizeT newCapacity)
{
    if (newCapacity > capacity())
        grow(newCapacity);
}


////////////////////////////////////////////////////////////
void String::erase(const SizeT index)
{
    SFML_BASE_ASSERT(index < size() && "Index is out of bounds");

    const SizeT currentSize = size();

    if (isSso())
    {
        SFML_BASE_MEMCPY(m_rep.sso.buffer + index, m_rep.sso.buffer + index + 1, currentSize - index);
        setSsoSize(currentSize - 1u);
    }
    else
    {
        SFML_BASE_MEMCPY(m_rep.heap.data + index, m_rep.heap.data + index + 1, currentSize - index);
        m_rep.heap.size = currentSize - 1u;
    }

    setNullTerminator();
}


////////////////////////////////////////////////////////////
void String::assign(const char* const cStr, const SizeT count)
{
    SFML_BASE_ASSERT(cStr != nullptr);

    if (!isSso())
        priv::VectorUtils::deallocate(m_rep.heap.data, getHeapCapacity() + 1);

    createFrom(cStr, count);
}


////////////////////////////////////////////////////////////
void String::insert(const SizeT pos, const char* const cStr)
{
    SFML_BASE_ASSERT(pos <= size() && "Insertion position is out of bounds");
    SFML_BASE_ASSERT(cStr != nullptr);

    const SizeT insertCount = SFML_BASE_STRLEN(cStr);
    if (insertCount == 0u)
        return;

    const SizeT oldSize = size();
    const SizeT newSize = oldSize + insertCount;

    // 1. Ensure we have enough capacity. This might reallocate and change `data()`.
    reserve(newSize);

    char* d = data();

    // 2. Make space for the new content by shifting the existing part to the right.
    // We must use memmove as the source and destination memory regions overlap.
    if (pos < oldSize)
    {
        SFML_BASE_MEMMOVE(d + pos + insertCount, // Destination
                          d + pos,               // Source
                          oldSize - pos);        // Number of characters to move
    }

    // 3. Copy the new content into the created space.
    SFML_BASE_MEMCPY(d + pos, cStr, insertCount);

    // 4. Update the size.
    if (isSso())
        setSsoSize(newSize);
    else
        m_rep.heap.size = newSize;

    // 5. Ensure null-termination.
    setNullTerminator();
}


////////////////////////////////////////////////////////////
void swap(String& lhs, String& rhs) noexcept
{
    alignas(String::RepUnion) char temp[sizeof(String::RepUnion)];

    SFML_BASE_MEMCPY(&temp, &lhs.m_rep, sizeof(String::RepUnion));
    SFML_BASE_MEMCPY(&lhs.m_rep, &rhs.m_rep, sizeof(String::RepUnion));
    SFML_BASE_MEMCPY(&rhs.m_rep, &temp, sizeof(String::RepUnion));
}


////////////////////////////////////////////////////////////
String operator+(const char lhs, const String& rhs)
{
    String result;
    result.reserve(1u + rhs.size());

    result.append(lhs);
    result.append(rhs);

    return result;
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const char rhs)
{
    String result;
    result.reserve(lhs.size() + 1u);

    result.append(lhs);
    result.append(rhs);

    return result;
}


////////////////////////////////////////////////////////////
String operator+(const char* const lhs, const String& rhs)
{
    SFML_BASE_ASSERT(lhs != nullptr);
    const SizeT lhsSize = SFML_BASE_STRLEN(lhs);

    String result;
    result.reserve(lhsSize + rhs.size());

    result.append(lhs);
    result.append(rhs);

    return result;
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const char* const rhs)
{
    SFML_BASE_ASSERT(rhs != nullptr);
    const SizeT rhsSize = SFML_BASE_STRLEN(rhs);

    String result;
    result.reserve(lhs.size() + rhsSize);

    result.append(lhs);
    result.append(rhs);

    return result;
}


////////////////////////////////////////////////////////////
String operator+(const StringView lhs, const String& rhs)
{
    String result;
    result.reserve(lhs.size() + rhs.size());

    result.append(lhs);
    result.append(rhs);

    return result;
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const StringView rhs)
{
    String result;
    result.reserve(lhs.size() + rhs.size());

    result.append(lhs);
    result.append(rhs);

    return result;
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const String& rhs)
{
    String result;
    result.reserve(lhs.size() + rhs.size());

    result.append(lhs);
    result.append(rhs);

    return result;
}

} // namespace sf::base
