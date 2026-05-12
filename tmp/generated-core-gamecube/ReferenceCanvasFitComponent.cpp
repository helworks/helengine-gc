#ifdef DrawText
#undef DrawText
#endif
#include "ReferenceCanvasFitComponent.hpp"
#include "UpdateComponent.hpp"
#include "runtime/native_list.hpp"
#include "AnchorSpace.hpp"
#include "float2.hpp"
#include "int2.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "system/math.hpp"
#include "SceneCanvasProfile.hpp"
#include "ReferenceCanvasFitSnapshot.hpp"
#include "system/math.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::AnchorSpace* ReferenceCanvasFitComponent::get_AnchorSpace()
{
return this->CurrentAnchorSpaceValue;
}

int32_t ReferenceCanvasFitComponent::get_ReferenceHeight()
{
return this->ReferenceHeightValue;}

void ReferenceCanvasFitComponent::set_ReferenceHeight(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_0000028E = "value";
auto __ctor_arg_0000028F = "Reference canvas height must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_0000028E, __ctor_arg_0000028F);
})();
    }
this->ReferenceHeightValue = value;
this->ApplyCurrentScale();
}

int32_t ReferenceCanvasFitComponent::get_ReferenceWidth()
{
return this->ReferenceWidthValue;}

void ReferenceCanvasFitComponent::set_ReferenceWidth(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_00000290 = "value";
auto __ctor_arg_00000291 = "Reference canvas width must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_00000290, __ctor_arg_00000291);
})();
    }
this->ReferenceWidthValue = value;
this->ApplyCurrentScale();
}

void ReferenceCanvasFitComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
this->RebuildSnapshots();
this->AttachToWindowResize();
this->PendingApplyValue = true;
}

void ReferenceCanvasFitComponent::ComponentRemoved(::Entity* entity)
{
UpdateComponent::ComponentRemoved(entity);
this->DetachFromWindowResize();
this->SnapshotsValue->Clear();
this->SnapshotEntityCountValue = 0;
this->PendingApplyValue = false;
}

void ReferenceCanvasFitComponent::ParentEnabledChange(bool newEnabled)
{
UpdateComponent::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
this->AttachToWindowResize();
this->PendingApplyValue = true;
return;    }
this->DetachFromWindowResize();
}

ReferenceCanvasFitComponent::ReferenceCanvasFitComponent() : AnchorBoundsChanged(), CurrentAnchorSpaceValue(), CurrentCanvasOriginValue(), IsSubscribedToWindowResizeValue(), PendingApplyValue(), ReferenceHeightValue(0), ReferenceWidthValue(0), SnapshotEntityCountValue(0), SnapshotsValue()
{
this->ReferenceWidthValue = SceneCanvasProfile::DefaultWidth;
this->ReferenceHeightValue = SceneCanvasProfile::DefaultHeight;
this->SnapshotsValue = new List<::ReferenceCanvasFitSnapshot*>();
this->CurrentAnchorSpaceValue = ([&]() {
auto __ctor_arg_00000292 = ::int2(this->ReferenceWidthValue, this->ReferenceHeightValue);
auto __ctor_arg_00000293 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_00000292, __ctor_arg_00000293);
})();
this->CurrentCanvasOriginValue = ::float2(0.0f, 0.0f);
}

void ReferenceCanvasFitComponent::Update()
{
UpdateComponent::Update();
    if (Parent == nullptr)
    {
return;    }
const int32_t currentEntityCount = this->CountEntitiesRecursive(Parent);
    if (currentEntityCount != this->SnapshotEntityCountValue)
    {
this->RebuildSnapshots();
this->PendingApplyValue = true;
    }
    if (this->PendingApplyValue)
    {
this->ApplyCurrentScale();
this->PendingApplyValue = false;
    }
}

uint8_t ReferenceCanvasFitComponent::get_UpdateOrder()
{
return this->UpdateComponent::get_UpdateOrder();
}

void ReferenceCanvasFitComponent::set_UpdateOrder(uint8_t value)
{
this->UpdateComponent::set_UpdateOrder(value);
}

::Entity* ReferenceCanvasFitComponent::get_Parent()
{
return this->Component::get_Parent();
}

void ReferenceCanvasFitComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

void ReferenceCanvasFitComponent::ApplyCurrentScale()
{
    if (Parent == nullptr || Core::get_Instance() == nullptr || Core::get_Instance()->get_RenderManager3D() == nullptr || this->SnapshotsValue->Count() == 0)
    {
return;    }
::AnchorSpace *resolvedAnchorSpace = this->ResolveCurrentAnchorSpace();
::float2 resolvedCanvasOrigin = this->ResolveCurrentCanvasOrigin(resolvedAnchorSpace);
const bool anchorSpaceChanged = this->DidAnchorSpaceChange(this->CurrentAnchorSpaceValue, resolvedAnchorSpace) || this->DidCanvasOriginChange(this->CurrentCanvasOriginValue, resolvedCanvasOrigin);
this->CurrentAnchorSpaceValue = resolvedAnchorSpace;
this->CurrentCanvasOriginValue = resolvedCanvasOrigin;
for (int32_t snapshotIndex = 0; snapshotIndex < this->SnapshotsValue->Count(); snapshotIndex++) {
(*this->SnapshotsValue)[snapshotIndex]->Apply(resolvedAnchorSpace, resolvedCanvasOrigin, this->ReferenceWidthValue, this->ReferenceHeightValue);
}
for (int32_t snapshotIndex = 0; snapshotIndex < this->SnapshotsValue->Count(); snapshotIndex++) {
(*this->SnapshotsValue)[snapshotIndex]->RefreshAnchoring();
}
    if (anchorSpaceChanged)
    {
this->RaiseAnchorBoundsChanged();
    }
}

void ReferenceCanvasFitComponent::AttachToWindowResize()
{
    if (this->IsSubscribedToWindowResizeValue)
    {
return;    }
    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_RenderManager3D() == nullptr)
    {
return;    }
Core::get_Instance()->get_RenderManager3D()->WindowResized += &ReferenceCanvasFitComponent::HandleWindowResized;
this->IsSubscribedToWindowResizeValue = true;
}

void ReferenceCanvasFitComponent::CaptureSnapshotsRecursive(::Entity* entity, bool isRootEntity)
{
this->SnapshotsValue->Add(new ::ReferenceCanvasFitSnapshot(entity, isRootEntity));
    if (entity->get_Children() == nullptr)
    {
return;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->Count(); childIndex++) {
this->CaptureSnapshotsRecursive((*entity->get_Children())[childIndex], false);
}
}

int32_t ReferenceCanvasFitComponent::CountEntitiesRecursive(::Entity* entity)
{
    if (entity == nullptr)
    {
return 0;    }
int32_t count = 1;
    if (entity->get_Children() == nullptr)
    {
return count;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->Count(); childIndex++) {
count += this->CountEntitiesRecursive((*entity->get_Children())[childIndex]);
}
return count;}

void ReferenceCanvasFitComponent::DetachFromWindowResize()
{
    if (!this->IsSubscribedToWindowResizeValue)
    {
return;    }
    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_RenderManager3D() == nullptr)
    {
this->IsSubscribedToWindowResizeValue = false;
return;    }
Core::get_Instance()->get_RenderManager3D()->WindowResized -= &ReferenceCanvasFitComponent::HandleWindowResized;
this->IsSubscribedToWindowResizeValue = false;
}

bool ReferenceCanvasFitComponent::DidAnchorSpaceChange(::AnchorSpace* currentAnchorSpace, ::AnchorSpace* resolvedAnchorSpace)
{
    if (currentAnchorSpace == nullptr)
    {
return true;    }
return currentAnchorSpace->get_Size().X != resolvedAnchorSpace->get_Size().X || currentAnchorSpace->get_Size().Y != resolvedAnchorSpace->get_Size().Y || currentAnchorSpace->get_Origin().X != resolvedAnchorSpace->get_Origin().X || currentAnchorSpace->get_Origin().Y != resolvedAnchorSpace->get_Origin().Y;}

bool ReferenceCanvasFitComponent::DidCanvasOriginChange(::float2 currentCanvasOrigin, ::float2 resolvedCanvasOrigin)
{
return currentCanvasOrigin.X != resolvedCanvasOrigin.X || currentCanvasOrigin.Y != resolvedCanvasOrigin.Y;}

void ReferenceCanvasFitComponent::HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
this->PendingApplyValue = true;
}

bool ReferenceCanvasFitComponent::LiveWindowMatchesReferenceAspect(double liveWidth, double liveHeight)
{
const double expectedWidth = liveHeight * this->ReferenceWidthValue / this->ReferenceHeightValue;
const double expectedHeight = liveWidth * this->ReferenceHeightValue / this->ReferenceWidthValue;
return Math::Abs(liveWidth - expectedWidth) <= 0.5 || Math::Abs(liveHeight - expectedHeight) <= 0.5;}

void ReferenceCanvasFitComponent::RaiseAnchorBoundsChanged()
{
    if (true)
    {
this->AnchorBoundsChanged.Invoke();
    }
}

void ReferenceCanvasFitComponent::RebuildSnapshots()
{
this->SnapshotsValue->Clear();
    if (Parent == nullptr)
    {
this->SnapshotEntityCountValue = 0;
return;    }
this->CaptureSnapshotsRecursive(Parent, true);
this->SnapshotEntityCountValue = this->SnapshotsValue->Count();
}

::AnchorSpace* ReferenceCanvasFitComponent::ResolveCurrentAnchorSpace()
{
::int2 mainWindowSize = Core::get_Instance()->get_RenderManager3D()->get_MainWindowSize();
const double liveWidth = mainWindowSize.X > 0 ? mainWindowSize.X : ReferenceWidthValue;
const double liveHeight = mainWindowSize.Y > 0 ? mainWindowSize.Y : ReferenceHeightValue;
    if (this->LiveWindowMatchesReferenceAspect(liveWidth, liveHeight))
    {
return ([&]() {
auto __ctor_arg_00000294 = ([&]() {
auto __ctor_arg_00000295 = static_cast<int32_t>(Math::Round(liveWidth));
auto __ctor_arg_00000296 = static_cast<int32_t>(Math::Round(liveHeight));
return ::int2(__ctor_arg_00000295, __ctor_arg_00000296);
})();
auto __ctor_arg_00000297 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_00000294, __ctor_arg_00000297);
})();    }
const double widthScale = liveWidth / this->ReferenceWidthValue;
const double heightScale = liveHeight / this->ReferenceHeightValue;
const double scale = Math::Min(widthScale, heightScale);
    if (scale <= 0.0)
    {
return ([&]() {
auto __ctor_arg_00000298 = ::int2(this->ReferenceWidthValue, this->ReferenceHeightValue);
auto __ctor_arg_00000299 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_00000298, __ctor_arg_00000299);
})();    }
const int32_t fittedWidth = Math::Max(1, static_cast<int32_t>(Math::Round(this->ReferenceWidthValue * scale)));
const int32_t fittedHeight = Math::Max(1, static_cast<int32_t>(Math::Round(this->ReferenceHeightValue * scale)));
return ([&]() {
auto __ctor_arg_0000029A = ::int2(fittedWidth, fittedHeight);
auto __ctor_arg_0000029B = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_0000029A, __ctor_arg_0000029B);
})();}

::float2 ReferenceCanvasFitComponent::ResolveCurrentCanvasOrigin(::AnchorSpace* anchorSpace)
{
::int2 mainWindowSize = Core::get_Instance()->get_RenderManager3D()->get_MainWindowSize();
const double liveWidth = mainWindowSize.X > 0 ? mainWindowSize.X : ReferenceWidthValue;
const double liveHeight = mainWindowSize.Y > 0 ? mainWindowSize.Y : ReferenceHeightValue;
const float originX = static_cast<float>(((liveWidth - anchorSpace->get_Size().X) * 0.5));
const float originY = static_cast<float>(((liveHeight - anchorSpace->get_Size().Y) * 0.5));
return ::float2(originX, originY);}

