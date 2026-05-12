#ifdef DrawText
#undef DrawText
#endif
#include "PointerInteractableHitResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "ICamera.hpp"
#include "IInteractable2D.hpp"
#include "Entity.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "IDrawable2D.hpp"
#include "GeometryUtils.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

void PointerInteractableHitResolver::GetRelativePointerForInteractable(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY, ::ICamera* camera, int32_t& relativeX, int32_t& relativeY)
{
    if (interactable == nullptr)
    {
throw new ArgumentNullException("interactable");
    }
::float3 position = interactable->get_Parent()->get_Position();
relativeX = static_cast<int32_t>(Math::Round(pointerX - position.X));
relativeY = static_cast<int32_t>(Math::Round(pointerY - position.Y));
}

::IInteractable2D* PointerInteractableHitResolver::ResolveTopInteractableAt(List<::IInteractable2D*>* interactables, List<::IDrawable2D*>* drawables2D, ::ICamera* camera, int32_t pointerX, int32_t pointerY)
{
    if (interactables == nullptr)
    {
throw new ArgumentNullException("interactables");
    }
    if (drawables2D == nullptr)
    {
throw new ArgumentNullException("drawables2D");
    }
    if (camera == nullptr)
    {
throw new ArgumentNullException("camera");
    }
const uint16_t cameraLayerMask = camera->get_LayerMask();
::IInteractable2D *hit = nullptr;
uint8_t hitRenderOrder = 0;
int32_t hitDrawableIndex = -1;
int32_t hitInteractableIndex = -1;
for (int32_t interactableIndex = 0; interactableIndex < interactables->Count(); interactableIndex++) {
::IInteractable2D *interactable = (*interactables)[interactableIndex];
    if ((interactable->get_Parent()->get_LayerMask() & cameraLayerMask) == 0)
    {
continue;
    }
    if (!IsInsideActiveClipRegions(interactable, pointerX, pointerY))
    {
continue;
    }
::float3 position = interactable->get_Parent()->get_Position();
::float4 rect = ::float4(position.X, position.Y, interactable->get_Size().X, interactable->get_Size().Y);
    if (!rect.Contains(pointerX, pointerY))
    {
continue;
    }
int32_t candidateDrawableIndex;
const uint8_t candidateRenderOrder = GetTopDrawableRenderOrder(drawables2D, interactable, cameraLayerMask, candidateDrawableIndex);
    if (hit == nullptr || CandidateIsInFront(candidateRenderOrder, candidateDrawableIndex, interactableIndex, hitRenderOrder, hitDrawableIndex, hitInteractableIndex))
    {
hit = interactable;
hitRenderOrder = candidateRenderOrder;
hitDrawableIndex = candidateDrawableIndex;
hitInteractableIndex = interactableIndex;
    }
}
return hit;}

bool PointerInteractableHitResolver::CandidateIsInFront(uint8_t candidateRenderOrder, int32_t candidateDrawableIndex, int32_t candidateInteractableIndex, uint8_t currentRenderOrder, int32_t currentDrawableIndex, int32_t currentInteractableIndex)
{
    if (candidateRenderOrder != currentRenderOrder)
    {
return candidateRenderOrder > currentRenderOrder;    }
    if (candidateDrawableIndex != currentDrawableIndex)
    {
return candidateDrawableIndex > currentDrawableIndex;    }
return candidateInteractableIndex > currentInteractableIndex;}

uint8_t PointerInteractableHitResolver::GetTopDrawableRenderOrder(List<::IDrawable2D*>* drawables2D, ::IInteractable2D* interactable, uint16_t cameraLayerMask, int32_t& candidateDrawableIndex)
{
candidateDrawableIndex = -1;
uint8_t renderOrder = 0;
    if (drawables2D == nullptr || interactable == nullptr)
    {
return renderOrder;    }
for (int32_t drawableIndex = 0; drawableIndex < drawables2D->Count(); drawableIndex++) {
::IDrawable2D *drawable = (*drawables2D)[drawableIndex];
    if (drawable->get_Parent() != interactable->get_Parent())
    {
continue;
    }
    if ((drawable->get_Parent()->get_LayerMask() & cameraLayerMask) == 0)
    {
continue;
    }
    if (candidateDrawableIndex < 0 || drawable->get_RenderOrder2D() >= renderOrder)
    {
renderOrder = drawable->get_RenderOrder2D();
candidateDrawableIndex = drawableIndex;
    }
}
return renderOrder;}

bool PointerInteractableHitResolver::IsInsideActiveClipRegions(::IInteractable2D* interactable, int32_t pointerX, int32_t pointerY)
{
    if (interactable == nullptr || interactable->get_Parent() == nullptr)
    {
return false;    }
::Entity *current = interactable->get_Parent();
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->Count(); componentIndex++) {
    IClipRegion2D* clipRegion = he_cpp_try_cast<IClipRegion2D>((*current->get_Components())[componentIndex]);
    if (clipRegion != nullptr)
    {
::float4 clipRect = clipRegion->GetClipRect();
    if (!GeometryUtils::IsPointInsideRect(pointerX, pointerY, ::float3(clipRect.X, clipRect.Y, 0.0f), static_cast<int32_t>(clipRect.Z), static_cast<int32_t>(clipRect.W)))
    {
return false;    }
    }
}
    }
current = current->get_Parent();
}
return true;}

