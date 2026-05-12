#ifdef DrawText
#undef DrawText
#endif
#include "RenderCommandListBuilder2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderCommandList2D.hpp"
#include "runtime/native_list.hpp"
#include "IRenderQueue2D.hpp"
#include "ClipRegionStackBuilder2D.hpp"
#include "system/math.hpp"
#include "float4.hpp"
#include "IClipRegion2D.hpp"
#include "int2.hpp"
#include "ISpriteDrawable2D.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "FontAsset.hpp"
#include "ITextDrawable2D.hpp"
#include "runtime/native_string.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "TextLayoutUtils.hpp"
#include "FontChar.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::RenderCommandList2D* RenderCommandListBuilder2D::Build(::IRenderQueue2D* renderQueue)
{
    if (renderQueue == nullptr)
    {
throw new ArgumentNullException("renderQueue");
    }
    if (this->CommandListValue == nullptr)
    {
this->CommandListValue = new ::RenderCommandList2D(Math::Max(renderQueue->get_Count(), 4));
    }
else {
this->CommandListValue->Reset();
}
this->ActiveClipChain->Clear();
this->NextClipChain->Clear();
renderQueue->VisitOrdered(this);
this->EmitTrailingClipPops();
return this->CommandListValue;}

RenderCommandListBuilder2D::RenderCommandListBuilder2D() : ActiveClipChain(), ClipRegionStackBuilder(), CommandListValue(), NextClipChain()
{
this->ClipRegionStackBuilder = new ::ClipRegionStackBuilder2D();
this->ActiveClipChain = new List<::IClipRegion2D*>();
this->NextClipChain = new List<::IClipRegion2D*>();
}

void RenderCommandListBuilder2D::Visit(::IDrawable2D* drawable)
{
    if (drawable == nullptr || drawable->get_Parent() == nullptr || !drawable->get_Parent()->get_Enabled())
    {
return;    }
this->ClipRegionStackBuilder->BuildClipChain(drawable, this->NextClipChain);
this->SyncClipTransitions();
    ISpriteDrawable2D* sprite = he_cpp_try_cast<ISpriteDrawable2D>(drawable);
    if (sprite != nullptr)
    {
this->EmitSprite(sprite);
return;    }
    ITextDrawable2D* text = he_cpp_try_cast<ITextDrawable2D>(drawable);
    if (text != nullptr)
    {
this->EmitText(text);
return;    }
    IRoundedRectDrawable2D* roundedRect = he_cpp_try_cast<IRoundedRectDrawable2D>(drawable);
    if (roundedRect != nullptr)
    {
this->EmitRoundedRect(roundedRect);
return;    }
throw new InvalidOperationException("Unsupported 2D drawable type.");
}

void RenderCommandListBuilder2D::EmitRoundedRect(::IRoundedRectDrawable2D* roundedRect)
{
::float3 position = roundedRect->get_Parent()->get_Position();
this->CommandListValue->AddRoundedRect(::float4(position.X, position.Y, roundedRect->get_Size().X, roundedRect->get_Size().Y), roundedRect->get_Radius(), roundedRect->get_BorderThickness(), roundedRect->get_Corners(), roundedRect->get_FillColor(), roundedRect->get_BorderColor());
}

void RenderCommandListBuilder2D::EmitSprite(::ISpriteDrawable2D* sprite)
{
    if (sprite->get_Texture() == nullptr)
    {
return;    }
::int2 size = sprite->get_Size();
const float width = size.X > 0 ? size.X : sprite->get_Texture()->get_Width();
const float height = size.Y > 0 ? size.Y : sprite->get_Texture()->get_Height();
::float3 position = sprite->get_Parent()->get_Position();
this->CommandListValue->AddTexturedQuad(sprite->get_Texture(), ::float4(position.X, position.Y, width, height), sprite->get_SourceRect(), sprite->get_Color());
}

void RenderCommandListBuilder2D::EmitText(::ITextDrawable2D* text)
{
::FontAsset *font = text->get_Font();
std::string content = text->get_Text();
const double fontScale = Math::Max(static_cast<double>(text->get_FontScale()), 0.0001);
    if (text->get_WrapText())
    {
content = TextLayoutUtils::WrapText(content, font, Math::Max(1, static_cast<int32_t>(Math::Round(text->get_Size().X / fontScale))));
    }
double offsetX = 0.0;
double offsetY = 0.0;
const double lineHeight = Math::Max(static_cast<double>(font->get_LineHeight()) * fontScale, 1.0);
const double baseX = Math::Round(text->get_Parent()->get_Position().X);
const double baseY = Math::Round(text->get_Parent()->get_Position().Y);
for (int32_t index = 0; index < static_cast<int32_t>(content.size()); index++) {
const char character = content[index];
    if (character == '\n')
    {
offsetY += lineHeight;
offsetX = 0.0;
continue;
    }
    if (character == ' ')
    {
offsetX += font->get_FontInfo()->get_SpaceWidth() * fontScale;
continue;
    }
::FontChar glyph;
    if (!font->get_Characters()->TryGetValue(character, glyph))
    {
continue;
    }
const double glyphWidth = glyph.SourceRect.Z * font->get_AtlasWidth() * fontScale;
const double glyphHeight = glyph.SourceRect.W * font->get_AtlasHeight() * fontScale;
const double snappedLineOffsetY = Math::Round(offsetY);
this->CommandListValue->AddGlyphQuad(font->get_Texture(), ::float4(static_cast<float>((baseX + offsetX)), static_cast<float>((baseY + snappedLineOffsetY + (glyph.OffsetY * fontScale))), static_cast<float>(glyphWidth), static_cast<float>(glyphHeight)), glyph.SourceRect, text->get_Color());
const double advanceWidth = glyph.AdvanceWidth > 0.0f ? glyph.AdvanceWidth * fontScale : glyphWidth;
offsetX += advanceWidth;
}
}

void RenderCommandListBuilder2D::EmitTrailingClipPops()
{
while (this->ActiveClipChain->Count() > 0) {
this->CommandListValue->AddClipPop();
this->ActiveClipChain->RemoveAt(this->ActiveClipChain->Count() - 1);
}
}

int32_t RenderCommandListBuilder2D::GetSharedPrefixLength()
{
int32_t sharedPrefixLength = 0;
const int32_t maxSharedLength = Math::Min(this->ActiveClipChain->Count(), this->NextClipChain->Count());
while (sharedPrefixLength < maxSharedLength && ((*this->ActiveClipChain)[sharedPrefixLength] == (*this->NextClipChain)[sharedPrefixLength])) {
sharedPrefixLength++;
}
return sharedPrefixLength;}

::float4 RenderCommandListBuilder2D::ResolveClipRectForPush(::IClipRegion2D* clipRegion)
{
::float4 resolvedRect = clipRegion->GetClipRect();
    if (this->ActiveClipChain->Count() <= 0)
    {
return resolvedRect;    }
::float4 currentRect = (*this->ActiveClipChain)[this->ActiveClipChain->Count() - 1]->GetClipRect();
return this->ClipRegionStackBuilder->Intersect(currentRect, resolvedRect);}

void RenderCommandListBuilder2D::SyncClipTransitions()
{
const int32_t sharedPrefixLength = this->GetSharedPrefixLength();
while (this->ActiveClipChain->Count() > sharedPrefixLength) {
this->CommandListValue->AddClipPop();
this->ActiveClipChain->RemoveAt(this->ActiveClipChain->Count() - 1);
}
while (this->ActiveClipChain->Count() < this->NextClipChain->Count()) {
::IClipRegion2D *clipRegion = (*this->NextClipChain)[this->ActiveClipChain->Count()];
::float4 resolvedRect = this->ResolveClipRectForPush(clipRegion);
this->CommandListValue->AddClipPush(resolvedRect);
this->ActiveClipChain->Add(clipRegion);
}
}

