#include <Doctest.hpp>

#include <cmath>

namespace doctest::Matchers
{

bool operator==(float value, const WithinRel& withinRel)
{
    return static_cast<double>(value) == doctest::Approx(withinRel.target).epsilon(withinRel.epsilon);
}

bool operator==(float value, const WithinAbs& withinAbs)
{
    return std::abs(value) == WithinRel{withinAbs.target, withinAbs.epsilon};
}

} // namespace doctest::Matchers

namespace Catch
{

Approx& Approx::margin(double newMargin)
{
    epsilon(newMargin);
    return *this;
}

} // namespace Catch

namespace doctest
{

doctest::String StringMaker<char32_t>::convert(const char32_t&)
{
    return ""; // TODO
}
doctest::String StringMaker<Matchers::WithinRel>::convert(const Matchers::WithinRel&)
{
    return ""; // TODO
}

doctest::String StringMaker<Matchers::WithinAbs>::convert(const Matchers::WithinAbs&)
{
    return ""; // TODO
}

doctest::String StringMaker<Catch::Approx>::convert(const Catch::Approx&)
{
    return ""; // TODO
}

} // namespace doctest

#ifdef SFML_ENABLE_PCH

int main(int argc, char** argv)
{
    return doctest::Context(argc, argv).run();
}

#endif
