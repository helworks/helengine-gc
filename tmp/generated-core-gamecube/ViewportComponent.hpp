#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IAnchorBoundsProvider;
class CameraComponent;
class int2;
class Core;
class RenderManager3D;
class float4;
class Entity;
class AnchorSpace;

#include "Component.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "Component.hpp"
#include "CameraComponent.hpp"
#include "int2.hpp"
#include "Core.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "int2.hpp"
#include "float4.hpp"
#include "CameraComponent.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "Entity.hpp"
#include "runtime/native_event.hpp"
#include "AnchorSpace.hpp"

class ViewportComponent : public Component, public IAnchorBoundsProvider
{
public:
    virtual ~ViewportComponent() = default;

    static uint8_t AncestorCameraBindingMode;

    static uint8_t FixedBindingMode;

    static uint8_t ScreenBindingMode;

    ::Event AnchorBoundsChanged;

    ::AnchorSpace* get_AnchorSpace();

    uint8_t get_BindingMode();

    void set_BindingMode(uint8_t value);

    ::int2 get_FixedSize();

    void set_FixedSize(::int2 value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void ParentEnabledChange(bool newEnabled);

    ViewportComponent();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    uint8_t BindingModeValue;

    ::CameraComponent* BoundCameraComponentValue;

    ::int2 FixedSizeValue;

    bool IsSubscribedToWindowResizeValue;

    void AttachToCamera();

    void AttachToWindowResize();

    void DetachFromCamera();

    void DetachFromWindowResize();

    void HandleCameraViewportChanged();

    void HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight);

    void RaiseAnchorBoundsChanged();

    void RefreshSubscriptions();

    ::CameraComponent* ResolveAncestorCameraComponent();

    ::int2 ResolveAnchorBounds();
};
