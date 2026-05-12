#ifdef DrawText
#undef DrawText
#endif
#include "ViewportComponent.hpp"
#include "Component.hpp"
#include "CameraComponent.hpp"
#include "int2.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "float4.hpp"
#include "system/math.hpp"
#include "Entity.hpp"
#include "AnchorSpace.hpp"
#include "float2.hpp"
#include "SceneCanvasProfile.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_list.hpp"

uint8_t ViewportComponent::AncestorCameraBindingMode = 1;

uint8_t ViewportComponent::FixedBindingMode = 2;

uint8_t ViewportComponent::ScreenBindingMode = 0;

::AnchorSpace* ViewportComponent::get_AnchorSpace()
{
this->RefreshSubscriptions();
return ([&]() {
auto __ctor_arg_000002AC = this->ResolveAnchorBounds();
auto __ctor_arg_000002AD = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_000002AC, __ctor_arg_000002AD);
})();}

uint8_t ViewportComponent::get_BindingMode()
{
return this->BindingModeValue;}

void ViewportComponent::set_BindingMode(uint8_t value)
{
    if (this->BindingModeValue != value)
    {
this->BindingModeValue = value;
this->RefreshSubscriptions();
this->RaiseAnchorBoundsChanged();
    }
}

::int2 ViewportComponent::get_FixedSize()
{
return this->FixedSizeValue;}

void ViewportComponent::set_FixedSize(::int2 value)
{
    if (this->FixedSizeValue.X != value.X || this->FixedSizeValue.Y != value.Y)
    {
this->FixedSizeValue = value;
this->RaiseAnchorBoundsChanged();
    }
}

void ViewportComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
this->RefreshSubscriptions();
}

void ViewportComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->DetachFromCamera();
this->DetachFromWindowResize();
}

void ViewportComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
this->RefreshSubscriptions();
this->RaiseAnchorBoundsChanged();
    }
else {
this->DetachFromCamera();
this->DetachFromWindowResize();
}
}

ViewportComponent::ViewportComponent() : AnchorBoundsChanged(), BindingModeValue(), BoundCameraComponentValue(), FixedSizeValue(), IsSubscribedToWindowResizeValue()
{
this->BindingModeValue = ScreenBindingMode;
this->FixedSizeValue = ::int2(SceneCanvasProfile::DefaultWidth, SceneCanvasProfile::DefaultHeight);
}

::Entity* ViewportComponent::get_Parent()
{
return this->Component::get_Parent();
}

void ViewportComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

void ViewportComponent::AttachToCamera()
{
    if (this->BoundCameraComponentValue == nullptr)
    {
return;    }
this->BoundCameraComponentValue->ViewportChanged += &ViewportComponent::HandleCameraViewportChanged;
}

void ViewportComponent::AttachToWindowResize()
{
    if (this->IsSubscribedToWindowResizeValue)
    {
return;    }
Core::get_Instance()->get_RenderManager3D()->WindowResized += &ViewportComponent::HandleWindowResized;
this->IsSubscribedToWindowResizeValue = true;
}

void ViewportComponent::DetachFromCamera()
{
    if (this->BoundCameraComponentValue != nullptr)
    {
this->BoundCameraComponentValue->ViewportChanged -= &ViewportComponent::HandleCameraViewportChanged;
this->BoundCameraComponentValue = nullptr;
    }
}

void ViewportComponent::DetachFromWindowResize()
{
    if (!this->IsSubscribedToWindowResizeValue)
    {
return;    }
Core::get_Instance()->get_RenderManager3D()->WindowResized -= &ViewportComponent::HandleWindowResized;
this->IsSubscribedToWindowResizeValue = false;
}

void ViewportComponent::HandleCameraViewportChanged()
{
this->RaiseAnchorBoundsChanged();
}

void ViewportComponent::HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
this->RaiseAnchorBoundsChanged();
}

void ViewportComponent::RaiseAnchorBoundsChanged()
{
    if (true)
    {
this->AnchorBoundsChanged.Invoke();
    }
}

void ViewportComponent::RefreshSubscriptions()
{
    if (this->BindingModeValue == ScreenBindingMode)
    {
this->DetachFromCamera();
this->AttachToWindowResize();
    }
else     if (this->BindingModeValue == AncestorCameraBindingMode)
    {
this->DetachFromWindowResize();
::CameraComponent *nextCameraComponent = this->ResolveAncestorCameraComponent();
    if (!(this->BoundCameraComponentValue == nextCameraComponent))
    {
this->DetachFromCamera();
this->BoundCameraComponentValue = nextCameraComponent;
this->AttachToCamera();
    }
    }
else {
this->DetachFromCamera();
this->DetachFromWindowResize();
}
}

::CameraComponent* ViewportComponent::ResolveAncestorCameraComponent()
{
::Entity *current = Parent;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t index = 0; index < current->get_Components()->Count(); index++) {
    CameraComponent* cameraComponent = he_cpp_try_cast<CameraComponent>((*current->get_Components())[index]);
    if (cameraComponent != nullptr)
    {
return cameraComponent;    }
}
    }
current = current->get_Parent();
}
return nullptr;}

::int2 ViewportComponent::ResolveAnchorBounds()
{
    if (this->BindingModeValue == ScreenBindingMode)
    {
::int2 screenSize = Core::get_Instance()->get_RenderManager3D()->get_MainWindowSize();
    if (screenSize.X > 0 && screenSize.Y > 0)
    {
return screenSize;    }
return this->FixedSizeValue;    }
    if (this->BindingModeValue == AncestorCameraBindingMode)
    {
::CameraComponent *cameraComponent = this->ResolveAncestorCameraComponent();
    if (cameraComponent != nullptr)
    {
::float4 viewport = cameraComponent->get_Viewport();
const int32_t viewportWidth = Math::Max(1, static_cast<int32_t>(Math::Round(viewport.Z)));
const int32_t viewportHeight = Math::Max(1, static_cast<int32_t>(Math::Round(viewport.W)));
return ::int2(viewportWidth, viewportHeight);    }
return this->FixedSizeValue;    }
return this->FixedSizeValue;}

