#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class float4;

#include "UpdateComponent.hpp"
#include "IUpdateable.hpp"
#include "UpdateComponent.hpp"
#include "float4.hpp"
#include "float4.hpp"
#include "runtime/native_string.hpp"

class DirectionalShadowTowerSpinComponent : public UpdateComponent
{
public:
    virtual ~DirectionalShadowTowerSpinComponent() = default;

    DirectionalShadowTowerSpinComponent();

    static std::string SerializedComponentTypeId;

    float AngularSpeedRadians;

    float get_AngularSpeedRadians();
    void set_AngularSpeedRadians(float value);

    float BaseYawRadians;

    float get_BaseYawRadians();
    void set_BaseYawRadians(float value);

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
};
