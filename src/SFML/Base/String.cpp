// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/String.hpp"

#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Strlen.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline sf::base::String operatorPlusImpl(
    const char* const     lhs,
    const sf::base::SizeT lhsSize,
    const char* const     rhs,
    const sf::base::SizeT rhsSize)
{
    sf::base::String result;
    result.reserve(lhsSize + rhsSize);

    result.append(sf::base::StringView{lhs, lhsSize});
    result.append(sf::base::StringView{rhs, rhsSize});

    return result;
}

} // namespace


namespace sf::base
{
////////////////////////////////////////////////////////////
void String::grow(const SizeT minCapacity)
{
    const SizeT oldCapacity   = capacity();
    const SizeT newCapacity   = oldCapacity + oldCapacity / 2u + 8u;
    const SizeT finalCapacity = newCapacity > minCapacity ? newCapacity : minCapacity;

    char* newData = priv::VectorUtils::allocate<char>(finalCapacity + 1u); // +1 for null terminator

    const SizeT currentSize = size();
    SFML_BASE_MEMCPY(newData, data(), currentSize);

    if (!isSso())
        priv::VectorUtils::deallocate(m_rep.heap.data, oldCapacity + 1u);

    setHeap(newData, currentSize, finalCapacity);
    data()[currentSize] = '\0'; // `setHeap` doesn't null terminate
}


////////////////////////////////////////////////////////////
void String::createFrom(const char* const cStr, const SizeT count)
{
    SFML_BASE_ASSERT(cStr != nullptr);

    if (count <= maxSsoSize)
    {
        SFML_BASE_MEMCPY(m_rep.sso.buffer, cStr, count);
        setSizeAndTerminate(count);
    }
    else
    {
        char* const newData = priv::VectorUtils::allocate<char>(count + 1);
        SFML_BASE_MEMCPY(newData, cStr, count);
        setHeap(newData, count, count);
        newData[count] = '\0'; // `setHeap` doesn't null terminate
    }
}


////////////////////////////////////////////////////////////
String::String(const char* const cStr) : String{cStr, SFML_BASE_STRLEN(cStr)}
{
    SFML_BASE_ASSERT(cStr != nullptr);
}


////////////////////////////////////////////////////////////
String::String(const char* const cStr, const SizeT count) : m_rep{}
{
    createFrom(cStr, count);
}


////////////////////////////////////////////////////////////
String::String(const StringView view) : String{view.data(), view.size()}
{
}


////////////////////////////////////////////////////////////
String::String(const String& other) : String{other.data(), other.size()}
{
}


////////////////////////////////////////////////////////////
String::String(String&& other) noexcept : m_rep{}
{
    SFML_BASE_MEMCPY(&m_rep, &other.m_rep, sizeof(m_rep));
    other.setSsoSize(0); // prevent double-free
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
    other.setSsoSize(0); // prevent double-free

    return *this;
}


////////////////////////////////////////////////////////////
String& String::operator=(const StringView view)
{
    const char* const src     = view.data();
    const SizeT       newSize = view.size();

    const char* const myData = data();
    const SizeT       mySize = size();

    const bool srcInsideThis = (src >= myData) && (src < myData + mySize);

    if (srcInsideThis)
    {
        const auto offset = static_cast<SizeT>(src - myData);

        // If it fits in current capacity we can memmove in-place (safe for overlap).
        if (newSize <= capacity())
        {
            SFML_BASE_MEMMOVE(data(), data() + offset, newSize);
            setSizeAndTerminate(newSize);
            return *this;
        }

        // Need a new allocation; copy from the old buffer before deallocating it.
        char* const newData = priv::VectorUtils::allocate<char>(newSize + 1u);
        SFML_BASE_MEMCPY(newData, myData + offset, newSize);
        newData[newSize] = '\0';

        if (!isSso())
            priv::VectorUtils::deallocate(m_rep.heap.data, getHeapCapacity() + 1u);

        setHeap(newData, newSize, newSize);
        return *this;
    }

    if (!isSso() && view.size() <= getHeapCapacity())
    {
        SFML_BASE_MEMCPY(m_rep.heap.data, view.data(), view.size());
        m_rep.heap.size         = view.size();
        data()[m_rep.heap.size] = '\0';
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
    setSizeAndTerminate(0u);
}


////////////////////////////////////////////////////////////
void String::pushBack(const char c)
{
    const SizeT currentSize = size();
    if (currentSize == capacity())
        grow(currentSize + 1);

    char* const d  = data();
    d[currentSize] = c;

    setSizeAndTerminate(currentSize + 1);
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

    // Check for self-append
    const char* const src    = view.data();
    const char* const myData = data();

    const bool srcInsideThis = (src >= myData) && (src < myData + currentSize);

    // If we will reallocate, compute offset first so we can still copy from original location
    const SizeT srcOffset = srcInsideThis ? static_cast<SizeT>(src - myData) : 0u;

    if (currentSize + otherSize > capacity())
        grow(currentSize + otherSize);

    char* const newMyData = data(); // May have changed after grow

    if (srcInsideThis)
        SFML_BASE_MEMMOVE(newMyData + currentSize, newMyData + srcOffset, otherSize);
    else
        SFML_BASE_MEMCPY(newMyData + currentSize, src, otherSize);

    setSizeAndTerminate(currentSize + otherSize);
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
void String::resize(const SizeT newSize, const char c)
{
    const SizeT currentSize = size();

    if (newSize < currentSize)
    {
        setSizeAndTerminate(newSize);
        return;
    }

    if (newSize > currentSize)
    {
        reserve(newSize);

        char* const d = data();
        for (SizeT i = currentSize; i < newSize; ++i)
            d[i] = c;

        setSizeAndTerminate(newSize);
    }
}


////////////////////////////////////////////////////////////
void String::erase(const SizeT index, SizeT count)
{
    const SizeT currentSize = size();
    SFML_BASE_ASSERT(index < currentSize && "Index is out of bounds");

    // If count is nPos or goes past the end, clamp it to erase until the end.
    if (count == nPos || index + count > currentSize)
        count = currentSize - index;

    if (count == 0u)
        return;

    const SizeT tailLength = currentSize - (index + count);

    SFML_BASE_MEMMOVE(data() + index, data() + index + count, tailLength);
    setSizeAndTerminate(currentSize - count);
}


////////////////////////////////////////////////////////////
void String::assign(const char* const cStr, const SizeT count)
{
    SFML_BASE_ASSERT(cStr != nullptr);
    this->operator=(StringView{cStr, count});
}


////////////////////////////////////////////////////////////
void String::insert(const SizeT pos, const char c)
{
    SFML_BASE_ASSERT(pos <= size() && "Insertion position is out of bounds");

    const SizeT oldSize = size();
    const SizeT newSize = oldSize + 1u;

    reserve(newSize);

    char* const d = data();

    // Make space for the new character
    if (pos < oldSize)
        SFML_BASE_MEMMOVE(d + pos + 1, d + pos, oldSize - pos);

    // Insert the character
    d[pos] = c;

    setSizeAndTerminate(newSize);
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

    char* const d = data();

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
    setSizeAndTerminate(newSize);
}


////////////////////////////////////////////////////////////
void swap(String& lhs, String& rhs) noexcept
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"

    alignas(String::RepUnion) char temp[sizeof(String::RepUnion)];

    SFML_BASE_MEMCPY(&temp, &lhs.m_rep, sizeof(String::RepUnion));
    SFML_BASE_MEMCPY(&lhs.m_rep, &rhs.m_rep, sizeof(String::RepUnion));
    SFML_BASE_MEMCPY(&rhs.m_rep, &temp, sizeof(String::RepUnion));

#pragma GCC diagnostic pop
}


////////////////////////////////////////////////////////////
String operator+(const char lhs, const String& rhs)
{
    return operatorPlusImpl(&lhs, 1u, rhs.data(), rhs.size());
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const char rhs)
{
    return operatorPlusImpl(lhs.data(), lhs.size(), &rhs, 1u);
}


////////////////////////////////////////////////////////////
String operator+(const char* const lhs, const String& rhs)
{
    SFML_BASE_ASSERT(lhs != nullptr);
    return operatorPlusImpl(lhs, SFML_BASE_STRLEN(lhs), rhs.data(), rhs.size());
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const char* const rhs)
{
    SFML_BASE_ASSERT(rhs != nullptr);
    return operatorPlusImpl(lhs.data(), lhs.size(), rhs, SFML_BASE_STRLEN(rhs));
}


////////////////////////////////////////////////////////////
String operator+(const StringView lhs, const String& rhs)
{
    return operatorPlusImpl(lhs.data(), lhs.size(), rhs.data(), rhs.size());
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const StringView rhs)
{
    return operatorPlusImpl(lhs.data(), lhs.size(), rhs.data(), rhs.size());
}


////////////////////////////////////////////////////////////
String operator+(const String& lhs, const String& rhs)
{
    return operatorPlusImpl(lhs.data(), lhs.size(), rhs.data(), rhs.size());
}

} // namespace sf::base
