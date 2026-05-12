#ifdef DrawText
#undef DrawText
#endif
#include "AutomaticScriptComponentRuntimeDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/array.hpp"
#include "MemberInfo.hpp"
#include "runtime/native_type.hpp"
#include "ConstructorInfo.hpp"
#include "Activator.hpp"
#include "PropertyInfo.hpp"
#include "FieldInfo.hpp"
#include "EngineBinaryEndianness.hpp"
#include "byte4.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/string_comparer.hpp"

uint8_t AutomaticScriptComponentRuntimeDeserializer::CurrentVersion = 1;

std::string AutomaticScriptComponentRuntimeDeserializer::get_ComponentTypeId()
{
return this->ComponentTypeIdValue;
}

AutomaticScriptComponentRuntimeDeserializer::AutomaticScriptComponentRuntimeDeserializer(std::string componentTypeId, Type* componentType) : ComponentTypeIdValue(), ComponentTypeValue(), MemberTypes(), Members()
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
throw ([&]() {
auto __ctor_arg_00000202 = "Component type id must be provided.";
auto __ctor_arg_00000203 = "componentTypeId";
return new ArgumentException(__ctor_arg_00000202, __ctor_arg_00000203);
})();
    }
    if (componentType == nullptr)
    {
throw new ArgumentNullException("componentType");
    }
    if (!he_cpp_type_of<Component>("Component")->IsAssignableFrom(componentType))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserializers require a ") + "Component" + std::string(" type."));
    }
this->ComponentTypeIdValue = componentTypeId;
this->ComponentTypeValue = componentType;
this->Members = this->LoadMembers(componentType);
this->MemberTypes = this->LoadMemberTypes(this->Members);
}

::Component* AutomaticScriptComponentRuntimeDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (!String::Equals(record->get_ComponentTypeId(), this->ComponentTypeIdValue, StringComparison::Ordinal))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
::Component *component = this->CreateComponent(this->ComponentTypeValue);
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000204 = ([&]() {
Array<uint8_t>* __coalesce_00000205 = record->get_Payload();
return __coalesce_00000205 != nullptr ? __coalesce_00000205 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000206 = false;
return new ::MemoryStream(__ctor_arg_00000204, __ctor_arg_00000206);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported automatic scripted component payload version '") + std::to_string(version) + std::string("'."));
    }
const int32_t memberCount = reader->ReadInt32();
    if (memberCount != this->Members->Length)
    {
throw new InvalidOperationException(std::string("Packaged scripted component '") + this->ComponentTypeIdValue + std::string("' expected ") + std::to_string(Members->Length) + std::string(" members but payload contained ") + std::to_string(memberCount) + std::string("."));
    }
for (int32_t index = 0; index < this->Members->Length; index++) {
this->SetMemberValue(component, (*this->Members)[index], this->ReadSupportedValue(reader, (*this->MemberTypes)[index]));
}
return component;}
}
}

::Component* AutomaticScriptComponentRuntimeDeserializer::CreateComponent(Type* componentType)
{
    if (componentType == nullptr)
    {
throw new ArgumentNullException("componentType");
    }
ConstructorInfo *constructor = componentType->GetConstructor(Type::EmptyTypes);
    if (constructor == nullptr || !constructor->IsPublic)
    {
throw new InvalidOperationException(std::string("Scripted component type '") + componentType->get_FullName() + std::string("' must expose a public parameterless constructor."));
    }
const void *instance = Activator::CreateInstance(componentType);
    if ()
    {
throw new InvalidOperationException(std::string("Scripted component type '") + componentType->get_FullName() + std::string("' could not be instantiated."));
    }
return component;}

Type* AutomaticScriptComponentRuntimeDeserializer::GetMemberType(MemberInfo* memberInfo)
{
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
return propertyInfo->get_PropertyType();    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
return fieldInfo->get_FieldType();    }
throw new InvalidOperationException(std::string("Reflected member '") + memberInfo?.Name + std::string("' is not a supported property or field."));
}

bool AutomaticScriptComponentRuntimeDeserializer::IsSupportedMember(MemberInfo* memberInfo)
{
    if (memberInfo->IsDefined(he_cpp_type_of<ScenePersistenceIgnoreAttribute>("ScenePersistenceIgnoreAttribute"), false))
    {
return false;    }
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
    if (propertyInfo->GetMethod == nullptr || !propertyInfo->GetMethod->IsPublic)
    {
return false;    }
    if (propertyInfo->SetMethod == nullptr || !propertyInfo->SetMethod->IsPublic)
    {
return false;    }
    if (propertyInfo->GetIndexParameters()->Length != 0)
    {
return false;    }
return true;    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
    if (!fieldInfo->IsPublic || fieldInfo->IsStatic || fieldInfo->IsInitOnly)
    {
return false;    }
return true;    }
return false;}

Array<Type*>* AutomaticScriptComponentRuntimeDeserializer::LoadMemberTypes(Array<MemberInfo*>* members)
{
    if (members == nullptr)
    {
throw new ArgumentNullException("members");
    }
Array<Type*> *memberTypes = new Array<Type*>(members->Length);
for (int32_t index = 0; index < members->Length; index++) {
(*memberTypes)[index] = GetMemberType((*members)[index]);
}
return memberTypes;}

Array<MemberInfo*>* AutomaticScriptComponentRuntimeDeserializer::LoadMembers(Type* componentType)
{
return componentType->GetMembers(BindingFlags::Instance | BindingFlags::Public)->Where<MemberInfo*>(new Func<MemberInfo*, bool>(&AutomaticScriptComponentRuntimeDeserializer::IsSupportedMember))->OrderBy<MemberInfo*, std::string>(new Func<MemberInfo*, std::string>(member => member->get_Name()), StringComparer::Ordinal)->ToArray<MemberInfo*>();}

void* AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(::EngineBinaryReader* reader, Type* valueType)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (valueType == nullptr)
    {
throw new ArgumentNullException("valueType");
    }
    if (valueType == he_cpp_type_of<std::string>("string"))
    {
return reader->ReadString();    }
    if (valueType == he_cpp_type_of<bool>("Boolean"))
    {
return reader->ReadByte() != 0;    }
    if (valueType == he_cpp_type_of<uint8_t>("byte"))
    {
return reader->ReadByte();    }
    if (valueType == he_cpp_type_of<uint16_t>("UInt16"))
    {
return reader->ReadUInt16();    }
    if (valueType == he_cpp_type_of<int32_t>("Int32"))
    {
return reader->ReadInt32();    }
    if (valueType == he_cpp_type_of<uint32_t>("UInt32"))
    {
return reader->ReadUInt32();    }
    if (valueType == he_cpp_type_of<int64_t>("Int64"))
    {
return reader->ReadInt64();    }
    if (valueType == he_cpp_type_of<float>("Single"))
    {
return reader->ReadSingle();    }
    if (valueType == he_cpp_type_of<int2>("int2"))
    {
return reader->ReadInt2();    }
    if (valueType == he_cpp_type_of<int4>("int4"))
    {
return reader->ReadInt4();    }
    if (valueType == he_cpp_type_of<float2>("float2"))
    {
return reader->ReadFloat2();    }
    if (valueType == he_cpp_type_of<float3>("float3"))
    {
return reader->ReadFloat3();    }
    if (valueType == he_cpp_type_of<float4>("float4"))
    {
return reader->ReadFloat4();    }
    if (valueType == he_cpp_type_of<byte4>("byte4"))
    {
return ([&]() {
auto __ctor_arg_00000207 = reader->ReadByte();
auto __ctor_arg_00000208 = reader->ReadByte();
auto __ctor_arg_00000209 = reader->ReadByte();
auto __ctor_arg_0000020A = reader->ReadByte();
return ::byte4(__ctor_arg_00000207, __ctor_arg_00000208, __ctor_arg_00000209, __ctor_arg_0000020A);
})();    }
    if (valueType == he_cpp_type_of<SceneEntityReference>("SceneEntityReference"))
    {
return reader->ReadSceneEntityReference();    }
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization does not support member type '") + valueType->get_FullName() + std::string("'."));
}

void AutomaticScriptComponentRuntimeDeserializer::SetMemberValue(::Component* component, MemberInfo* memberInfo, void* value)
{
    if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
propertyInfo->SetValue(component, value);
return;    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
fieldInfo->SetValue(component, value);
return;    }
throw new InvalidOperationException(std::string("Reflected member '") + memberInfo?.Name + std::string("' is not a supported property or field."));
}

