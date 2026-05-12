#ifdef DrawText
#undef DrawText
#endif
#include "FontChar.hpp"

FontChar::FontChar() : AdvanceWidth(), BearingX(), BearingY(), OffsetY(), SourceRect()
{
}

FontChar::FontChar(::float4 sourceRect, float offsetY, float advanceWidth, float bearingX, float bearingY) : AdvanceWidth(), BearingX(), BearingY(), OffsetY(), SourceRect()
{
this->SourceRect = sourceRect;
this->OffsetY = offsetY;
this->AdvanceWidth = advanceWidth;
this->BearingX = bearingX;
this->BearingY = bearingY;
}

