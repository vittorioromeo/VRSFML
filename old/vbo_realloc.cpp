base::SizeT vboCapacity{0u}; //!< Currently allocated capacity of the VBO
base::SizeT eboCapacity{0u}; //!< Currently allocated capacity of the EBO

[[gnu::always_inline]] void reallocObjectIfNeeded(GLenum type, auto& object, base::SizeT& capacity, base::SizeT byteCount)
{
    if (byteCount <= capacity) [[likely]]
        return;

    const auto newCapacity = (capacity * 3u / 2u) + byteCount;

    object.bind();
    glCheck(glBufferData(type, static_cast<GLsizeiptr>(newCapacity), nullptr, GL_DYNAMIC_DRAW));

    capacity = newCapacity;
}

[[gnu::always_inline]] void objectReallocAndMemcpy(
    GLenum          type,
    auto&           object,
    base::SizeT&    capacity,
    const void*     data,
    sf::base::SizeT byteCount)
{
    reallocObjectIfNeeded(type, object, capacity, byteCount);
    glCheck(glBufferSubData(type, 0u, byteCount, data));
}

[[gnu::always_inline]] void vboReallocAndMemcpy(const void* data, sf::base::SizeT byteCount)
{
    objectReallocAndMemcpy(GL_ARRAY_BUFFER, vbo, vboCapacity, data, byteCount);
}

[[gnu::always_inline]] void eboReallocAndMemcpy(const void* data, sf::base::SizeT byteCount)
{
    objectReallocAndMemcpy(GL_ELEMENT_ARRAY_BUFFER, ebo, eboCapacity, data, byteCount);
}
