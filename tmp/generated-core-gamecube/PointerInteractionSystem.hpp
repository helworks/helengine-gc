#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ObjectManager;
class Core;
class InputSystem;
class PointerInteractableHitResolver;
class int2;
class IInteractable2D;
class ICamera;
class float4;
class RenderManager3D;
class IDrawable2D;

#include "runtime/native_exceptions.hpp"
#include "ObjectManager.hpp"
#include "Core.hpp"
#include "runtime/native_list.hpp"
#include "ObjectManager.hpp"
#include "runtime/native_list.hpp"
#include "PointerInteraction.hpp"
#include "PointerInteraction.hpp"
#include "InputSystem.hpp"
#include "PointerInteractableHitResolver.hpp"
#include "PointerInteractableHitResolver.hpp"
#include "int2.hpp"
#include "IInteractable2D.hpp"
#include "ICamera.hpp"
#include "runtime/native_list.hpp"
#include "ICamera.hpp"
#include "float4.hpp"
#include "RenderManager3D.hpp"
#include "Core.hpp"
#include "IInteractable2D.hpp"
#include "PointerCursorKind.hpp"
#include "InputSystem.hpp"
#include "IDrawable2D.hpp"
#include "float4.hpp"

class PointerInteractionSystem
{
public:
    virtual ~PointerInteractionSystem() = default;

    ::Core* Core;

    ::Core* get_Core();
    void set_Core(::Core* value);

    ::IInteractable2D* Highlighted;

    ::IInteractable2D* get_Highlighted();
    void set_Highlighted(::IInteractable2D* value);

    ::PointerCursorKind get_HoverCursor();

    ::IInteractable2D* Hovering;

    ::IInteractable2D* get_Hovering();
    void set_Hovering(::IInteractable2D* value);

    ::InputSystem* Input;

    ::InputSystem* get_Input();
    void set_Input(::InputSystem* value);

    void ClearInteractionFor(::IInteractable2D* interactable);

    PointerInteractionSystem(::Core* core, ::InputSystem* inputSystem);

    void Update();
private:
    ::ICamera* capturedCamera;

    ::ICamera* FindCameraForInteractableAt(::IInteractable2D* interactable, int32_t x, int32_t y);

    void ResolveTopInteractableAt(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, int32_t x, int32_t y, ::IInteractable2D*& hitInteractable, ::ICamera*& hitCamera);

    ::float4 ResolveViewportInWindowSpace(::ICamera* camera);
};
