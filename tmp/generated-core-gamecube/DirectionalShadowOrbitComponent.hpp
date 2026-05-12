#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class float4;
class float3;

#include "UpdateComponent.hpp"
#include "IUpdateable.hpp"
#include "UpdateComponent.hpp"
#include "float4.hpp"
#include "float4.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"

class DirectionalShadowOrbitComponent : public UpdateComponent
{
public:
    virtual ~DirectionalShadowOrbitComponent() = default;

    DirectionalShadowOrbitComponent();

    static std::string SerializedComponentTypeId;

    float AngularSpeedRadians;

    float get_AngularSpeedRadians();
    void set_AngularSpeedRadians(float value);

    float BaseAngleRadians;

    float get_BaseAngleRadians();
    void set_BaseAngleRadians(float value);

    ::float3 OrbitCenter;

    ::float3 get_OrbitCenter();
    void set_OrbitCenter(::float3 value);

    float OrbitHeight;

    float get_OrbitHeight();
    void set_OrbitHeight(float value);

    float OrbitRadius;

    float get_OrbitRadius();
    void set_OrbitRadius(float value);

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);
};
