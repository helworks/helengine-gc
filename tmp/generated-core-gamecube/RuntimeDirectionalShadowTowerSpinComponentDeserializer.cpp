#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeDirectionalShadowTowerSpinComponentDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "Component.hpp"
#include "DirectionalShadowTowerSpinComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "DirectionalShadowMotionComponentScenePayloadSerializer.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

std::string RuntimeDirectionalShadowTowerSpinComponentDeserializer::get_ComponentTypeId()
{
return DirectionalShadowTowerSpinComponent::SerializedComponentTypeId;
}

::Component* RuntimeDirectionalShadowTowerSpinComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (!String::Equals(record->get_ComponentTypeId(), this->get_ComponentTypeId(), StringComparison::OrdinalIgnoreCase))
    {
throw new InvalidOperationException(std::string("Directional-shadow tower-spin deserializer cannot deserialize '") + record->get_ComponentTypeId() + std::string("'."));
    }
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000023A = ([&]() {
Array<uint8_t>* __coalesce_0000023B = record->get_Payload();
return __coalesce_0000023B != nullptr ? __coalesce_0000023B : Array<uint8_t>::Empty();
})();
auto __ctor_arg_0000023C = false;
return new ::MemoryStream(__ctor_arg_0000023A, __ctor_arg_0000023C);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != DirectionalShadowMotionComponentScenePayloadSerializer::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported directional-shadow tower-spin payload version '") + std::to_string(version) + std::string("'."));
    }
return DirectionalShadowMotionComponentScenePayloadSerializer::ReadTowerSpin(reader);}
}
}

