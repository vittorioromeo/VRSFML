#include "SFML/Graphics/BlendMode.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <GraphicsUtil.hpp>

TEST_CASE("[Graphics] sf::BlendMode")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(sf::BlendMode));

        STATIC_CHECK(!SFML_BASE_IS_TRIVIAL(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_STANDARD_LAYOUT(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_AGGREGATE(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_COPYABLE(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(sf::BlendMode));
        STATIC_CHECK(SFML_BASE_IS_TRIVIALLY_ASSIGNABLE(sf::BlendMode, sf::BlendMode));

        STATIC_CHECK(sizeof(sf::BlendMode) <= 4); // should be packed via bitfields
    }

    SECTION("Construction")
    {
        SECTION("Combined color and alpha constructor using default parameter")
        {
            const auto blendMode = sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::SrcColor);

            CHECK(blendMode.colorSrcFactor == sf::BlendMode::Factor::Zero);
            CHECK(blendMode.colorDstFactor == sf::BlendMode::Factor::SrcColor);
            CHECK(blendMode.colorEquation == sf::BlendMode::Equation::Add);
            CHECK(blendMode.alphaSrcFactor == sf::BlendMode::Factor::Zero);
            CHECK(blendMode.alphaDstFactor == sf::BlendMode::Factor::SrcColor);
            CHECK(blendMode.alphaEquation == sf::BlendMode::Equation::Add);
        }

        SECTION("Combined color and alpha constructor")
        {
            const auto blendMode = sf::BlendMode::from(sf::BlendMode::Factor::Zero,
                                                       sf::BlendMode::Factor::SrcColor,
                                                       sf::BlendMode::Equation::ReverseSubtract);

            CHECK(blendMode.colorSrcFactor == sf::BlendMode::Factor::Zero);
            CHECK(blendMode.colorDstFactor == sf::BlendMode::Factor::SrcColor);
            CHECK(blendMode.colorEquation == sf::BlendMode::Equation::ReverseSubtract);
            CHECK(blendMode.alphaSrcFactor == sf::BlendMode::Factor::Zero);
            CHECK(blendMode.alphaDstFactor == sf::BlendMode::Factor::SrcColor);
            CHECK(blendMode.alphaEquation == sf::BlendMode::Equation::ReverseSubtract);
        }

        SECTION("Separate color and alpha constructor")
        {
            const sf::BlendMode blendMode(sf::BlendMode::Factor::Zero,
                                          sf::BlendMode::Factor::SrcColor,
                                          sf::BlendMode::Equation::ReverseSubtract,
                                          sf::BlendMode::Factor::OneMinusDstAlpha,
                                          sf::BlendMode::Factor::DstAlpha,
                                          sf::BlendMode::Equation::Max);
            CHECK(blendMode.colorSrcFactor == sf::BlendMode::Factor::Zero);
            CHECK(blendMode.colorDstFactor == sf::BlendMode::Factor::SrcColor);
            CHECK(blendMode.colorEquation == sf::BlendMode::Equation::ReverseSubtract);
            CHECK(blendMode.alphaSrcFactor == sf::BlendMode::Factor::OneMinusDstAlpha);
            CHECK(blendMode.alphaDstFactor == sf::BlendMode::Factor::DstAlpha);
            CHECK(blendMode.alphaEquation == sf::BlendMode::Equation::Max);
        }
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::One) ==
                  sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::One));
            CHECK(sf::BlendMode(sf::BlendMode::Factor::Zero,
                                sf::BlendMode::Factor::SrcColor,
                                sf::BlendMode::Equation::ReverseSubtract,
                                sf::BlendMode::Factor::OneMinusDstAlpha,
                                sf::BlendMode::Factor::DstAlpha,
                                sf::BlendMode::Equation::Max) ==
                  sf::BlendMode(sf::BlendMode::Factor::Zero,
                                sf::BlendMode::Factor::SrcColor,
                                sf::BlendMode::Equation::ReverseSubtract,
                                sf::BlendMode::Factor::OneMinusDstAlpha,
                                sf::BlendMode::Factor::DstAlpha,
                                sf::BlendMode::Equation::Max));

            CHECK_FALSE(sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::One) ==
                        sf::BlendMode::from(sf::BlendMode::Factor::One, sf::BlendMode::Factor::Zero));
            CHECK_FALSE(
                sf::BlendMode(sf::BlendMode::Factor::Zero,
                              sf::BlendMode::Factor::SrcColor,
                              sf::BlendMode::Equation::ReverseSubtract,
                              sf::BlendMode::Factor::OneMinusDstAlpha,
                              sf::BlendMode::Factor::DstAlpha,
                              sf::BlendMode::Equation::Max) ==
                sf::BlendMode(sf::BlendMode::Factor::One,
                              sf::BlendMode::Factor::SrcColor,
                              sf::BlendMode::Equation::ReverseSubtract,
                              sf::BlendMode::Factor::OneMinusDstAlpha,
                              sf::BlendMode::Factor::DstAlpha,
                              sf::BlendMode::Equation::Max));
        }

        SECTION("operator!=")
        {
            CHECK_FALSE(sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::One) !=
                        sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::One));
            CHECK_FALSE(
                sf::BlendMode(sf::BlendMode::Factor::Zero,
                              sf::BlendMode::Factor::SrcColor,
                              sf::BlendMode::Equation::ReverseSubtract,
                              sf::BlendMode::Factor::OneMinusDstAlpha,
                              sf::BlendMode::Factor::DstAlpha,
                              sf::BlendMode::Equation::Max) !=
                sf::BlendMode(sf::BlendMode::Factor::Zero,
                              sf::BlendMode::Factor::SrcColor,
                              sf::BlendMode::Equation::ReverseSubtract,
                              sf::BlendMode::Factor::OneMinusDstAlpha,
                              sf::BlendMode::Factor::DstAlpha,
                              sf::BlendMode::Equation::Max));

            CHECK(sf::BlendMode::from(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::One) !=
                  sf::BlendMode::from(sf::BlendMode::Factor::One, sf::BlendMode::Factor::Zero));
            CHECK(sf::BlendMode(sf::BlendMode::Factor::Zero,
                                sf::BlendMode::Factor::SrcColor,
                                sf::BlendMode::Equation::ReverseSubtract,
                                sf::BlendMode::Factor::OneMinusDstAlpha,
                                sf::BlendMode::Factor::DstAlpha,
                                sf::BlendMode::Equation::Max) !=
                  sf::BlendMode(sf::BlendMode::Factor::One,
                                sf::BlendMode::Factor::SrcColor,
                                sf::BlendMode::Equation::ReverseSubtract,
                                sf::BlendMode::Factor::OneMinusDstAlpha,
                                sf::BlendMode::Factor::DstAlpha,
                                sf::BlendMode::Equation::Max));
        }
    }

    SECTION("Static constants")
    {
        CHECK(sf::BlendMode{}.colorSrcFactor == sf::BlendMode::Factor::SrcAlpha);
        CHECK(sf::BlendMode{}.colorDstFactor == sf::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(sf::BlendMode{}.colorEquation == sf::BlendMode::Equation::Add);
        CHECK(sf::BlendMode{}.alphaSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMode{}.alphaDstFactor == sf::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(sf::BlendMode{}.alphaEquation == sf::BlendMode::Equation::Add);

        CHECK(sf::BlendAlpha.colorSrcFactor == sf::BlendMode::Factor::SrcAlpha);
        CHECK(sf::BlendAlpha.colorDstFactor == sf::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(sf::BlendAlpha.colorEquation == sf::BlendMode::Equation::Add);
        CHECK(sf::BlendAlpha.alphaSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendAlpha.alphaDstFactor == sf::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(sf::BlendAlpha.alphaEquation == sf::BlendMode::Equation::Add);

        CHECK(sf::BlendAdd.colorSrcFactor == sf::BlendMode::Factor::SrcAlpha);
        CHECK(sf::BlendAdd.colorDstFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendAdd.colorEquation == sf::BlendMode::Equation::Add);
        CHECK(sf::BlendAdd.alphaSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendAdd.alphaDstFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendAdd.alphaEquation == sf::BlendMode::Equation::Add);

        CHECK(sf::BlendMultiply.colorSrcFactor == sf::BlendMode::Factor::DstColor);
        CHECK(sf::BlendMultiply.colorDstFactor == sf::BlendMode::Factor::Zero);
        CHECK(sf::BlendMultiply.colorEquation == sf::BlendMode::Equation::Add);
        CHECK(sf::BlendMultiply.alphaSrcFactor == sf::BlendMode::Factor::DstColor);
        CHECK(sf::BlendMultiply.alphaDstFactor == sf::BlendMode::Factor::Zero);
        CHECK(sf::BlendMultiply.alphaEquation == sf::BlendMode::Equation::Add);

        CHECK(sf::BlendMin.colorSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMin.colorDstFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMin.colorEquation == sf::BlendMode::Equation::Min);
        CHECK(sf::BlendMin.alphaSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMin.alphaDstFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMin.alphaEquation == sf::BlendMode::Equation::Min);

        CHECK(sf::BlendMax.colorSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMax.colorDstFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMax.colorEquation == sf::BlendMode::Equation::Max);
        CHECK(sf::BlendMax.alphaSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMax.alphaDstFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendMax.alphaEquation == sf::BlendMode::Equation::Max);

        CHECK(sf::BlendNone.colorSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendNone.colorDstFactor == sf::BlendMode::Factor::Zero);
        CHECK(sf::BlendNone.colorEquation == sf::BlendMode::Equation::Add);
        CHECK(sf::BlendNone.alphaSrcFactor == sf::BlendMode::Factor::One);
        CHECK(sf::BlendNone.alphaDstFactor == sf::BlendMode::Factor::Zero);
        CHECK(sf::BlendNone.alphaEquation == sf::BlendMode::Equation::Add);
    }
}
