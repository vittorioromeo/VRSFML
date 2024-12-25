////////////////////////////////////////////////////////////
[[gnu::always_inline, gnu::flatten]] inline constexpr void appendPreTransformedQuadVertices(
    sf::Vertex*&         vertexPtr,
    const sf::Transform& t,
    const sf::Vertex&    a,
    const sf::Vertex&    b,
    const sf::Vertex&    c,
    const sf::Vertex&    d) noexcept
{
    const sf::Vector2f ap = a.position;
    const sf::Vector2f bp = b.position;
    const sf::Vector2f cp = c.position;
    const sf::Vector2f dp = d.position;

    __builtin_assume(ap.x == cp.x);
    __builtin_assume(bp.x == dp.x);

    __builtin_assume(ap.y == bp.y);
    __builtin_assume(cp.y == dp.y);

    const float atx = t.m_a00 * ap.x + t.m_a01 * ap.y + t.m_a02;
    const float btx = t.m_a00 * bp.x + t.m_a01 * ap.y + t.m_a02;
    const float ctx = t.m_a00 * ap.x + t.m_a01 * cp.y + t.m_a02;
    const float dtx = t.m_a00 * bp.x + t.m_a01 * cp.y + t.m_a02;

    const float aty = t.m_a10 * ap.x + t.m_a11 * ap.y + t.m_a12;
    const float bty = t.m_a10 * bp.x + t.m_a11 * ap.y + t.m_a12;
    const float cty = t.m_a10 * ap.x + t.m_a11 * cp.y + t.m_a12;
    const float dty = t.m_a10 * bp.x + t.m_a11 * cp.y + t.m_a12;

    vertexPtr[0] = {{atx, aty}, a.color, a.texCoords};
    vertexPtr[1] = {{btx, bty}, b.color, b.texCoords};
    vertexPtr[2] = {{ctx, cty}, c.color, c.texCoords};
    vertexPtr[3] = {{dtx, dty}, d.color, d.texCoords};

    vertexPtr += 4;
}
