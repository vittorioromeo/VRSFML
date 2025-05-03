


////////////////////////////////////////////////////////////
struct StringHash
{
    using is_transparent = void;

    [[nodiscard, gnu::always_inline, gnu::flatten]] sf::base::SizeT operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] sf::base::SizeT operator()(sf::base::StringView txt) const
    {
        return std::hash<std::string_view>{}({txt.data(), txt.size()});
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] sf::base::SizeT operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};


////////////////////////////////////////////////////////////
struct StringEq
{
    using is_transparent = void;

    [[nodiscard, gnu::always_inline, gnu::flatten]] bool operator()(const sf::base::StringView& a, const std::string& b) const
    {
        return a == sf::base::StringView{b};
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] bool operator()(const std::string& a, const sf::base::StringView& b) const
    {
        return sf::base::StringView{a} == b;
    }

    [[nodiscard, gnu::always_inline]] bool operator()(const sf::base::StringView& a, const sf::base::StringView& b) const
    {
        return a == b;
    }

    [[nodiscard, gnu::always_inline]] bool operator()(const std::string& a, const std::string& b) const
    {
        return a == b;
    }
};
