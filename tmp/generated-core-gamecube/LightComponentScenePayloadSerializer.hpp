#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class EngineBinaryWriter;
class DirectionalLightComponent;
class AmbientLightComponent;
class PointLightComponent;
class SpotLightComponent;
class float4;
class EngineBinaryReader;
class LightComponent;

#include "runtime/native_exceptions.hpp"
#include "EngineBinaryWriter.hpp"
#include "DirectionalLightComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "DirectionalLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "PointLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "SpotLightComponent.hpp"
#include "float4.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryWriter.hpp"
#include "LightComponent.hpp"
#include "float4.hpp"

class LightComponentScenePayloadSerializer
{
public:
    virtual ~LightComponentScenePayloadSerializer() = default;

    static uint8_t CurrentVersion;

    static ::AmbientLightComponent* ReadAmbientLight(::EngineBinaryReader* reader);

    static ::AmbientLightComponent* ReadAmbientLight(::EngineBinaryReader* reader, uint8_t version);

    static ::DirectionalLightComponent* ReadDirectionalLight(::EngineBinaryReader* reader);

    static ::DirectionalLightComponent* ReadDirectionalLight(::EngineBinaryReader* reader, uint8_t version);

    static ::PointLightComponent* ReadPointLight(::EngineBinaryReader* reader);

    static ::PointLightComponent* ReadPointLight(::EngineBinaryReader* reader, uint8_t version);

    static ::SpotLightComponent* ReadSpotLight(::EngineBinaryReader* reader);

    static ::SpotLightComponent* ReadSpotLight(::EngineBinaryReader* reader, uint8_t version);

    static void WriteAmbientLight(::EngineBinaryWriter* writer, ::AmbientLightComponent* lightComponent);

    static void WriteDirectionalLight(::EngineBinaryWriter* writer, ::DirectionalLightComponent* lightComponent);

    static void WritePointLight(::EngineBinaryWriter* writer, ::PointLightComponent* lightComponent);

    static void WriteSpotLight(::EngineBinaryWriter* writer, ::SpotLightComponent* lightComponent);
private:
    static void ReadCommonLightFields(::EngineBinaryReader* reader, ::LightComponent* lightComponent);

    static ::float4 ReadFloat4(::EngineBinaryReader* reader);

    static void WriteCommonLightFields(::EngineBinaryWriter* writer, ::LightComponent* lightComponent);

    static void WriteFloat4(::EngineBinaryWriter* writer, ::float4 value);
};
