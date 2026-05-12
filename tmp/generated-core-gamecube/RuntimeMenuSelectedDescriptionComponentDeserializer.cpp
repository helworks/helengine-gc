#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMenuSelectedDescriptionComponentDeserializer.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "MenuSelectedDescriptionComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

std::string RuntimeMenuSelectedDescriptionComponentDeserializer::get_ComponentTypeId()
{
return MenuSelectedDescriptionComponent::SerializedComponentTypeId;
}

::Component* RuntimeMenuSelectedDescriptionComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000252 = ([&]() {
Array<uint8_t>* __coalesce_00000253 = record->get_Payload();
return __coalesce_00000253 != nullptr ? __coalesce_00000253 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000254 = false;
return new ::MemoryStream(__ctor_arg_00000252, __ctor_arg_00000254);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != MenuSelectedDescriptionComponent::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported menu selected-description component payload version '") + std::to_string(version) + std::string("'."));
    }
return new ::MenuSelectedDescriptionComponent();}
}
}

