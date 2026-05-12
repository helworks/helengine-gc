#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;
class Component;
class EngineBinaryReader;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;

#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "EngineBinaryReader.hpp"
#include "Component.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "MemberInfo.hpp"
#include "runtime/native_type.hpp"
#include "ConstructorInfo.hpp"
#include "Activator.hpp"
#include "Activator.hpp"
#include "PropertyInfo.hpp"
#include "FieldInfo.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "MemberInfo.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"

class AutomaticScriptComponentRuntimeDeserializer : public IRuntimeComponentDeserializer
{
public:
    virtual ~AutomaticScriptComponentRuntimeDeserializer() = default;

    static uint8_t CurrentVersion;

    std::string get_ComponentTypeId();

    AutomaticScriptComponentRuntimeDeserializer(std::string componentTypeId, Type* componentType);

    ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
private:
    std::string ComponentTypeIdValue;

    Type* ComponentTypeValue;

    Array<Type*>* MemberTypes;

    Array<MemberInfo*>* Members;

    static ::Component* CreateComponent(Type* componentType);

    static Type* GetMemberType(MemberInfo* memberInfo);

    static bool IsSupportedMember(MemberInfo* memberInfo);

    static Array<Type*>* LoadMemberTypes(Array<MemberInfo*>* members);

    static Array<MemberInfo*>* LoadMembers(Type* componentType);

    static void* ReadSupportedValue(::EngineBinaryReader* reader, Type* valueType);

    static void SetMemberValue(::Component* component, MemberInfo* memberInfo, void* value);
};
