
#define SFML_PRIV_DEFINE_HELPER_GET_FN(name, ...)                                                                    \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline constexpr Vector2f name(this const auto& self) \
    {                                                                                                                \
        const auto& [pos, size] = self.getGlobalBounds();                                                            \
        return pos + Vector2f __VA_ARGS__;                                                                           \
    }

    // TODO P0:
    SFML_PRIV_DEFINE_HELPER_GET_FN(getTopLeft, {0.f, 0.f});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getTopCenter, {size.x / 2.f, 0.f});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getTopRight, {size.x, 0.f});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getCenterLeft, {0.f, size.y / 2.f});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getCenter, {size.x / 2.f, size.y / 2.f});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getCenterRight, {size.x, size.y / 2.f});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomLeft, {0.f, size.y});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomCenter, {size.x / 2.f, size.y});
    SFML_PRIV_DEFINE_HELPER_GET_FN(getBottomRight, {size.x, size.y});
