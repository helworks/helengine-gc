#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRenderVisitor2D;
class RenderCommandList2D;
class IRenderQueue2D;
class ClipRegionStackBuilder2D;
class float4;
class IClipRegion2D;
class int2;
class ISpriteDrawable2D;
class float3;
class Entity;
class FontAsset;
class ITextDrawable2D;
class IRoundedRectDrawable2D;
class IDrawable2D;

#include "IRenderVisitor2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderCommandList2D.hpp"
#include "runtime/native_list.hpp"
#include "IRenderQueue2D.hpp"
#include "ClipRegionStackBuilder2D.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "float4.hpp"
#include "IClipRegion2D.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "ISpriteDrawable2D.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "FontAsset.hpp"
#include "ITextDrawable2D.hpp"
#include "runtime/native_string.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "runtime/native_list.hpp"
#include "IClipRegion2D.hpp"
#include "ClipRegionStackBuilder2D.hpp"
#include "RenderCommandList2D.hpp"
#include "IRenderQueue2D.hpp"
#include "IDrawable2D.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"

class RenderCommandListBuilder2D : public IRenderVisitor2D
{
public:
    virtual ~RenderCommandListBuilder2D() = default;

    ::RenderCommandList2D* Build(::IRenderQueue2D* renderQueue);

    RenderCommandListBuilder2D();

    void Visit(::IDrawable2D* drawable);
private:
    List<::IClipRegion2D*>* ActiveClipChain;

    ::ClipRegionStackBuilder2D* ClipRegionStackBuilder;

    ::RenderCommandList2D* CommandListValue;

    List<::IClipRegion2D*>* NextClipChain;

    void EmitRoundedRect(::IRoundedRectDrawable2D* roundedRect);

    void EmitSprite(::ISpriteDrawable2D* sprite);

    void EmitText(::ITextDrawable2D* text);

    void EmitTrailingClipPops();

    int32_t GetSharedPrefixLength();

    ::float4 ResolveClipRectForPush(::IClipRegion2D* clipRegion);

    void SyncClipTransitions();
};
