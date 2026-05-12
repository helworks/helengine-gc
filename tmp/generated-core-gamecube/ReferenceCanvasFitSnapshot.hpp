#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class AnchorComponent;
class RoundedRectComponent;
class TextComponent;
class ClipRectComponent;
class InteractableComponent;
class ScrollComponent;
class int2;
class float4;
class Entity;
class float3;
class AnchorSpace;
class float2;

#include "system/math.hpp"
#include "system/math.hpp"
#include "AnchorComponent.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "ScrollComponent.hpp"
#include "int2.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "AnchorComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "RoundedRectComponent.hpp"
#include "ScrollComponent.hpp"
#include "TextComponent.hpp"
#include "AnchorSpace.hpp"
#include "float2.hpp"

class ReferenceCanvasFitSnapshot
{
public:
    virtual ~ReferenceCanvasFitSnapshot() = default;

    ::float4 AnchorDistances;

    ::float4 get_AnchorDistances();

    ::int2 ClipRectSize;

    ::int2 get_ClipRectSize();

    ::Entity* Entity;

    ::Entity* get_Entity();

    ::int2 InteractableSize;

    ::int2 get_InteractableSize();

    bool IsRootEntity;

    bool get_IsRootEntity();

    ::float3 LocalPosition;

    ::float3 get_LocalPosition();

    float RoundedRectBorderThickness;

    float get_RoundedRectBorderThickness();

    float RoundedRectRadius;

    float get_RoundedRectRadius();

    ::int2 RoundedRectSize;

    ::int2 get_RoundedRectSize();

    int32_t ScrollItemExtent;

    int32_t get_ScrollItemExtent();

    ::int2 ScrollSize;

    ::int2 get_ScrollSize();

    float TextFontScale;

    float get_TextFontScale();

    ::int2 TextSize;

    ::int2 get_TextSize();

    ::AnchorComponent* TrackedAnchorComponent;

    ::AnchorComponent* get_TrackedAnchorComponent();

    ::ClipRectComponent* TrackedClipRectComponent;

    ::ClipRectComponent* get_TrackedClipRectComponent();

    ::InteractableComponent* TrackedInteractableComponent;

    ::InteractableComponent* get_TrackedInteractableComponent();

    ::RoundedRectComponent* TrackedRoundedRectComponent;

    ::RoundedRectComponent* get_TrackedRoundedRectComponent();

    ::ScrollComponent* TrackedScrollComponent;

    ::ScrollComponent* get_TrackedScrollComponent();

    ::TextComponent* TrackedTextComponent;

    ::TextComponent* get_TrackedTextComponent();

    void Apply(::AnchorSpace* anchorSpace, ::float2 canvasOrigin, int32_t referenceWidth, int32_t referenceHeight);

    ReferenceCanvasFitSnapshot(::Entity* entity, bool isRootEntity);

    void RefreshAnchoring();
private:
    static ::AnchorComponent* FindAnchorComponent(::Entity* entity);

    static ::ClipRectComponent* FindClipRectComponent(::Entity* entity);

    static ::InteractableComponent* FindInteractableComponent(::Entity* entity);

    static ::RoundedRectComponent* FindRoundedRectComponent(::Entity* entity);

    static ::ScrollComponent* FindScrollComponent(::Entity* entity);

    static ::TextComponent* FindTextComponent(::Entity* entity);

    static float ScaleFloat(float value, double scale);

    static int32_t ScaleInt(int32_t value, double scale);

    static ::int2 ScaleInt2(::int2 value, double widthScale, double heightScale);
};
