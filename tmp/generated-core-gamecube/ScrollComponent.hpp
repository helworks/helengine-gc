#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IClipRegion2D;
class IUpdateable;
class float4;
class Core;
class InputSystem;
class float3;
class Entity;
class int2;

#include "UpdateComponent.hpp"
#include "IClipRegion2D.hpp"
#include "IUpdateable.hpp"
#include "float4.hpp"
#include "Core.hpp"
#include "Core.hpp"
#include "InputSystem.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "float4.hpp"
#include "Entity.hpp"
#include "runtime/native_event.hpp"
#include "int2.hpp"

class ScrollComponent : public UpdateComponent, public IClipRegion2D
{
public:
    virtual ~ScrollComponent() = default;

    ScrollComponent();

    ::Entity* get_ContentRoot();

    void set_ContentRoot(::Entity* value);

    int32_t get_ItemCount();

    void set_ItemCount(int32_t value);

    int32_t get_ItemExtent();

    void set_ItemExtent(int32_t value);

    int32_t get_MaximumScrollOffset();

    bool get_RequiresPointerInside();

    void set_RequiresPointerInside(bool value);

    int32_t ScrollOffset;

    int32_t get_ScrollOffset();
    void set_ScrollOffset(int32_t value);

    ::Event ScrollOffsetChanged;

    int32_t get_ScrollStepCount();

    void set_ScrollStepCount(int32_t value);

    ::int2 get_Size();

    void set_Size(::int2 value);

    bool get_UsesAutomaticVisibleItemCount();

    int32_t get_VisibleItemCount();

    void set_VisibleItemCount(int32_t value);

    int32_t get_WheelNotchSize();

    void set_WheelNotchSize(int32_t value);

    void ClampScrollOffset();

    bool ContainsScreenPoint(int32_t x, int32_t y);

    ::float4 GetClipRect();

    void ResetScrollOffset();

    bool ScrollTo(int32_t scrollOffset);

    bool TryApplyWheelInput();

    void Update();

    ::Entity* get_Parent();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    void set_Parent(::Entity* value);
private:
    static int32_t StandardWheelNotch;

    ::Entity* ContentRootValue;

    int32_t ItemCountValue;

    int32_t ItemExtentValue;

    bool RequiresPointerInsideValue;

    int32_t ScrollStepCountValue;

    ::int2 SizeValue;

    int32_t VisibleItemCountValue;

    int32_t WheelNotchSizeValue;

    void ApplyContentRootOffset();

    int32_t ClampOffset(int32_t scrollOffset);

    int32_t GetVisibleItemCount();

    bool SetScrollOffset(int32_t scrollOffset, bool raiseEvent);
};
