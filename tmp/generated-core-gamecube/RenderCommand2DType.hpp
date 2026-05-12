#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_enum.hpp"
#include "IComparable.hpp"
#include "IConvertible.hpp"
#include "ISpanFormattable.hpp"
#include "IFormattable.hpp"

enum class RenderCommand2DType
{
    ClipPush = 1,
    ClipPop = 2,
    TexturedQuad = 3,
    GlyphQuad = 4,
    RoundedRect = 5
};
