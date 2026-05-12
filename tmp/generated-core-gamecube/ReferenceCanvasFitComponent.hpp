#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IAnchorBoundsProvider;
class IUpdateable;
class AnchorSpace;
class float2;
class int2;
class Core;
class RenderManager3D;
class ReferenceCanvasFitSnapshot;
class Entity;

#include "UpdateComponent.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "IUpdateable.hpp"
#include "UpdateComponent.hpp"
#include "runtime/native_list.hpp"
#include "AnchorSpace.hpp"
#include "float2.hpp"
#include "int2.hpp"
#include "Core.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "AnchorSpace.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "float2.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_list.hpp"
#include "ReferenceCanvasFitSnapshot.hpp"
#include "Entity.hpp"

class ReferenceCanvasFitComponent : public UpdateComponent, public IAnchorBoundsProvider
{
public:
    virtual ~ReferenceCanvasFitComponent() = default;

    ::Event AnchorBoundsChanged;

    ::AnchorSpace* get_AnchorSpace();

    int32_t get_ReferenceHeight();

    void set_ReferenceHeight(int32_t value);

    int32_t get_ReferenceWidth();

    void set_ReferenceWidth(int32_t value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void ParentEnabledChange(bool newEnabled);

    ReferenceCanvasFitComponent();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    ::AnchorSpace* CurrentAnchorSpaceValue;

    ::float2 CurrentCanvasOriginValue;

    bool IsSubscribedToWindowResizeValue;

    bool PendingApplyValue;

    int32_t ReferenceHeightValue;

    int32_t ReferenceWidthValue;

    int32_t SnapshotEntityCountValue;

    List<::ReferenceCanvasFitSnapshot*>* SnapshotsValue;

    void ApplyCurrentScale();

    void AttachToWindowResize();

    void CaptureSnapshotsRecursive(::Entity* entity, bool isRootEntity);

    int32_t CountEntitiesRecursive(::Entity* entity);

    void DetachFromWindowResize();

    bool DidAnchorSpaceChange(::AnchorSpace* currentAnchorSpace, ::AnchorSpace* resolvedAnchorSpace);

    bool DidCanvasOriginChange(::float2 currentCanvasOrigin, ::float2 resolvedCanvasOrigin);

    void HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight);

    bool LiveWindowMatchesReferenceAspect(double liveWidth, double liveHeight);

    void RaiseAnchorBoundsChanged();

    void RebuildSnapshots();

    ::AnchorSpace* ResolveCurrentAnchorSpace();

    ::float2 ResolveCurrentCanvasOrigin(::AnchorSpace* anchorSpace);
};
