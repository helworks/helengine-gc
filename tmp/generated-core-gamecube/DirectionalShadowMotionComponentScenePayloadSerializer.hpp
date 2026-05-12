#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class EngineBinaryWriter;
class DirectionalShadowCameraOrbitComponent;
class DirectionalShadowOrbitComponent;
class DirectionalShadowSunSweepComponent;
class DirectionalShadowTowerSpinComponent;
class float3;
class EngineBinaryReader;

#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "DirectionalShadowCameraOrbitComponent.hpp"
#include "DirectionalShadowCameraOrbitComponent.hpp"
#include "DirectionalShadowOrbitComponent.hpp"
#include "DirectionalShadowOrbitComponent.hpp"
#include "DirectionalShadowSunSweepComponent.hpp"
#include "DirectionalShadowTowerSpinComponent.hpp"
#include "float3.hpp"
#include "EngineBinaryReader.hpp"
#include "DirectionalShadowSunSweepComponent.hpp"
#include "DirectionalShadowTowerSpinComponent.hpp"
#include "EngineBinaryWriter.hpp"
#include "float3.hpp"

class DirectionalShadowMotionComponentScenePayloadSerializer
{
public:
    virtual ~DirectionalShadowMotionComponentScenePayloadSerializer() = default;

    static uint8_t CurrentVersion;

    static ::DirectionalShadowCameraOrbitComponent* ReadCameraOrbit(::EngineBinaryReader* reader);

    static ::DirectionalShadowOrbitComponent* ReadOrbit(::EngineBinaryReader* reader);

    static ::DirectionalShadowSunSweepComponent* ReadSunSweep(::EngineBinaryReader* reader);

    static ::DirectionalShadowTowerSpinComponent* ReadTowerSpin(::EngineBinaryReader* reader);

    static void WriteCameraOrbit(::EngineBinaryWriter* writer, ::DirectionalShadowCameraOrbitComponent* component);

    static void WriteOrbit(::EngineBinaryWriter* writer, ::DirectionalShadowOrbitComponent* component);

    static void WriteSunSweep(::EngineBinaryWriter* writer, ::DirectionalShadowSunSweepComponent* component);

    static void WriteTowerSpin(::EngineBinaryWriter* writer, ::DirectionalShadowTowerSpinComponent* component);
private:
    static ::float3 ReadFloat3(::EngineBinaryReader* reader);

    static void ReadOrbitFields(::EngineBinaryReader* reader, ::float3& orbitCenter, float& orbitRadius, float& orbitHeight, float& baseAngleRadians, float& angularSpeedRadians);

    static void WriteFloat3(::EngineBinaryWriter* writer, ::float3 value);

    static void WriteOrbitFields(::EngineBinaryWriter* writer, ::float3 orbitCenter, float orbitRadius, float orbitHeight, float baseAngleRadians, float angularSpeedRadians);
};
