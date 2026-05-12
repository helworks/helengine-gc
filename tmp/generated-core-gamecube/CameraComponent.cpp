#ifdef DrawText
#undef DrawText
#endif
#include "CameraComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "CoreInitializationOptions.hpp"
#include "Core.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "ObjectManager.hpp"
#include "CameraProjectionUtils.hpp"
#include "float4.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "RenderList2D.hpp"
#include "RenderList3D.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

uint8_t CameraComponent::get_CameraDrawOrder()
{
return this->cameraDrawOrder;}

void CameraComponent::set_CameraDrawOrder(uint8_t value)
{
    if (this->cameraDrawOrder != value)
    {
    if (Parent != nullptr && Parent->get_IsHierarchyEnabled())
    {
Core::get_Instance()->get_ObjectManager()->RemoveCamera(this);
this->cameraDrawOrder = value;
Core::get_Instance()->get_ObjectManager()->RegisterCamera(this);
    }
else {
this->cameraDrawOrder = value;
}
    }
}

::CameraClearSettings CameraComponent::get_ClearSettings()
{
return this->ClearSettings;
}

void CameraComponent::set_ClearSettings(::CameraClearSettings value)
{
this->ClearSettings = value;
}

float CameraComponent::get_FarPlaneDistance()
{
return this->FarPlaneDistanceValue;}

void CameraComponent::set_FarPlaneDistance(float value)
{
this->FarPlaneDistanceValue = CameraProjectionUtils::ClampFarPlaneDistance(this->NearPlaneDistanceValue, value);
}

uint16_t CameraComponent::get_LayerMask()
{
return this->layerMask;}

void CameraComponent::set_LayerMask(uint16_t value)
{
    if (this->layerMask != value)
    {
    if (Parent != nullptr && Parent->get_IsHierarchyEnabled())
    {
Core::get_Instance()->get_ObjectManager()->RemoveCamera(this);
this->layerMask = value;
Core::get_Instance()->get_ObjectManager()->RegisterCamera(this);
    }
else {
this->layerMask = value;
}
    }
}

float CameraComponent::get_NearPlaneDistance()
{
return this->NearPlaneDistanceValue;}

void CameraComponent::set_NearPlaneDistance(float value)
{
this->NearPlaneDistanceValue = CameraProjectionUtils::ClampNearPlaneDistance(value, this->FarPlaneDistanceValue);
this->FarPlaneDistanceValue = CameraProjectionUtils::ClampFarPlaneDistance(this->NearPlaneDistanceValue, this->FarPlaneDistanceValue);
}

::IRenderQueue2D* CameraComponent::get_RenderQueue2D()
{
return this->renderList2D;}

::IRenderQueue3D* CameraComponent::get_RenderQueue3D()
{
return this->renderList3D;}

::CameraRenderSettings* CameraComponent::get_RenderSettings()
{
return this->RenderSettingsValue;}

void CameraComponent::set_RenderSettings(::CameraRenderSettings* value)
{
this->RenderSettingsValue = (value != nullptr ? value : throw new ArgumentNullException("value"));
}

::RenderTarget* CameraComponent::get_RenderTarget()
{
return this->RenderTarget;
}

void CameraComponent::set_RenderTarget(::RenderTarget* value)
{
this->RenderTarget = value;
}

::float4 CameraComponent::get_Viewport()
{
return this->viewportValue;}

void CameraComponent::set_Viewport(::float4 value)
{
    if (this->viewportValue.X != value.X || this->viewportValue.Y != value.Y || this->viewportValue.Z != value.Z || this->viewportValue.W != value.W)
    {
this->viewportValue = value;
this->RaiseViewportChanged();
    }
}

CameraComponent::CameraComponent() : ClearSettings(), RenderTarget(), ViewportChanged(), FarPlaneDistanceValue(), NearPlaneDistanceValue(), RenderSettingsValue(), cameraDrawOrder(), layerMask(), renderList2D(), renderList3D(), viewportValue()
{
this->set_LayerMask(0b11111111);
this->viewportValue = ::float4(0, 0, 1, 1);
this->NearPlaneDistanceValue = 0.1f;
this->FarPlaneDistanceValue = 100.0f;
this->set_ClearSettings(([&]() {
auto __ctor_arg_0000027C = true;
auto __ctor_arg_0000027D = ::float4(0.0f, 0.0f, 0.0f, 0.0f);
auto __ctor_arg_0000027E = true;
auto __ctor_arg_0000027F = 1.0f;
auto __ctor_arg_00000280 = false;
auto __ctor_arg_00000281 = 0;
return ::CameraClearSettings(__ctor_arg_0000027C, __ctor_arg_0000027D, __ctor_arg_0000027E, __ctor_arg_0000027F, __ctor_arg_00000280, __ctor_arg_00000281);
})());
this->set_RenderSettings(new ::CameraRenderSettings());
this->InitializeLists();
}

void CameraComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
    if (entity->get_IsHierarchyEnabled())
    {
Core::get_Instance()->get_ObjectManager()->RegisterCamera(this);
    }
}

void CameraComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
Core::get_Instance()->get_ObjectManager()->RegisterCamera(this);
    }
else {
Core::get_Instance()->get_ObjectManager()->RemoveCamera(this);
}
}

::Entity* CameraComponent::get_Parent()
{
return this->Component::get_Parent();
}

void CameraComponent::set_Parent(::Entity* value)
{
this->Component::set_Parent(value);
}

void CameraComponent::InitializeLists()
{
    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_InitializationOptions() == nullptr)
    {
throw new InvalidOperationException("Core initialization options must be set before creating camera lists.");
    }
::CoreInitializationOptions *settings = Core::get_Instance()->get_InitializationOptions();
settings->Normalize();
this->renderList2D = new ::RenderList2D(settings->get_RenderList2DInitialCapacity());
this->renderList3D = new ::RenderList3D(settings->get_RenderList3DInitialCapacity());
}

void CameraComponent::RaiseViewportChanged()
{
    if (true)
    {
this->ViewportChanged.Invoke();
    }
}

