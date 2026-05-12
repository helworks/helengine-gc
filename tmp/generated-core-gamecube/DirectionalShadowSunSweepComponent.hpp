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

class DirectionalShadowSunSweepComponent : public UpdateComponent
{
public:
    virtual ~DirectionalShadowSunSweepComponent() = default;

    DirectionalShadowSunSweepComponent();

    static std::string SerializedComponentTypeId;

    float MaxYawRadians;

    float get_MaxYawRadians();
    void set_MaxYawRadians(float value);

    float MinYawRadians;

    float get_MinYawRadians();
    void set_MinYawRadians(float value);

    float PitchRadians;

    float get_PitchRadians();
    void set_PitchRadians(float value);

    float SweepSpeedRadians;

    float get_SweepSpeedRadians();
    void set_SweepSpeedRadians(float value);

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
};
