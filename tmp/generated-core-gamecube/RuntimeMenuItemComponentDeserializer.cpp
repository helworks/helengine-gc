#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMenuItemComponentDeserializer.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "byte4.hpp"
#include "MenuItemComponent.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

std::string RuntimeMenuItemComponentDeserializer::get_ComponentTypeId()
{
return MenuItemComponent::SerializedComponentTypeId;
}

::Component* RuntimeMenuItemComponentDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000246 = ([&]() {
Array<uint8_t>* __coalesce_00000247 = record->get_Payload();
return __coalesce_00000247 != nullptr ? __coalesce_00000247 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000248 = false;
return new ::MemoryStream(__ctor_arg_00000246, __ctor_arg_00000248);
})();
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, EngineBinaryEndianness::LittleEndian, true);
const uint8_t version = reader->ReadByte();
    if (version != MenuItemComponent::CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported menu item component payload version '") + std::to_string(version) + std::string("'."));
    }
return ([&]() {
auto __object_00000249 = new ::MenuItemComponent();
__object_00000249->set_PanelId(reader->ReadString());
__object_00000249->set_ItemId(reader->ReadString());
__object_00000249->set_Description(reader->ReadString());
__object_00000249->set_ActionKind(static_cast<MenuActionKind>(reader->ReadByte()));
__object_00000249->set_TargetId(reader->ReadString());
__object_00000249->set_IdleFillColor(this->ReadByte4(reader));
__object_00000249->set_IdleBorderColor(this->ReadByte4(reader));
__object_00000249->set_SelectedFillColor(this->ReadByte4(reader));
__object_00000249->set_SelectedBorderColor(this->ReadByte4(reader));
return __object_00000249;
})();}
}
}

::byte4 RuntimeMenuItemComponentDeserializer::ReadByte4(::EngineBinaryReader* reader)
{
return ([&]() {
auto __ctor_arg_0000024A = reader->ReadByte();
auto __ctor_arg_0000024B = reader->ReadByte();
auto __ctor_arg_0000024C = reader->ReadByte();
auto __ctor_arg_0000024D = reader->ReadByte();
return ::byte4(__ctor_arg_0000024A, __ctor_arg_0000024B, __ctor_arg_0000024C, __ctor_arg_0000024D);
})();}

