#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;
class EngineBinaryReader;
class SceneAssetReference;
class TextComponent;
class Component;
class byte4;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;

#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "SceneAssetReference.hpp"
#include "TextComponent.hpp"
#include "Component.hpp"
#include "SceneAssetReference.hpp"
#include "byte4.hpp"
#include "runtime/native_string.hpp"
#include "Component.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "byte4.hpp"

class RuntimeTextComponentDeserializer : public IRuntimeComponentDeserializer
{
public:
    virtual ~RuntimeTextComponentDeserializer() = default;

    std::string get_ComponentTypeId();

    ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
private:
    static std::string ComponentType;

    static uint8_t CurrentVersion;

    static ::byte4 ReadByte4(::EngineBinaryReader* reader);

    static ::SceneAssetReference* ReadOptionalReference(::EngineBinaryReader* reader);
};
