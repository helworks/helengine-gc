#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeDirectionalShadowSunSweepComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "Component.hpp"
#include "DirectionalShadowSunSweepComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "DirectionalShadowMotionComponentScenePayloadSerializer.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeDirectionalShadowSunSweepComponentDeserializer::get_ComponentTypeId()
{
return DirectionalShadowSunSweepComponent::SerializedComponentTypeId;
}

::Component* RuntimeDirectionalShadowSunSweepComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (!String::Equals(record->get_ComponentTypeId(), this->get_ComponentTypeId(), StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Directional-shadow sun-sweep deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000237 = ([&]() {
Array<uint8_t>* __coalesce_00000238 = record->get_Payload();
return __coalesce_00000238 != nullptr ? __coalesce_00000238 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000239 = false;
return new ::MemoryStream(__ctor_arg_00000237, __ctor_arg_00000239);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != DirectionalShadowMotionComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported directional-shadow sun-sweep payload version '") + std::to_string(version) + std::string("'."));
    }
return DirectionalShadowMotionComponentScenePayloadSerializer::ReadSunSweep(reader);}
}
}

