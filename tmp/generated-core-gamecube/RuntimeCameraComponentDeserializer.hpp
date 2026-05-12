#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;
class EngineBinaryReader;
class CameraComponent;
class Component;
class float4;
class CameraClearSettings;
class CameraRenderSettings;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;

#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryReader.hpp"
#include "CameraComponent.hpp"
#include "Component.hpp"
#include "float4.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "runtime/native_string.hpp"
#include "Component.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "CameraClearSettings.hpp"
#include "float4.hpp"
#include "CameraRenderSettings.hpp"

class RuntimeCameraComponentDeserializer : public IRuntimeComponentDeserializer
{
public:
    virtual ~RuntimeCameraComponentDeserializer() = default;

    std::string get_ComponentTypeId();

    ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
private:
    static std::string ComponentType;

    static uint8_t CurrentVersion;

    static ::CameraClearSettings ReadClearSettings(::EngineBinaryReader* reader);

    static ::float4 ReadFloat4(::EngineBinaryReader* reader);

    static ::CameraRenderSettings* ReadRenderSettings(::EngineBinaryReader* reader);
};
