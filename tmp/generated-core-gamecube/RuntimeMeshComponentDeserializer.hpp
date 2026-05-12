#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;
class EngineBinaryReader;
class MeshComponentScenePayloadSerializer;
class MeshComponent;
class Component;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;
class RuntimeMaterial;
class SceneAssetReference;

#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryReader.hpp"
#include "MeshComponentScenePayloadSerializer.hpp"
#include "MeshComponentScenePayloadSerializer.hpp"
#include "MeshComponent.hpp"
#include "MeshComponent.hpp"
#include "Component.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "Component.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "runtime/array.hpp"
#include "RuntimeMaterial.hpp"
#include "runtime/array.hpp"
#include "SceneAssetReference.hpp"

class RuntimeMeshComponentDeserializer : public IRuntimeComponentDeserializer
{
public:
    virtual ~RuntimeMeshComponentDeserializer() = default;

    std::string get_ComponentTypeId();

    ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
private:
    static std::string ComponentType;

    static uint8_t CurrentVersion;

    static Array<::RuntimeMaterial*>* ResolveMaterials(Array<::SceneAssetReference*>* references, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
};
