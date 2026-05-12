#ifdef DrawText
#undef DrawText
#endif
#include "ScrollComponent.hpp"
#include "float4.hpp"
#include "Core.hpp"
#include "InputSystem.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/math.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

ScrollComponent::ScrollComponent() : ScrollOffset(0), ScrollOffsetChanged(), ContentRootValue(), ItemCountValue(0), ItemExtentValue(1), RequiresPointerInsideValue(true), ScrollStepCountValue(1), SizeValue(), VisibleItemCountValue(0), WheelNotchSizeValue(StandardWheelNotch)
{
}

::Entity* ScrollComponent::get_ContentRoot()
{
return this->ContentRootValue;}

void ScrollComponent::set_ContentRoot(::Entity* value)
{
this->ContentRootValue = value;
this->ApplyContentRootOffset();
}

int32_t ScrollComponent::get_ItemCount()
{
return this->ItemCountValue;}

void ScrollComponent::set_ItemCount(int32_t value)
{
    if (value < 0)
    {
throw ([&]() {
auto __ctor_arg_0000029C = "value";
auto __ctor_arg_0000029D = "Item count must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_0000029C, __ctor_arg_0000029D);
})();
    }
this->ItemCountValue = value;
this->ClampScrollOffset();
}

int32_t ScrollComponent::get_ItemExtent()
{
return this->ItemExtentValue;}

void ScrollComponent::set_ItemExtent(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_0000029E = "value";
auto __ctor_arg_0000029F = "Item extent must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_0000029E, __ctor_arg_0000029F);
})();
    }
this->ItemExtentValue = value;
this->ClampScrollOffset();
this->ApplyContentRootOffset();
}

int32_t ScrollComponent::get_MaximumScrollOffset()
{
return Math::Max(0, this->ItemCountValue - this->GetVisibleItemCount());}

bool ScrollComponent::get_RequiresPointerInside()
{
return this->RequiresPointerInsideValue;}

void ScrollComponent::set_RequiresPointerInside(bool value)
{
this->RequiresPointerInsideValue = value;
}

int32_t ScrollComponent::get_ScrollOffset()
{
return this->ScrollOffset;
}

void ScrollComponent::set_ScrollOffset(int32_t value)
{
this->ScrollOffset = value;
}

int32_t ScrollComponent::get_ScrollStepCount()
{
return this->ScrollStepCountValue;}

void ScrollComponent::set_ScrollStepCount(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_000002A0 = "value";
auto __ctor_arg_000002A1 = "Scroll step count must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_000002A0, __ctor_arg_000002A1);
})();
    }
this->ScrollStepCountValue = value;
}

::int2 ScrollComponent::get_Size()
{
return this->SizeValue;}

void ScrollComponent::set_Size(::int2 value)
{
    if (value.X < 0 || value.Y < 0)
    {
throw ([&]() {
auto __ctor_arg_000002A2 = "value";
auto __ctor_arg_000002A3 = "Scroll viewport size must not be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000002A2, __ctor_arg_000002A3);
})();
    }
this->SizeValue = value;
}

bool ScrollComponent::get_UsesAutomaticVisibleItemCount()
{
return this->VisibleItemCountValue < 1;}

int32_t ScrollComponent::get_VisibleItemCount()
{
return this->GetVisibleItemCount();}

void ScrollComponent::set_VisibleItemCount(int32_t value)
{
    if (value < 0)
    {
throw ([&]() {
auto __ctor_arg_000002A4 = "value";
auto __ctor_arg_000002A5 = "Visible item count must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000002A4, __ctor_arg_000002A5);
})();
    }
this->VisibleItemCountValue = value;
this->ClampScrollOffset();
}

int32_t ScrollComponent::get_WheelNotchSize()
{
return this->WheelNotchSizeValue;}

void ScrollComponent::set_WheelNotchSize(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_000002A6 = "value";
auto __ctor_arg_000002A7 = "Wheel notch size must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_000002A6, __ctor_arg_000002A7);
})();
    }
this->WheelNotchSizeValue = value;
}

void ScrollComponent::ClampScrollOffset()
{
this->SetScrollOffset(this->ScrollOffset, false);
}

bool ScrollComponent::ContainsScreenPoint(int32_t x, int32_t y)
{
    if (Parent == nullptr)
    {
return false;    }
::float4 viewportRect = this->GetClipRect();
return x >= viewportRect.X && x < viewportRect.X + viewportRect.Z && y >= viewportRect.Y && y < viewportRect.Y + viewportRect.W;}

::float4 ScrollComponent::GetClipRect()
{
    if (Parent == nullptr)
    {
throw new InvalidOperationException("Scroll components require an attached parent entity.");
    }
::float3 origin = Parent->get_Position();
return ::float4(origin.X, origin.Y, this->SizeValue.X, this->SizeValue.Y);}

void ScrollComponent::ResetScrollOffset()
{
this->SetScrollOffset(0, false);
}

bool ScrollComponent::ScrollTo(int32_t scrollOffset)
{
return this->SetScrollOffset(scrollOffset, true);}

bool ScrollComponent::TryApplyWheelInput()
{
    if (Parent == nullptr)
    {
return false;    }
    if (this->get_MaximumScrollOffset() <= 0)
    {
return false;    }
    if (this->RequiresPointerInsideValue && !this->ContainsScreenPoint(Core::get_Instance()->get_Input()->GetMouseX(), Core::get_Instance()->get_Input()->GetMouseY()))
    {
return false;    }
const int32_t wheelDelta = Core::get_Instance()->get_Input()->GetMouseScrollWheelDelta();
    if (wheelDelta == 0)
    {
return false;    }
int32_t scrollSteps = wheelDelta / this->WheelNotchSizeValue;
    if (scrollSteps == 0)
    {
scrollSteps = wheelDelta > 0 ? 1 : -1;
    }
scrollSteps *= this->ScrollStepCountValue;
const int32_t nextOffset = this->ScrollOffset - scrollSteps;
return this->SetScrollOffset(nextOffset, true);}

void ScrollComponent::Update()
{
this->TryApplyWheelInput();
}

::Entity* ScrollComponent::get_Parent()
{
return this->Component::get_Parent();
}

uint8_t ScrollComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void ScrollComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

void ScrollComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

int32_t ScrollComponent::StandardWheelNotch = 120;

void ScrollComponent::ApplyContentRootOffset()
{
    if (this->ContentRootValue == nullptr)
    {
return;    }
::float3 position = this->ContentRootValue->get_LocalPosition();
this->ContentRootValue->set_LocalPosition(::float3(position.X, -(this->ScrollOffset * this->ItemExtentValue), position.Z));
}

int32_t ScrollComponent::ClampOffset(int32_t scrollOffset)
{
const int32_t maxOffset = this->get_MaximumScrollOffset();
    if (scrollOffset < 0)
    {
return 0;    }
    if (scrollOffset > maxOffset)
    {
return maxOffset;    }
return scrollOffset;}

int32_t ScrollComponent::GetVisibleItemCount()
{
    if (this->VisibleItemCountValue > 0)
    {
return this->VisibleItemCountValue;    }
const int32_t extent = this->ItemExtentValue;
    if (extent <= 0)
    {
return 1;    }
return Math::Max(1, (this->SizeValue.Y + extent - 1) / extent);}

bool ScrollComponent::SetScrollOffset(int32_t scrollOffset, bool raiseEvent)
{
const int32_t clampedOffset = this->ClampOffset(scrollOffset);
    if (clampedOffset == this->ScrollOffset)
    {
return false;    }
this->set_ScrollOffset(clampedOffset);
    if (raiseEvent && true)
    {
this->ScrollOffsetChanged.Invoke(this, ScrollOffset);
    }
this->ApplyContentRootOffset();
return true;}

