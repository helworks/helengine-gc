#ifdef DrawText
#undef DrawText
#endif
#include "RuntimePointLightComponentDeserializer.hpp"
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

std::string RuntimePointLightComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimePointLightComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
else     if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Point light deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000259 = ([&]() {
Array<uint8_t>* __coalesce_0000025A = record->get_Payload();
return __coalesce_0000025A != nullptr ? __coalesce_0000025A : Array<uint8_t>::Empty();
})();
auto __ctor_arg_0000025B = false;
return new ::MemoryStream(__ctor_arg_00000259, __ctor_arg_0000025B);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != LightComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported point light payload version '") + std::to_string(version) + std::string("'."));
    }
return LightComponentScenePayloadSerializer::ReadPointLight(reader, version);}
}
}

std::string RuntimePointLightComponentDeserializer::ComponentType = "helengine.PointLightComponent";

