#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMenuPanelComponentDeserializer.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "MenuPanelComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

std::string RuntimeMenuPanelComponentDeserializer::get_ComponentTypeId()
{
return MenuPanelComponent::SerializedComponentTypeId;
}

::Component* RuntimeMenuPanelComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_0000024E = ([&]() {
Array<uint8_t>* __coalesce_0000024F = record->get_Payload();
return __coalesce_0000024F != nullptr ? __coalesce_0000024F : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000250 = false;
return new ::MemoryStream(__ctor_arg_0000024E, __ctor_arg_00000250);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != MenuPanelComponent::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported menu panel component payload version '") + std::to_string(version) + std::string("'."));
    }
return ([&]() {
auto __object_00000251 = new ::MenuPanelComponent();
__object_00000251->set_PanelId(reader->ReadString());
return __object_00000251;
})();}
}
}

