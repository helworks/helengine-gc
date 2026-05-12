#ifdef DrawText
#undef DrawText
#endif
#include "AnchorComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "AnchorSpace.hpp"
#include "int2.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "float4.hpp"
#include "runtime/native_nullable.hpp"
#include "Component.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "Core.hpp"
#include "float2.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_nullable.hpp"
#include "runtime/native_string.hpp"

AnchorComponent::AnchorComponent() : AnchorDistances(), AnchorFlags(), IsSubscribedToWindowResize(), anchorBoundsProvider()
{
}

uint8_t AnchorComponent::BottomAnchorFlag = 8;

uint8_t AnchorComponent::LeftAnchorFlag = 1;

uint8_t AnchorComponent::RightAnchorFlag = 2;

uint8_t AnchorComponent::TopAnchorFlag = 4;

::float4 AnchorComponent::get_AnchorDistances()
{
return this->AnchorDistances;
}

void AnchorComponent::set_AnchorDistances(::float4 value)
{
this->AnchorDistances = value;
}

uint8_t AnchorComponent::get_AnchorFlags()
{
return this->AnchorFlags;
}

void AnchorComponent::set_AnchorFlags(uint8_t value)
{
this->AnchorFlags = value;
}

bool AnchorComponent::get_IsAnchored()
{
return this->AnchorFlags != 0;
}

void AnchorComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (this->get_IsAnchored())
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
}

void AnchorComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->DisableAnchoring();
}

void AnchorComponent::DisableAnchoring()
{
this->DetachFromBoundsProvider();
this->DetachFromWindowResize();
this->set_AnchorFlags(0);
this->set_AnchorDistances(::float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void AnchorComponent::EnableAnchoring(bool left, bool right, bool top, bool bottom)
{
    if (!left && !right && !top && !bottom)
    {
this->DisableAnchoring();
return;    }
    if (Parent == nullptr)
    {
throw new InvalidOperationException("AnchorComponent must be attached before anchoring can be enabled.");
    }
this->RefreshSubscriptions();
::AnchorSpace *anchorSpace = this->GetAnchorSpace();
::int2 anchoredSize = this->GetAnchorSize();
::float3 localPosition = Parent->get_LocalPosition();
uint8_t anchorFlags = 0;
::float4 anchorDistances = ::float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (left)
    {
anchorFlags |= LeftAnchorFlag;
anchorDistances.X = localPosition.X - anchorSpace->get_Origin().X;
    }
    if (right)
    {
anchorFlags |= RightAnchorFlag;
anchorDistances.Y = anchorSpace->get_Size().X - (localPosition.X - anchorSpace->get_Origin().X) - anchoredSize.X;
    }
    if (top)
    {
anchorFlags |= TopAnchorFlag;
anchorDistances.Z = localPosition.Y - anchorSpace->get_Origin().Y;
    }
    if (bottom)
    {
anchorFlags |= BottomAnchorFlag;
anchorDistances.W = anchorSpace->get_Size().Y - (localPosition.Y - anchorSpace->get_Origin().Y) - anchoredSize.Y;
    }
this->set_AnchorFlags(anchorFlags);
this->set_AnchorDistances(anchorDistances);
    if (Parent != nullptr && Core::get_Instance() != nullptr && Core::get_Instance()->get_RenderManager3D() != nullptr)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
}

std::string AnchorComponent::GetAnchorInfo()
{
    if (!this->get_IsAnchored())
    {
return "Not anchored";    }
std::string info = "Anchored to: ";
List<std::string> *anchors = new List<std::string>();
    if ((this->AnchorFlags & LeftAnchorFlag) != 0)
    {
anchors->Add(std::string("Left (") + std::to_string(this->AnchorDistances.X) + std::string("px)"));
    }
    if ((this->AnchorFlags & RightAnchorFlag) != 0)
    {
anchors->Add(std::string("Right (") + std::to_string(this->AnchorDistances.Y) + std::string("px)"));
    }
    if ((this->AnchorFlags & TopAnchorFlag) != 0)
    {
anchors->Add(std::string("Top (") + std::to_string(this->AnchorDistances.Z) + std::string("px)"));
    }
    if ((this->AnchorFlags & BottomAnchorFlag) != 0)
    {
anchors->Add(std::string("Bottom (") + std::to_string(this->AnchorDistances.W) + std::string("px)"));
    }
return String::Concat(info, String::JoinArray(", ", anchors->ToArray()));}

void AnchorComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!this->get_IsAnchored())
    {
return;    }
    if (newEnabled)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
else {
this->DetachFromBoundsProvider();
this->DetachFromWindowResize();
}
}

void AnchorComponent::RefreshAnchoring()
{
    if (!this->get_IsAnchored() || Parent == nullptr)
    {
return;    }
this->RefreshSubscriptions();
::AnchorSpace *anchorSpace = this->GetAnchorSpace();
::int2 anchorSize = this->GetAnchorSize();
::float3 localPosition = Parent->get_LocalPosition();
    if ((this->AnchorFlags & LeftAnchorFlag) != 0)
    {
localPosition.X = anchorSpace->get_Origin().X + this->AnchorDistances.X;
    }
else     if ((this->AnchorFlags & RightAnchorFlag) != 0)
    {
localPosition.X = anchorSpace->get_Origin().X + anchorSpace->get_Size().X - this->AnchorDistances.Y - anchorSize.X;
    }
    if ((this->AnchorFlags & TopAnchorFlag) != 0)
    {
localPosition.Y = anchorSpace->get_Origin().Y + this->AnchorDistances.Z;
    }
else     if ((this->AnchorFlags & BottomAnchorFlag) != 0)
    {
localPosition.Y = anchorSpace->get_Origin().Y + anchorSpace->get_Size().Y - this->AnchorDistances.W - anchorSize.Y;
    }
Parent->set_LocalPosition(localPosition);
}

void AnchorComponent::SetAnchorDistances(Nullable<float> left, Nullable<float> right, Nullable<float> top, Nullable<float> bottom)
{
    if (!left.HasValue && !right.HasValue && !top.HasValue && !bottom.HasValue)
    {
this->DisableAnchoring();
return;    }
uint8_t anchorFlags = 0;
::float4 anchorDistances = ::float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (left.HasValue)
    {
anchorFlags |= LeftAnchorFlag;
anchorDistances.X = left.Value;
    }
    if (right.HasValue)
    {
anchorFlags |= RightAnchorFlag;
anchorDistances.Y = right.Value;
    }
    if (top.HasValue)
    {
anchorFlags |= TopAnchorFlag;
anchorDistances.Z = top.Value;
    }
    if (bottom.HasValue)
    {
anchorFlags |= BottomAnchorFlag;
anchorDistances.W = bottom.Value;
    }
this->set_AnchorFlags(anchorFlags);
this->set_AnchorDistances(anchorDistances);
    if (Parent != nullptr && Core::get_Instance() != nullptr && Core::get_Instance()->get_RenderManager3D() != nullptr)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
}

::Entity* AnchorComponent::get_Parent()
{
return this->Component::get_Parent();
}

void AnchorComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

void AnchorComponent::AttachToWindowResize()
{
    if (this->IsSubscribedToWindowResize)
    {
return;    }
    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_RenderManager3D() == nullptr)
    {
return;    }
Core::get_Instance()->get_RenderManager3D()->WindowResized += &AnchorComponent::HandleWindowResized;
this->IsSubscribedToWindowResize = true;
}

void AnchorComponent::DetachFromBoundsProvider()
{
    if (this->anchorBoundsProvider != nullptr)
    {
this->anchorBoundsProvider->AnchorBoundsChanged -= &AnchorComponent::HandleAnchorBoundsChanged;
this->anchorBoundsProvider = nullptr;
    }
}

void AnchorComponent::DetachFromWindowResize()
{
    if (!this->IsSubscribedToWindowResize)
    {
return;    }
Core::get_Instance()->get_RenderManager3D()->WindowResized -= &AnchorComponent::HandleWindowResized;
this->IsSubscribedToWindowResize = false;
}

int32_t AnchorComponent::GetAnchorArea(::int2 size)
{
    if (size.X < 0 || size.Y < 0)
    {
return -1;    }
return size.X * size.Y;}

::int2 AnchorComponent::GetAnchorSize()
{
    if (Parent == nullptr)
    {
return ::int2(0, 0);    }
::IAnchorSizeProvider *bestProvider = nullptr;
int32_t bestArea = -1;
    IAnchorSizeProvider* parentProvider = he_cpp_try_cast<IAnchorSizeProvider>(Parent);
    if (parentProvider != nullptr)
    {
bestProvider = parentProvider;
bestArea = this->GetAnchorArea(parentProvider->get_AnchorSize());
    }
for (int32_t i = 0; i < Parent->get_Components()->Count(); i++) {
    IAnchorSizeProvider* sizeProvider = he_cpp_try_cast<IAnchorSizeProvider>((*Parent->get_Components())[i]);
    if (sizeProvider != nullptr)
    {
const int32_t area = this->GetAnchorArea(sizeProvider->get_AnchorSize());
    if (area > bestArea)
    {
bestProvider = sizeProvider;
bestArea = area;
    }
    }
}
    if (bestProvider == nullptr)
    {
return ::int2(0, 0);    }
return bestProvider->get_AnchorSize();}

::AnchorSpace* AnchorComponent::GetAnchorSpace()
{
    if (this->anchorBoundsProvider != nullptr)
    {
return this->anchorBoundsProvider->get_AnchorSpace();    }
return ([&]() {
auto __ctor_arg_00000200 = Core::get_Instance()->get_RenderManager3D()->get_MainWindowSize();
auto __ctor_arg_00000201 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_00000200, __ctor_arg_00000201);
})();}

void AnchorComponent::HandleAnchorBoundsChanged()
{
this->RefreshAnchoring();
}

void AnchorComponent::HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
this->RefreshAnchoring();
}

void AnchorComponent::RefreshSubscriptions()
{
::IAnchorBoundsProvider *newProvider = this->ResolveAnchorBoundsProvider();
    if (!(this->anchorBoundsProvider == newProvider))
    {
this->DetachFromBoundsProvider();
this->anchorBoundsProvider = newProvider;
    if (this->anchorBoundsProvider != nullptr)
    {
this->anchorBoundsProvider->AnchorBoundsChanged += &AnchorComponent::HandleAnchorBoundsChanged;
    }
    }
    if (this->anchorBoundsProvider == nullptr)
    {
this->AttachToWindowResize();
    }
else {
this->DetachFromWindowResize();
}
}

::IAnchorBoundsProvider* AnchorComponent::ResolveAnchorBoundsProvider()
{
::Entity *current = Parent;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t i = current->get_Components()->Count() - 1; i >= 0; i--) {
    IAnchorBoundsProvider* componentProvider = he_cpp_try_cast<IAnchorBoundsProvider>((*current->get_Components())[i]);
    if (componentProvider != nullptr)
    {
return componentProvider;    }
}
    }
    IAnchorBoundsProvider* provider = he_cpp_try_cast<IAnchorBoundsProvider>(current);
    if (provider != nullptr)
    {
return provider;    }
current = current->get_Parent();
}
return nullptr;}

