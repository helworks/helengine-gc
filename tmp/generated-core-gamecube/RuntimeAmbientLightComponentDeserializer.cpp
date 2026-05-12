#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeAmbientLightComponentDeserializer.hpp"
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

std::string RuntimeAmbientLightComponentDeserializer::get_ComponentTypeId()
{
return ComponentType;
}

::Component* RuntimeAmbientLightComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
else     if (!String::Equals(record->get_ComponentTypeId(), ComponentType, StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Ambient light deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000021C = ([&]() {
Array<uint8_t>* __coalesce_0000021D = record->get_Payload();
return __coalesce_0000021D != nullptr ? __coalesce_0000021D : Array<uint8_t>::Empty();
})();
auto __ctor_arg_0000021E = false;
return new ::MemoryStream(__ctor_arg_0000021C, __ctor_arg_0000021E);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != LightComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported ambient light payload version '") + std::to_string(version) + std::string("'."));
    }
return LightComponentScenePayloadSerializer::ReadAmbientLight(reader, version);}
}
}

std::string RuntimeAmbientLightComponentDeserializer::ComponentType = "helengine.AmbientLightComponent";

