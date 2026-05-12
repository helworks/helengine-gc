#ifdef DrawText
#undef DrawText
#endif
#include "ReferenceCanvasFitSnapshot.hpp"
#include "system/math.hpp"
#include "AnchorComponent.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "ScrollComponent.hpp"
#include "int2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::float4 ReferenceCanvasFitSnapshot::get_AnchorDistances()
{
return this->AnchorDistances;
}

::int2 ReferenceCanvasFitSnapshot::get_ClipRectSize()
{
return this->ClipRectSize;
}

::Entity* ReferenceCanvasFitSnapshot::get_Entity()
{
return this->Entity;
}

::int2 ReferenceCanvasFitSnapshot::get_InteractableSize()
{
return this->InteractableSize;
}

bool ReferenceCanvasFitSnapshot::get_IsRootEntity()
{
return this->IsRootEntity;
}

::float3 ReferenceCanvasFitSnapshot::get_LocalPosition()
{
return this->LocalPosition;
}

float ReferenceCanvasFitSnapshot::get_RoundedRectBorderThickness()
{
return this->RoundedRectBorderThickness;
}

float ReferenceCanvasFitSnapshot::get_RoundedRectRadius()
{
return this->RoundedRectRadius;
}

::int2 ReferenceCanvasFitSnapshot::get_RoundedRectSize()
{
return this->RoundedRectSize;
}

int32_t ReferenceCanvasFitSnapshot::get_ScrollItemExtent()
{
return this->ScrollItemExtent;
}

::int2 ReferenceCanvasFitSnapshot::get_ScrollSize()
{
return this->ScrollSize;
}

float ReferenceCanvasFitSnapshot::get_TextFontScale()
{
return this->TextFontScale;
}

::int2 ReferenceCanvasFitSnapshot::get_TextSize()
{
return this->TextSize;
}

::AnchorComponent* ReferenceCanvasFitSnapshot::get_TrackedAnchorComponent()
{
return this->TrackedAnchorComponent;
}

::ClipRectComponent* ReferenceCanvasFitSnapshot::get_TrackedClipRectComponent()
{
return this->TrackedClipRectComponent;
}

::InteractableComponent* ReferenceCanvasFitSnapshot::get_TrackedInteractableComponent()
{
return this->TrackedInteractableComponent;
}

::RoundedRectComponent* ReferenceCanvasFitSnapshot::get_TrackedRoundedRectComponent()
{
return this->TrackedRoundedRectComponent;
}

::ScrollComponent* ReferenceCanvasFitSnapshot::get_TrackedScrollComponent()
{
return this->TrackedScrollComponent;
}

::TextComponent* ReferenceCanvasFitSnapshot::get_TrackedTextComponent()
{
return this->TrackedTextComponent;
}

void ReferenceCanvasFitSnapshot::Apply(::AnchorSpace* anchorSpace, ::float2 canvasOrigin, int32_t referenceWidth, int32_t referenceHeight)
{
const double widthScale = referenceWidth > 0 ? static_cast<double>(anchorSpace->get_Size().X) / referenceWidth : 1.0;
const double heightScale = referenceHeight > 0 ? static_cast<double>(anchorSpace->get_Size().Y) / referenceHeight : 1.0;
const double scalarScale = Math::Min(widthScale, heightScale);
float positionX = this->ScaleFloat(this->LocalPosition.X, widthScale);
float positionY = this->ScaleFloat(this->LocalPosition.Y, heightScale);
    if (this->IsRootEntity)
    {
positionX += canvasOrigin.X;
positionY += canvasOrigin.Y;
    }
this->Entity->set_LocalPosition(::float3(positionX, positionY, this->LocalPosition.Z));
    if (this->TrackedAnchorComponent != nullptr)
    {
this->TrackedAnchorComponent->set_AnchorDistances(([&]() {
auto __ctor_arg_00000105 = this->ScaleFloat(this->AnchorDistances.X, widthScale);
auto __ctor_arg_00000106 = this->ScaleFloat(this->AnchorDistances.Y, widthScale);
auto __ctor_arg_00000107 = this->ScaleFloat(this->AnchorDistances.Z, heightScale);
auto __ctor_arg_00000108 = this->ScaleFloat(this->AnchorDistances.W, heightScale);
return ::float4(__ctor_arg_00000105, __ctor_arg_00000106, __ctor_arg_00000107, __ctor_arg_00000108);
})());
    }
    if (this->TrackedRoundedRectComponent != nullptr)
    {
this->TrackedRoundedRectComponent->set_Size(this->ScaleInt2(this->RoundedRectSize, widthScale, heightScale));
this->TrackedRoundedRectComponent->set_Radius(this->ScaleFloat(this->RoundedRectRadius, scalarScale));
this->TrackedRoundedRectComponent->set_BorderThickness(this->ScaleFloat(this->RoundedRectBorderThickness, scalarScale));
    }
    if (this->TrackedTextComponent != nullptr)
    {
this->TrackedTextComponent->set_Size(this->ScaleInt2(this->TextSize, widthScale, heightScale));
this->TrackedTextComponent->set_FontScale(this->ScaleFloat(this->TextFontScale, scalarScale));
    }
    if (this->TrackedClipRectComponent != nullptr)
    {
this->TrackedClipRectComponent->set_Size(this->ScaleInt2(this->ClipRectSize, widthScale, heightScale));
    }
    if (this->TrackedInteractableComponent != nullptr)
    {
this->TrackedInteractableComponent->set_Size(this->ScaleInt2(this->InteractableSize, widthScale, heightScale));
    }
    if (this->TrackedScrollComponent != nullptr)
    {
this->TrackedScrollComponent->set_Size(this->ScaleInt2(this->ScrollSize, widthScale, heightScale));
this->TrackedScrollComponent->set_ItemExtent(this->ScaleInt(this->ScrollItemExtent, scalarScale));
    }
}

ReferenceCanvasFitSnapshot::ReferenceCanvasFitSnapshot(::Entity* entity, bool isRootEntity) : AnchorDistances(), ClipRectSize(), Entity(), InteractableSize(), IsRootEntity(), LocalPosition(), RoundedRectBorderThickness(), RoundedRectRadius(), RoundedRectSize(), ScrollItemExtent(0), ScrollSize(), TextFontScale(), TextSize(), TrackedAnchorComponent(), TrackedClipRectComponent(), TrackedInteractableComponent(), TrackedRoundedRectComponent(), TrackedScrollComponent(), TrackedTextComponent()
{
this->Entity = (entity != nullptr ? entity : throw new ArgumentNullException("entity"));
this->IsRootEntity = isRootEntity;
this->LocalPosition = entity->get_LocalPosition();
this->TrackedAnchorComponent = this->FindAnchorComponent(entity);
    if (this->TrackedAnchorComponent != nullptr)
    {
this->AnchorDistances = this->TrackedAnchorComponent->get_AnchorDistances();
    }
this->TrackedRoundedRectComponent = this->FindRoundedRectComponent(entity);
    if (this->TrackedRoundedRectComponent != nullptr)
    {
this->RoundedRectSize = this->TrackedRoundedRectComponent->get_Size();
this->RoundedRectRadius = this->TrackedRoundedRectComponent->get_Radius();
this->RoundedRectBorderThickness = this->TrackedRoundedRectComponent->get_BorderThickness();
    }
this->TrackedTextComponent = this->FindTextComponent(entity);
    if (this->TrackedTextComponent != nullptr)
    {
this->TextSize = this->TrackedTextComponent->get_Size();
this->TextFontScale = this->TrackedTextComponent->get_FontScale();
    }
this->TrackedClipRectComponent = this->FindClipRectComponent(entity);
    if (this->TrackedClipRectComponent != nullptr)
    {
this->ClipRectSize = this->TrackedClipRectComponent->get_Size();
    }
this->TrackedInteractableComponent = this->FindInteractableComponent(entity);
    if (this->TrackedInteractableComponent != nullptr)
    {
this->InteractableSize = this->TrackedInteractableComponent->get_Size();
    }
this->TrackedScrollComponent = this->FindScrollComponent(entity);
    if (this->TrackedScrollComponent != nullptr)
    {
this->ScrollSize = this->TrackedScrollComponent->get_Size();
this->ScrollItemExtent = this->TrackedScrollComponent->get_ItemExtent();
    }
}

void ReferenceCanvasFitSnapshot::RefreshAnchoring()
{
    if (this->TrackedAnchorComponent != nullptr)
    {
this->TrackedAnchorComponent->RefreshAnchoring();
    }
}

::AnchorComponent* ReferenceCanvasFitSnapshot::FindAnchorComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    AnchorComponent* component = he_cpp_try_cast<AnchorComponent>((*entity->get_Components())[componentIndex]);
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::ClipRectComponent* ReferenceCanvasFitSnapshot::FindClipRectComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    ClipRectComponent* component = he_cpp_try_cast<ClipRectComponent>((*entity->get_Components())[componentIndex]);
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::InteractableComponent* ReferenceCanvasFitSnapshot::FindInteractableComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    InteractableComponent* component = he_cpp_try_cast<InteractableComponent>((*entity->get_Components())[componentIndex]);
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::RoundedRectComponent* ReferenceCanvasFitSnapshot::FindRoundedRectComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    RoundedRectComponent* component = he_cpp_try_cast<RoundedRectComponent>((*entity->get_Components())[componentIndex]);
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::ScrollComponent* ReferenceCanvasFitSnapshot::FindScrollComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    ScrollComponent* component = he_cpp_try_cast<ScrollComponent>((*entity->get_Components())[componentIndex]);
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::TextComponent* ReferenceCanvasFitSnapshot::FindTextComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->Count(); componentIndex++) {
    TextComponent* component = he_cpp_try_cast<TextComponent>((*entity->get_Components())[componentIndex]);
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

float ReferenceCanvasFitSnapshot::ScaleFloat(float value, double scale)
{
return static_cast<float>((value * scale));}

int32_t ReferenceCanvasFitSnapshot::ScaleInt(int32_t value, double scale)
{
    if (value == 0)
    {
return 0;    }
return Math::Max(1, static_cast<int32_t>(Math::Round(value * scale)));}

::int2 ReferenceCanvasFitSnapshot::ScaleInt2(::int2 value, double widthScale, double heightScale)
{
return ([&]() {
auto __ctor_arg_00000109 = ScaleInt(value.X, widthScale);
auto __ctor_arg_0000010A = ScaleInt(value.Y, heightScale);
return ::int2(__ctor_arg_00000109, __ctor_arg_0000010A);
})();}

