#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMenuComponentDeserializer.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "MenuComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

std::string RuntimeMenuComponentDeserializer::get_ComponentTypeId()
{
return MenuComponent::SerializedComponentTypeId;
}

::Component* RuntimeMenuComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000242 = ([&]() {
Array<uint8_t>* __coalesce_00000243 = record->get_Payload();
return __coalesce_00000243 != nullptr ? __coalesce_00000243 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000244 = false;
return new ::MemoryStream(__ctor_arg_00000242, __ctor_arg_00000244);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != MenuComponent::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported menu component payload version '") + std::to_string(version) + std::string("'."));
    }
return ([&]() {
auto __object_00000245 = new ::MenuComponent();
__object_00000245->set_ProviderTypeName(reader->ReadString());
__object_00000245->set_InitialPanelId(reader->ReadString());
return __object_00000245;
})();}
}
}

