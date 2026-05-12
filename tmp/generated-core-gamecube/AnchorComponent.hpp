#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class AnchorSpace;
class int2;
class float3;
class Entity;
class float4;
class IAnchorBoundsProvider;
class IAnchorSizeProvider;

#include "Component.hpp"
#include "runtime/native_exceptions.hpp"
#include "AnchorSpace.hpp"
#include "int2.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "float4.hpp"
#include "runtime/native_nullable.hpp"
#include "Component.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "Entity.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "AnchorSpace.hpp"
#include "int2.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_nullable.hpp"

class AnchorComponent : public Component
{
public:
    virtual ~AnchorComponent() = default;

    AnchorComponent();

    static uint8_t BottomAnchorFlag;

    static uint8_t LeftAnchorFlag;

    static uint8_t RightAnchorFlag;

    static uint8_t TopAnchorFlag;

    ::float4 AnchorDistances;

    ::float4 get_AnchorDistances();
    void set_AnchorDistances(::float4 value);

    uint8_t AnchorFlags;

    uint8_t get_AnchorFlags();
    void set_AnchorFlags(uint8_t value);

    bool get_IsAnchored();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void DisableAnchoring();

    void EnableAnchoring(bool left, bool right, bool top, bool bottom);

    std::string GetAnchorInfo();

    void ParentEnabledChange(bool newEnabled);

    void RefreshAnchoring();

    void SetAnchorDistances(Nullable<float> left, Nullable<float> right, Nullable<float> top, Nullable<float> bottom);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
private:
    bool IsSubscribedToWindowResize;

    ::IAnchorBoundsProvider* anchorBoundsProvider;

    void AttachToWindowResize();

    void DetachFromBoundsProvider();

    void DetachFromWindowResize();

    int32_t GetAnchorArea(::int2 size);

    ::int2 GetAnchorSize();

    ::AnchorSpace* GetAnchorSpace();

    void HandleAnchorBoundsChanged();

    void HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight);

    void RefreshSubscriptions();

    ::IAnchorBoundsProvider* ResolveAnchorBoundsProvider();
};
