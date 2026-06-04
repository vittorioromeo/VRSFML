#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/GLUtils/Glad.hpp"
#include "Zancle/Graphics/BlendMode.hpp"
#include "Zancle/Graphics/GlDataType.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/StencilMode.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/GetArraySize.hpp"


////////////////////////////////////////////////////////////
#define ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(fnName, sfEnumType, ...)                                          \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::const]] constexpr GLenum fnName(const sfEnumType sfEnumValue) \
    {                                                                                                                 \
        constexpr GLenum glValues[] __VA_ARGS__;                                                                      \
                                                                                                                      \
        ZB_ASSERT(static_cast<unsigned int>(sfEnumValue) < ::zb::getArraySize(glValues));                             \
        return glValues[static_cast<unsigned int>(sfEnumValue)];                                                      \
    }


namespace za::priv
{
////////////////////////////////////////////////////////////
// Convert an za::BlendMode::Factor constant to the corresponding OpenGL constant.
ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    factorToGlConstant,
    za::BlendMode::Factor,
    {GL_ZERO,
     GL_ONE,
     GL_SRC_COLOR,
     GL_ONE_MINUS_SRC_COLOR,
     GL_DST_COLOR,
     GL_ONE_MINUS_DST_COLOR,
     GL_SRC_ALPHA,
     GL_ONE_MINUS_SRC_ALPHA,
     GL_DST_ALPHA,
     GL_ONE_MINUS_DST_ALPHA});


////////////////////////////////////////////////////////////
// Convert an za::BlendMode::Equation constant to the corresponding OpenGL constant.
ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(equationToGlConstant,
                                            za::BlendMode::Equation,
                                            {GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX});


////////////////////////////////////////////////////////////
// Convert an UpdateOperation constant to the corresponding OpenGL constant.
ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(stencilOperationToGlConstant,
                                            za::StencilUpdateOperation,
                                            {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_DECR, GL_INVERT});


////////////////////////////////////////////////////////////
// Convert a Comparison constant to the corresponding OpenGL constant.
ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    stencilFunctionToGlConstant,
    za::StencilComparison,
    {GL_NEVER, GL_LESS, GL_LEQUAL, GL_GREATER, GL_GEQUAL, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS});


////////////////////////////////////////////////////////////
ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    primitiveTypeToOpenGLMode,
    za::PrimitiveType,
    {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN});


////////////////////////////////////////////////////////////
ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN(
    dataTypeToOpenGLDataType,
    za::GlDataType,
    {GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_INT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE});

} // namespace za::priv


////////////////////////////////////////////////////////////
#undef ZA_PRIV_DEFINE_ENUM_TO_GLENUM_CONVERSION_FN
