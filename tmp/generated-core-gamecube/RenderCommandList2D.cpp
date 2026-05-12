#ifdef DrawText
#undef DrawText
#endif
#include "RenderCommandList2D.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderCommand2DType.hpp"
#include "float4.hpp"
#include "RuntimeTexture.hpp"
#include "byte4.hpp"
#include "RoundedRectCorners.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

int32_t RenderCommandList2D::get_Count()
{
return this->CommandTypes->Count();}

void RenderCommandList2D::AddClipPop()
{
this->CommandTypes->Add(RenderCommand2DType::ClipPop);
this->PayloadIndices->Add(-1);
}

void RenderCommandList2D::AddClipPush(::float4 clipRect)
{
const int32_t payloadIndex = this->ClipRects->Count();
this->ClipRects->Add(clipRect);
this->CommandTypes->Add(RenderCommand2DType::ClipPush);
this->PayloadIndices->Add(payloadIndex);
}

void RenderCommandList2D::AddGlyphQuad(::RuntimeTexture* texture, ::float4 bounds, ::float4 sourceRect, ::byte4 color)
{
    if (texture == nullptr)
    {
throw new ArgumentNullException("texture");
    }
const int32_t payloadIndex = this->GlyphTextures->Count();
this->GlyphTextures->Add(texture);
this->GlyphBounds->Add(bounds);
this->GlyphSourceRects->Add(sourceRect);
this->GlyphColors->Add(color);
this->CommandTypes->Add(RenderCommand2DType::GlyphQuad);
this->PayloadIndices->Add(payloadIndex);
}

void RenderCommandList2D::AddRoundedRect(::float4 bounds, float radius, float borderThickness, ::RoundedRectCorners corners, ::byte4 fillColor, ::byte4 borderColor)
{
const int32_t payloadIndex = this->RoundedRectBounds->Count();
this->RoundedRectBounds->Add(bounds);
this->RoundedRectRadii->Add(radius);
this->RoundedRectBorderThicknesses->Add(borderThickness);
this->RoundedRectCornersValues->Add(corners);
this->RoundedRectFillColors->Add(fillColor);
this->RoundedRectBorderColors->Add(borderColor);
this->CommandTypes->Add(RenderCommand2DType::RoundedRect);
this->PayloadIndices->Add(payloadIndex);
}

void RenderCommandList2D::AddTexturedQuad(::RuntimeTexture* texture, ::float4 bounds, ::float4 sourceRect, ::byte4 color)
{
    if (texture == nullptr)
    {
throw new ArgumentNullException("texture");
    }
const int32_t payloadIndex = this->QuadTextures->Count();
this->QuadTextures->Add(texture);
this->QuadBounds->Add(bounds);
this->QuadSourceRects->Add(sourceRect);
this->QuadColors->Add(color);
this->CommandTypes->Add(RenderCommand2DType::TexturedQuad);
this->PayloadIndices->Add(payloadIndex);
}

int32_t RenderCommandList2D::GetClipPushPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices)[commandIndex];}

::float4 RenderCommandList2D::GetClipPushRect(int32_t payloadIndex)
{
return (*this->ClipRects)[payloadIndex];}

::RenderCommand2DType RenderCommandList2D::GetCommandType(int32_t commandIndex)
{
return (*this->CommandTypes)[commandIndex];}

::float4 RenderCommandList2D::GetGlyphQuadBounds(int32_t payloadIndex)
{
return (*this->GlyphBounds)[payloadIndex];}

::byte4 RenderCommandList2D::GetGlyphQuadColor(int32_t payloadIndex)
{
return (*this->GlyphColors)[payloadIndex];}

int32_t RenderCommandList2D::GetGlyphQuadPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices)[commandIndex];}

::float4 RenderCommandList2D::GetGlyphQuadSourceRect(int32_t payloadIndex)
{
return (*this->GlyphSourceRects)[payloadIndex];}

::RuntimeTexture* RenderCommandList2D::GetGlyphQuadTexture(int32_t payloadIndex)
{
return (*this->GlyphTextures)[payloadIndex];}

::byte4 RenderCommandList2D::GetRoundedRectBorderColor(int32_t payloadIndex)
{
return (*this->RoundedRectBorderColors)[payloadIndex];}

float RenderCommandList2D::GetRoundedRectBorderThickness(int32_t payloadIndex)
{
return (*this->RoundedRectBorderThicknesses)[payloadIndex];}

::float4 RenderCommandList2D::GetRoundedRectBounds(int32_t payloadIndex)
{
return (*this->RoundedRectBounds)[payloadIndex];}

::RoundedRectCorners RenderCommandList2D::GetRoundedRectCorners(int32_t payloadIndex)
{
return (*this->RoundedRectCornersValues)[payloadIndex];}

::byte4 RenderCommandList2D::GetRoundedRectFillColor(int32_t payloadIndex)
{
return (*this->RoundedRectFillColors)[payloadIndex];}

int32_t RenderCommandList2D::GetRoundedRectPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices)[commandIndex];}

float RenderCommandList2D::GetRoundedRectRadius(int32_t payloadIndex)
{
return (*this->RoundedRectRadii)[payloadIndex];}

::float4 RenderCommandList2D::GetTexturedQuadBounds(int32_t payloadIndex)
{
return (*this->QuadBounds)[payloadIndex];}

::byte4 RenderCommandList2D::GetTexturedQuadColor(int32_t payloadIndex)
{
return (*this->QuadColors)[payloadIndex];}

int32_t RenderCommandList2D::GetTexturedQuadPayloadIndex(int32_t commandIndex)
{
return (*this->PayloadIndices)[commandIndex];}

::float4 RenderCommandList2D::GetTexturedQuadSourceRect(int32_t payloadIndex)
{
return (*this->QuadSourceRects)[payloadIndex];}

::RuntimeTexture* RenderCommandList2D::GetTexturedQuadTexture(int32_t payloadIndex)
{
return (*this->QuadTextures)[payloadIndex];}

RenderCommandList2D::RenderCommandList2D(int32_t initialCapacity) : ClipRects(), CommandTypes(), GlyphBounds(), GlyphColors(), GlyphSourceRects(), GlyphTextures(), PayloadIndices(), QuadBounds(), QuadColors(), QuadSourceRects(), QuadTextures(), RoundedRectBorderColors(), RoundedRectBorderThicknesses(), RoundedRectBounds(), RoundedRectCornersValues(), RoundedRectFillColors(), RoundedRectRadii()
{
    if (initialCapacity < 0)
    {
throw new ArgumentOutOfRangeException("initialCapacity");
    }
this->CommandTypes = new List<::RenderCommand2DType>(initialCapacity);
this->PayloadIndices = new List<int32_t>(initialCapacity);
this->ClipRects = new List<::float4>(initialCapacity);
this->QuadTextures = new List<::RuntimeTexture*>(initialCapacity);
this->QuadBounds = new List<::float4>(initialCapacity);
this->QuadSourceRects = new List<::float4>(initialCapacity);
this->QuadColors = new List<::byte4>(initialCapacity);
this->GlyphTextures = new List<::RuntimeTexture*>(initialCapacity);
this->GlyphBounds = new List<::float4>(initialCapacity);
this->GlyphSourceRects = new List<::float4>(initialCapacity);
this->GlyphColors = new List<::byte4>(initialCapacity);
this->RoundedRectBounds = new List<::float4>(initialCapacity);
this->RoundedRectRadii = new List<float>(initialCapacity);
this->RoundedRectBorderThicknesses = new List<float>(initialCapacity);
this->RoundedRectCornersValues = new List<::RoundedRectCorners>(initialCapacity);
this->RoundedRectFillColors = new List<::byte4>(initialCapacity);
this->RoundedRectBorderColors = new List<::byte4>(initialCapacity);
}

void RenderCommandList2D::Reset()
{
this->CommandTypes->Clear();
this->PayloadIndices->Clear();
this->ClipRects->Clear();
this->QuadTextures->Clear();
this->QuadBounds->Clear();
this->QuadSourceRects->Clear();
this->QuadColors->Clear();
this->GlyphTextures->Clear();
this->GlyphBounds->Clear();
this->GlyphSourceRects->Clear();
this->GlyphColors->Clear();
this->RoundedRectBounds->Clear();
this->RoundedRectRadii->Clear();
this->RoundedRectBorderThicknesses->Clear();
this->RoundedRectCornersValues->Clear();
this->RoundedRectFillColors->Clear();
this->RoundedRectBorderColors->Clear();
}

