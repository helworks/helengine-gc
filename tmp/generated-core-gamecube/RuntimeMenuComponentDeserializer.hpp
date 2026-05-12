#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;
class EngineBinaryReader;
class Component;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;

#include "IRuntimeComponentDeserializer.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "Component.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"

class RuntimeMenuComponentDeserializer : public IRuntimeComponentDeserializer
{
public:
    virtual ~RuntimeMenuComponentDeserializer() = default;

    std::string get_ComponentTypeId();

    ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
};
