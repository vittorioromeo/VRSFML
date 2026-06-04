#include "GraphicsUtil.hpp"
#include "Tst/Tst.hpp"
#include "Zancle/Graphics/BlendMode.hpp"
#include "ZancleBase/Trait/IsAggregate.hpp"
#include "ZancleBase/Trait/IsStandardLayout.hpp"
#include "ZancleBase/Trait/IsTrivial.hpp"
#include "ZancleBase/Trait/IsTriviallyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"


TEST_CASE("[Graphics] za::BlendMode")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(za::BlendMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(za::BlendMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(za::BlendMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(za::BlendMode));

        STATIC_CHECK(!ZB_IS_TRIVIAL(za::BlendMode));
        STATIC_CHECK(ZB_IS_STANDARD_LAYOUT(za::BlendMode));
        STATIC_CHECK(ZB_IS_AGGREGATE(za::BlendMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_COPYABLE(za::BlendMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_DESTRUCTIBLE(za::BlendMode));
        STATIC_CHECK(ZB_IS_TRIVIALLY_ASSIGNABLE(za::BlendMode, za::BlendMode));

        STATIC_CHECK(sizeof(za::BlendMode) <= 4); // should be packed via bitfields
    }

    SECTION("Construction")
    {
        SECTION("Combined color and alpha constructor using default parameter")
        {
            const auto blendMode = za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::SrcColor);

            CHECK(blendMode.colorSrcFactor == za::BlendMode::Factor::Zero);
            CHECK(blendMode.colorDstFactor == za::BlendMode::Factor::SrcColor);
            CHECK(blendMode.colorEquation == za::BlendMode::Equation::Add);
            CHECK(blendMode.alphaSrcFactor == za::BlendMode::Factor::Zero);
            CHECK(blendMode.alphaDstFactor == za::BlendMode::Factor::SrcColor);
            CHECK(blendMode.alphaEquation == za::BlendMode::Equation::Add);
        }

        SECTION("Combined color and alpha constructor")
        {
            const auto blendMode = za::BlendMode::from(za::BlendMode::Factor::Zero,
                                                       za::BlendMode::Factor::SrcColor,
                                                       za::BlendMode::Equation::ReverseSubtract);

            CHECK(blendMode.colorSrcFactor == za::BlendMode::Factor::Zero);
            CHECK(blendMode.colorDstFactor == za::BlendMode::Factor::SrcColor);
            CHECK(blendMode.colorEquation == za::BlendMode::Equation::ReverseSubtract);
            CHECK(blendMode.alphaSrcFactor == za::BlendMode::Factor::Zero);
            CHECK(blendMode.alphaDstFactor == za::BlendMode::Factor::SrcColor);
            CHECK(blendMode.alphaEquation == za::BlendMode::Equation::ReverseSubtract);
        }

        SECTION("Separate color and alpha constructor")
        {
            const za::BlendMode blendMode(za::BlendMode::Factor::Zero,
                                          za::BlendMode::Factor::SrcColor,
                                          za::BlendMode::Equation::ReverseSubtract,
                                          za::BlendMode::Factor::OneMinusDstAlpha,
                                          za::BlendMode::Factor::DstAlpha,
                                          za::BlendMode::Equation::Max);
            CHECK(blendMode.colorSrcFactor == za::BlendMode::Factor::Zero);
            CHECK(blendMode.colorDstFactor == za::BlendMode::Factor::SrcColor);
            CHECK(blendMode.colorEquation == za::BlendMode::Equation::ReverseSubtract);
            CHECK(blendMode.alphaSrcFactor == za::BlendMode::Factor::OneMinusDstAlpha);
            CHECK(blendMode.alphaDstFactor == za::BlendMode::Factor::DstAlpha);
            CHECK(blendMode.alphaEquation == za::BlendMode::Equation::Max);
        }
    }

    SECTION("Operators")
    {
        SECTION("operator==")
        {
            CHECK(za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::One) ==
                  za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::One));
            CHECK(za::BlendMode(za::BlendMode::Factor::Zero,
                                za::BlendMode::Factor::SrcColor,
                                za::BlendMode::Equation::ReverseSubtract,
                                za::BlendMode::Factor::OneMinusDstAlpha,
                                za::BlendMode::Factor::DstAlpha,
                                za::BlendMode::Equation::Max) ==
                  za::BlendMode(za::BlendMode::Factor::Zero,
                                za::BlendMode::Factor::SrcColor,
                                za::BlendMode::Equation::ReverseSubtract,
                                za::BlendMode::Factor::OneMinusDstAlpha,
                                za::BlendMode::Factor::DstAlpha,
                                za::BlendMode::Equation::Max));

            CHECK_FALSE(za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::One) ==
                        za::BlendMode::from(za::BlendMode::Factor::One, za::BlendMode::Factor::Zero));
            CHECK_FALSE(
                za::BlendMode(za::BlendMode::Factor::Zero,
                              za::BlendMode::Factor::SrcColor,
                              za::BlendMode::Equation::ReverseSubtract,
                              za::BlendMode::Factor::OneMinusDstAlpha,
                              za::BlendMode::Factor::DstAlpha,
                              za::BlendMode::Equation::Max) ==
                za::BlendMode(za::BlendMode::Factor::One,
                              za::BlendMode::Factor::SrcColor,
                              za::BlendMode::Equation::ReverseSubtract,
                              za::BlendMode::Factor::OneMinusDstAlpha,
                              za::BlendMode::Factor::DstAlpha,
                              za::BlendMode::Equation::Max));
        }

        SECTION("operator!=")
        {
            CHECK_FALSE(za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::One) !=
                        za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::One));
            CHECK_FALSE(
                za::BlendMode(za::BlendMode::Factor::Zero,
                              za::BlendMode::Factor::SrcColor,
                              za::BlendMode::Equation::ReverseSubtract,
                              za::BlendMode::Factor::OneMinusDstAlpha,
                              za::BlendMode::Factor::DstAlpha,
                              za::BlendMode::Equation::Max) !=
                za::BlendMode(za::BlendMode::Factor::Zero,
                              za::BlendMode::Factor::SrcColor,
                              za::BlendMode::Equation::ReverseSubtract,
                              za::BlendMode::Factor::OneMinusDstAlpha,
                              za::BlendMode::Factor::DstAlpha,
                              za::BlendMode::Equation::Max));

            CHECK(za::BlendMode::from(za::BlendMode::Factor::Zero, za::BlendMode::Factor::One) !=
                  za::BlendMode::from(za::BlendMode::Factor::One, za::BlendMode::Factor::Zero));
            CHECK(za::BlendMode(za::BlendMode::Factor::Zero,
                                za::BlendMode::Factor::SrcColor,
                                za::BlendMode::Equation::ReverseSubtract,
                                za::BlendMode::Factor::OneMinusDstAlpha,
                                za::BlendMode::Factor::DstAlpha,
                                za::BlendMode::Equation::Max) !=
                  za::BlendMode(za::BlendMode::Factor::One,
                                za::BlendMode::Factor::SrcColor,
                                za::BlendMode::Equation::ReverseSubtract,
                                za::BlendMode::Factor::OneMinusDstAlpha,
                                za::BlendMode::Factor::DstAlpha,
                                za::BlendMode::Equation::Max));
        }
    }

    SECTION("Static constants")
    {
        CHECK(za::BlendMode{}.colorSrcFactor == za::BlendMode::Factor::SrcAlpha);
        CHECK(za::BlendMode{}.colorDstFactor == za::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(za::BlendMode{}.colorEquation == za::BlendMode::Equation::Add);
        CHECK(za::BlendMode{}.alphaSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMode{}.alphaDstFactor == za::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(za::BlendMode{}.alphaEquation == za::BlendMode::Equation::Add);

        CHECK(za::BlendAlpha.colorSrcFactor == za::BlendMode::Factor::SrcAlpha);
        CHECK(za::BlendAlpha.colorDstFactor == za::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(za::BlendAlpha.colorEquation == za::BlendMode::Equation::Add);
        CHECK(za::BlendAlpha.alphaSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendAlpha.alphaDstFactor == za::BlendMode::Factor::OneMinusSrcAlpha);
        CHECK(za::BlendAlpha.alphaEquation == za::BlendMode::Equation::Add);

        CHECK(za::BlendAdd.colorSrcFactor == za::BlendMode::Factor::SrcAlpha);
        CHECK(za::BlendAdd.colorDstFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendAdd.colorEquation == za::BlendMode::Equation::Add);
        CHECK(za::BlendAdd.alphaSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendAdd.alphaDstFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendAdd.alphaEquation == za::BlendMode::Equation::Add);

        CHECK(za::BlendMultiply.colorSrcFactor == za::BlendMode::Factor::DstColor);
        CHECK(za::BlendMultiply.colorDstFactor == za::BlendMode::Factor::Zero);
        CHECK(za::BlendMultiply.colorEquation == za::BlendMode::Equation::Add);
        CHECK(za::BlendMultiply.alphaSrcFactor == za::BlendMode::Factor::DstColor);
        CHECK(za::BlendMultiply.alphaDstFactor == za::BlendMode::Factor::Zero);
        CHECK(za::BlendMultiply.alphaEquation == za::BlendMode::Equation::Add);

        CHECK(za::BlendMin.colorSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMin.colorDstFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMin.colorEquation == za::BlendMode::Equation::Min);
        CHECK(za::BlendMin.alphaSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMin.alphaDstFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMin.alphaEquation == za::BlendMode::Equation::Min);

        CHECK(za::BlendMax.colorSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMax.colorDstFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMax.colorEquation == za::BlendMode::Equation::Max);
        CHECK(za::BlendMax.alphaSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMax.alphaDstFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendMax.alphaEquation == za::BlendMode::Equation::Max);

        CHECK(za::BlendNone.colorSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendNone.colorDstFactor == za::BlendMode::Factor::Zero);
        CHECK(za::BlendNone.colorEquation == za::BlendMode::Equation::Add);
        CHECK(za::BlendNone.alphaSrcFactor == za::BlendMode::Factor::One);
        CHECK(za::BlendNone.alphaDstFactor == za::BlendMode::Factor::Zero);
        CHECK(za::BlendNone.alphaEquation == za::BlendMode::Equation::Add);
    }
}
