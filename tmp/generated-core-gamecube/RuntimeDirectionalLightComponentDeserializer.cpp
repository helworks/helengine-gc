#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeDirectionalLightComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "Component.hpp"
#include "EngineBinaryEndianness.hpp"
#include "LightComponentScenePayloadSerializer.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeDirectionalLightComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeDirectionalLightComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
else     if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Directional light deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000022E = ([&]() {
Array<uint8_t>* __coalesce_0000022F = record->get_Payload();
return __coalesce_0000022F != nullptr ? __coalesce_0000022F : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000230 = false;
return new ::MemoryStream(__ctor_arg_0000022E, __ctor_arg_00000230);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != LightComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported directional light payload version '") + std::to_string(version) + std::string("'."));
    }
return LightComponentScenePayloadSerializer::ReadDirectionalLight(reader, version);}
}
}

std::string RuntimeDirectionalLightComponentDeserializer::ComponentType = "helengine.DirectionalLightComponent";

