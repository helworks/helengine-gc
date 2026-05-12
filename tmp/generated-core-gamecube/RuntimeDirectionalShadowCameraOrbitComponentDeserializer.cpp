#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeDirectionalShadowCameraOrbitComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "Component.hpp"
#include "DirectionalShadowCameraOrbitComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "DirectionalShadowMotionComponentScenePayloadSerializer.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeDirectionalShadowCameraOrbitComponentDeserializer::get_ComponentTypeId()
{
return DirectionalShadowCameraOrbitComponent::SerializedComponentTypeId;
}

::Component* RuntimeDirectionalShadowCameraOrbitComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (!String::Equals(record->get_ComponentTypeId(), this->get_ComponentTypeId(), StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Directional-shadow camera-orbit deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000231 = ([&]() {
Array<uint8_t>* __coalesce_00000232 = record->get_Payload();
return __coalesce_00000232 != nullptr ? __coalesce_00000232 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000233 = false;
return new ::MemoryStream(__ctor_arg_00000231, __ctor_arg_00000233);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != DirectionalShadowMotionComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported directional-shadow camera-orbit payload version '") + std::to_string(version) + std::string("'."));
    }
return DirectionalShadowMotionComponentScenePayloadSerializer::ReadCameraOrbit(reader);}
}
}

