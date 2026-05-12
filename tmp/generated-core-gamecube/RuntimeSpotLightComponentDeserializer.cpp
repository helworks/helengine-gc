#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSpotLightComponentDeserializer.hpp"
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

std::string RuntimeSpotLightComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeSpotLightComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
else     if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Spot light deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000264 = ([&]() {
Array<uint8_t>* __coalesce_00000265 = record->get_Payload();
return __coalesce_00000265 != nullptr ? __coalesce_00000265 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000266 = false;
return new ::MemoryStream(__ctor_arg_00000264, __ctor_arg_00000266);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != LightComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported spot light payload version '") + std::to_string(version) + std::string("'."));
    }
return LightComponentScenePayloadSerializer::ReadSpotLight(reader, version);}
}
}

std::string RuntimeSpotLightComponentDeserializer::ComponentType = "helengine.SpotLightComponent";

