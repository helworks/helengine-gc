#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ICamera;
class IInteractable2D;
class Entity;
class float3;
class IDrawable2D;

#include "runtime/native_exceptions.hpp"
#include "ICamera.hpp"
#include "IInteractable2D.hpp"
#include "IInteractable2D.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "IDrawable2D.hpp"

class PointerInteractableHitResolver
{
public:
    virtual ~PointerInteractableHitResolver() = default;

    static void GetRelativePointerForInteractable(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY, ::ICamera* camera, int32_t& relativeX, int32_t& relativeY);

    static ::IInteractable2D* ResolveTopInteractableAt(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, ::ICamera* camera, int32_t pointerX, int32_t pointerY);
private:
    static bool CandidateIsInFront(uint8_t candidateRenderOrder, int32_t candidateDrawableIndex, int32_t candidateInteractableIndex, uint8_t currentRenderOrder, int32_t currentDrawableIndex, int32_t currentInteractableIndex);

    static uint8_t GetTopDrawableRenderOrder(List<::IDrawable2D*>* drawables2D, ::IInteractable2D* interactable, uint16_t cameraLayerMask, int32_t& candidateDrawableIndex);

    static bool IsInsideActiveClipRegions(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY);
};
