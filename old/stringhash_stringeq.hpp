


////////////////////////////////////////////////////////////
struct StringHash
{
    using is_transparent = void;

    [[nodiscard, gnu::always_inline, gnu::flatten]] zb::SizeT operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] zb::SizeT operator()(zb::StringView txt) const
    {
        return std::hash<std::string_view>{}({txt.data(), txt.size()});
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] zb::SizeT operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};


////////////////////////////////////////////////////////////
struct StringEq
{
    using is_transparent = void;

    [[nodiscard, gnu::always_inline, gnu::flatten]] bool operator()(const zb::StringView& a, const std::string& b) const
    {
        return a == zb::StringView{b};
    }

    [[nodiscard, gnu::always_inline, gnu::flatten]] bool operator()(const std::string& a, const zb::StringView& b) const
    {
        return zb::StringView{a} == b;
    }

    [[nodiscard, gnu::always_inline]] bool operator()(const zb::StringView& a, const zb::StringView& b) const
    {
        return a == b;
    }

    [[nodiscard, gnu::always_inline]] bool operator()(const std::string& a, const std::string& b) const
    {
        return a == b;
    }
};
